// FeigeReset.cpp - 飞鸽传书(FeigeQt) 试用天数重置工具
// 基于 IDA Pro 逆向分析结果
//
// 分析结论:
//   配置文件: %USERPROFILE%\Documents\Feige\config\config.ini
//   QSettings INI 格式, [info] 节中的 freetime 键存储试用剩余天数
//   默认 30 天, 当 freetime >= 31 时判定为过期
//
// 用法: FeigeReset.exe [选项]
//   无参数      - 重置试用天数为 30 天
//   /days:N    - 设置试用天数为 N (1-30)
//   /vip       - 同时清除 VIP 注册信息 (回到试用模式)
//   /backup    - 仅备份, 不修改
//   /restore   - 从备份恢复
//   /info      - 显示当前试用信息

#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ==================== 路径工具 ====================

// 获取 Documents\Feige\config\config.ini 完整路径
BOOL GetConfigPath(WCHAR* outPath, DWORD size)
{
    WCHAR docsPath[MAX_PATH];
    
    // 方法1: 通过 SHGetFolderPath 获取 Documents 路径
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, docsPath)))
    {
        swprintf_s(outPath, size, L"%s\\Feige\\config\\config.ini", docsPath);
        return TRUE;
    }
    
    // 方法2: 通过环境变量
    if (GetEnvironmentVariableW(L"USERPROFILE", docsPath, MAX_PATH))
    {
        swprintf_s(outPath, size, L"%s\\Documents\\Feige\\config\\config.ini", docsPath);
        return TRUE;
    }
    
    return FALSE;
}

// 获取备份路径
void GetBackupPath(const WCHAR* configPath, WCHAR* outPath, DWORD size)
{
    swprintf_s(outPath, size, L"%s.backup", configPath);
}

// ==================== 文件操作 ====================

// 读取整个文件到内存
char* ReadFileAll(const WCHAR* path, DWORD* outSize)
{
    HANDLE hFile = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, 
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return NULL;
    
    DWORD size = GetFileSize(hFile, NULL);
    char* buf = (char*)malloc(size + 1);
    if (!buf) { CloseHandle(hFile); return NULL; }
    
    DWORD read;
    ReadFile(hFile, buf, size, &read, NULL);
    buf[read] = '\0';
    CloseHandle(hFile);
    
    if (outSize) *outSize = read;
    return buf;
}

// 写入文件
BOOL WriteFileAll(const WCHAR* path, const char* data, DWORD size)
{
    HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;
    
    DWORD written;
    BOOL ok = WriteFile(hFile, data, size, &written, NULL);
    CloseHandle(hFile);
    return ok && (written == size);
}

// 复制文件
BOOL CopyFileSimple(const WCHAR* src, const WCHAR* dst)
{
    return CopyFileW(src, dst, FALSE);
}

// ==================== INI 解析 ====================

// 在内容中查找 [info] 节
char* FindSection(char* content, const char* section)
{
    char search[128];
    sprintf_s(search, sizeof(search), "\n[%s]", section);
    
    // 也检查文件开头
    char startSearch[128];
    sprintf_s(startSearch, sizeof(startSearch), "[%s]", section);
    
    char* p = strstr(content, search);
    if (!p) p = strstr(content, startSearch);
    return p;
}

// 在节中找到指定键的值位置, 返回指向值首字符的指针
// 同时通过 outLineEnd 返回行尾位置
char* FindKeyInSection(char* sectionStart, const char* key, char** outLineEnd)
{
    // 跳过节名行
    char* lineStart = strchr(sectionStart, '\n');
    if (!lineStart) return NULL;
    lineStart++; // 到下一行开始
    
    // 遍历行
    while (*lineStart)
    {
        // 检查是否到了下一个节
        if (*lineStart == '[') break;
        
        // 跳过空行和空白
        while (*lineStart == '\r' || *lineStart == '\n') lineStart++;
        if (*lineStart == '\0' || *lineStart == '[') break;
        
        // 找到 '='
        char* eq = strchr(lineStart, '=');
        if (!eq) {
            // 不是键值对, 跳到下一行
            char* nl = strchr(lineStart, '\n');
            if (!nl) break;
            lineStart = nl + 1;
            continue;
        }
        
        // 提取键名 (去掉前后空白)
        char* keyStart = lineStart;
        while (*keyStart == ' ' || *keyStart == '\t') keyStart++;
        char* keyEnd = eq - 1;
        while (keyEnd > keyStart && (*keyEnd == ' ' || *keyEnd == '\t')) keyEnd--;
        int keyLen = (int)(keyEnd - keyStart + 1);
        
        // 比较键名
        if (keyLen == (int)strlen(key) && strncmp(keyStart, key, keyLen) == 0)
        {
            // 找到值开始位置 (= 之后)
            char* valStart = eq + 1;
            while (*valStart == ' ' || *valStart == '\t') valStart++;
            
            // 找到值结束位置 (行尾)
            char* valEnd = valStart;
            while (*valEnd && *valEnd != '\r' && *valEnd != '\n') valEnd++;
            
            if (outLineEnd) *outLineEnd = valEnd;
            return valStart;
        }
        
        // 跳到下一行
        lineStart = eq;
        char* nl = strchr(lineStart, '\n');
        if (!nl) break;
        lineStart = nl + 1;
    }
    
    return NULL;
}

// ==================== 主功能 ====================

// 重置试用天数
BOOL ResetTrialDays(const WCHAR* configPath, int days)
{
    printf("[*] 读取配置文件: %S\n", configPath);
    
    DWORD size;
    char* content = ReadFileAll(configPath, &size);
    if (!content)
    {
        printf("[!] 无法读取配置文件! 请确认 Feige 已安装并运行过。\n");
        return FALSE;
    }
    
    printf("[*] 文件大小: %u 字节\n", size);
    
    // 备份原文件
    WCHAR backupPath[MAX_PATH];
    GetBackupPath(configPath, backupPath, sizeof(backupPath));
    
    if (!CopyFileSimple(configPath, backupPath))
    {
        // 如果备份已存在, 继续
        DWORD err = GetLastError();
        if (err != ERROR_FILE_EXISTS)
        {
            printf("[!] 备份失败 (错误码: %u)\n", err);
        }
    }
    printf("[*] 已备份到: %S\n", backupPath);
    
    // 查找 [info] 节
    char* section = FindSection(content, "info");
    if (!section)
    {
        printf("[!] 未找到 [info] 节! 配置文件可能已损坏。\n");
        free(content);
        return FALSE;
    }
    printf("[*] 找到 [info] 节\n");
    
    // 查找 freetime 键
    char* lineEnd = NULL;
    char* valStart = FindKeyInSection(section, "freetime", &lineEnd);
    if (!valStart)
    {
        printf("[!] 未找到 freetime 键! 将自动创建。\n");
        
        // 在 [info] 节末尾添加 freetime
        // 找到 [info] 节的结尾 (\n[ 或文件尾)
        char* sectionEnd = section + 1;
        while (*sectionEnd && *sectionEnd != '[')
        {
            // 跳过当前行
            char* nl = strchr(sectionEnd, '\n');
            if (!nl) break;
            sectionEnd = nl + 1;
            // 检查下一行是否是新的节
            if (*sectionEnd == '[') break;
        }
        
        // 在 sectionEnd 位置插入 freetime=30
        char newVal[64];
        sprintf_s(newVal, sizeof(newVal), "freetime=%d\r\n", days);
        
        int prefixLen = (int)(sectionEnd - content);
        int suffixLen = (int)(size - prefixLen);
        int newValLen = (int)strlen(newVal);
        
        char* newContent = (char*)malloc(size + newValLen + 1);
        memcpy(newContent, content, prefixLen);
        memcpy(newContent + prefixLen, newVal, newValLen);
        memcpy(newContent + prefixLen + newValLen, content + prefixLen, suffixLen);
        newContent[size + newValLen] = '\0';
        
        free(content);
        content = newContent;
        size = size + newValLen;
    }
    else
    {
        // 读取当前值
        char oldVal[64];
        int oldValLen = (int)(lineEnd - valStart);
        if (oldValLen >= (int)sizeof(oldVal)) oldValLen = sizeof(oldVal) - 1;
        memcpy(oldVal, valStart, oldValLen);
        oldVal[oldValLen] = '\0';
        
        // 处理 @Variant 编码: 如果是 @Variant(...) 格式则替换整个值
        char newVal[64];
        sprintf_s(newVal, sizeof(newVal), "%d", days);
        
        int oldValStart = (int)(valStart - content);
        int oldValEnd = (int)(lineEnd - content);
        
        int newSize = size - (oldValEnd - oldValStart) + (int)strlen(newVal);
        char* newContent = (char*)malloc(newSize + 1);
        
        memcpy(newContent, content, oldValStart);
        memcpy(newContent + oldValStart, newVal, strlen(newVal));
        memcpy(newContent + oldValStart + strlen(newVal), 
               content + oldValEnd, size - oldValEnd);
        newContent[newSize] = '\0';
        
        free(content);
        content = newContent;
        size = newSize;
    }
    
    // 写入文件
    if (!WriteFileAll(configPath, content, size))
    {
        printf("[!] 写入配置文件失败! 请以管理员身份运行。\n");
        free(content);
        return FALSE;
    }
    
    printf("[+] 成功! freetime 已重置为 %d 天。\n", days);
    printf("[*] 原始文件已备份为 .backup\n");
    free(content);
    return TRUE;
}

// 清除 VIP 信息
BOOL ClearVipInfo(const WCHAR* configPath)
{
    printf("[*] 清除 VIP 注册信息...\n");
    
    DWORD size;
    char* content = ReadFileAll(configPath, &size);
    if (!content)
    {
        printf("[!] 无法读取配置文件!\n");
        return FALSE;
    }
    
    // 备份
    WCHAR backupPath[MAX_PATH];
    GetBackupPath(configPath, backupPath, sizeof(backupPath));
    CopyFileSimple(configPath, backupPath);
    
    // 移除 vipcode 和 vipdate 行
    char* section = FindSection(content, "info");
    if (section)
    {
        // 简单方法: 用空格覆盖 vipcode=xxx 行和 vipdate=xxx 行
        char* p = section;
        while (*p && *p != '[')
        {
            if (strncmp(p, "vipcode=", 8) == 0 || strncmp(p, "vipdate=", 8) == 0)
            {
                // 将这行替换为空格 (保留换行格式)
                char* nl = strchr(p, '\n');
                if (nl)
                {
                    memset(p, ' ', (int)(nl - p));
                }
            }
            p = strchr(p, '\n');
            if (!p) break;
            p++;
            if (*p == '[') break;
        }
    }
    
    if (!WriteFileAll(configPath, content, size))
    {
        printf("[!] 写入失败!\n");
        free(content);
        return FALSE;
    }
    
    printf("[+] VIP 信息已清除, 回到试用模式。\n");
    free(content);
    return TRUE;
}

// 显示当前信息
BOOL ShowInfo(const WCHAR* configPath)
{
    DWORD size;
    char* content = ReadFileAll(configPath, &size);
    if (!content)
    {
        printf("[!] 无法读取配置文件!\n");
        printf("    预期路径: %S\n", configPath);
        return FALSE;
    }
    
    printf("\n========== Feige 试用状态 ==========\n");
    printf("配置文件: %S\n\n", configPath);
    
    char* section = FindSection(content, "info");
    if (section)
    {
        char* end;
        
        char* freetime = FindKeyInSection(section, "freetime", &end);
        if (freetime)
        {
            char val[32];
            int len = (int)(end - freetime);
            if (len > 30) len = 30;
            memcpy(val, freetime, len);
            val[len] = '\0';
            int days = atoi(val);
            printf("  试用剩余天数: %d 天", days);
            if (days >= 31) printf(" [已过期!]");
            printf("\n");
        }
        else
        {
            printf("  试用剩余天数: (未设置, 默认 30 天)\n");
        }
        
        char* vipcode = FindKeyInSection(section, "vipcode", &end);
        if (vipcode)
        {
            char val[128];
            int len = (int)(end - vipcode);
            if (len > 127) len = 127;
            memcpy(val, vipcode, len);
            val[len] = '\0';
            if (val[0] && val[0] != ' ')
                printf("  VIP 注册码: %s\n", val);
            else
                printf("  VIP 注册码: (已清除)\n");
        }
        else
        {
            printf("  VIP 注册码: (无)\n");
        }
        
        char* vipdate = FindKeyInSection(section, "vipdate", &end);
        if (vipdate)
        {
            char val[64];
            int len = (int)(end - vipdate);
            if (len > 63) len = 63;
            memcpy(val, vipdate, len);
            val[len] = '\0';
            if (val[0] && val[0] != ' ')
                printf("  VIP 到期日期: %s\n", val);
            else
                printf("  VIP 到期日期: (已清除)\n");
        }
        else
        {
            printf("  VIP 到期日期: (无)\n");
        }
        
        char* mac = FindKeyInSection(section, "mac", &end);
        if (mac)
        {
            char val[32];
            int len = (int)(end - mac);
            if (len > 31) len = 31;
            memcpy(val, mac, len);
            val[len] = '\0';
            printf("  绑定 MAC: %s\n", val);
        }
    }
    else
    {
        printf("  (未找到 [info] 节)\n");
    }
    
    printf("\n=====================================\n");
    free(content);
    return TRUE;
}

// ==================== 主函数 ====================

void PrintUsage()
{
    printf("FeigeReset - 飞鸽传书(FeigeQt) 试用天数重置工具\n");
    printf("基于 IDA Pro 逆向分析 | config.ini (QSettings INI 格式)\n\n");
    printf("用法: FeigeReset.exe [选项]\n\n");
    printf("选项:\n");
    printf("  (无参数)    重置试用天数为 30 天\n");
    printf("  /days:N    设置试用天数为 N (1-30)\n");
    printf("  /vip       同时清除 VIP 注册信息, 回到试用模式\n");
    printf("  /info      仅显示当前试用状态\n");
    printf("  /backup    仅备份配置文件 (不修改)\n");
    printf("  /restore   从备份恢复配置文件\n");
    printf("\n示例:\n");
    printf("  FeigeReset.exe              # 重置为 30 天\n");
    printf("  FeigeReset.exe /days:30     # 重置为 30 天\n");
    printf("  FeigeReset.exe /vip         # 清除 VIP + 重置 30 天\n");
    printf("  FeigeReset.exe /info        # 查看试用状态\n");
}

int wmain(int argc, WCHAR* argv[])
{
    // 设置控制台编码
    SetConsoleOutputCP(CP_UTF8);
    
    printf("\n");
    printf("  FeigeReset v1.0 - 飞鸽传书试用重置工具\n");
    printf("  ======================================\n\n");
    
    // 获取配置文件路径
    WCHAR configPath[MAX_PATH];
    if (!GetConfigPath(configPath, MAX_PATH))
    {
        printf("[!] 无法确定配置文件路径!\n");
        return 1;
    }
    
    // 检查文件是否存在
    DWORD attrs = GetFileAttributesW(configPath);
    if (attrs == INVALID_FILE_ATTRIBUTES)
    {
        printf("[!] 配置文件不存在: %S\n", configPath);
        printf("[*] 如果 Feige 尚未运行过, 配置文件会在首次启动时自动创建。\n");
        
        // 创建目录和初始配置文件
        WCHAR dirPath[MAX_PATH];
        wcscpy_s(dirPath, MAX_PATH, configPath);
        WCHAR* lastSlash = wcsrchr(dirPath, L'\\');
        if (lastSlash)
        {
            *lastSlash = L'\0';
            // 递归创建目录
            WCHAR tmpPath[MAX_PATH];
            wcscpy_s(tmpPath, MAX_PATH, dirPath);
            for (WCHAR* p = tmpPath; *p; p++)
            {
                if (*p == L'\\')
                {
                    *p = L'\0';
                    CreateDirectoryW(tmpPath, NULL);
                    *p = L'\\';
                }
            }
            CreateDirectoryW(tmpPath, NULL);
            
            // 写入默认配置
            const char* defaultConfig = "[info]\r\nfreetime=30\r\n\r\n";
            if (WriteFileAll(configPath, defaultConfig, (DWORD)strlen(defaultConfig)))
            {
                printf("[+] 已创建初始配置文件, freetime=30 天!\n");
                return 0;
            }
        }
        return 1;
    }
    
    // 解析参数
    int days = 30;
    BOOL doReset = TRUE;
    BOOL doVipClear = FALSE;
    BOOL doInfo = FALSE;
    BOOL doBackup = FALSE;
    BOOL doRestore = FALSE;
    
    for (int i = 1; i < argc; i++)
    {
        if (wcsncmp(argv[i], L"/days:", 6) == 0 || wcsncmp(argv[i], L"/d:", 3) == 0)
        {
            const WCHAR* p = wcschr(argv[i], L':') + 1;
            days = _wtoi(p);
            if (days < 1) days = 1;
            if (days > 30) days = 30;
        }
        else if (_wcsicmp(argv[i], L"/vip") == 0)
        {
            doVipClear = TRUE;
        }
        else if (_wcsicmp(argv[i], L"/info") == 0)
        {
            doInfo = TRUE;
            doReset = FALSE;
        }
        else if (_wcsicmp(argv[i], L"/backup") == 0)
        {
            doBackup = TRUE;
            doReset = FALSE;
        }
        else if (_wcsicmp(argv[i], L"/restore") == 0)
        {
            doRestore = TRUE;
            doReset = FALSE;
        }
    }
    
    // 执行操作
    if (doRestore)
    {
        WCHAR backupPath[MAX_PATH];
        GetBackupPath(configPath, backupPath, sizeof(backupPath));
        if (CopyFileSimple(backupPath, configPath))
            printf("[+] 已从备份恢复!\n");
        else
            printf("[!] 备份文件不存在或恢复失败\n");
        return 0;
    }
    
    if (doBackup)
    {
        WCHAR backupPath[MAX_PATH];
        GetBackupPath(configPath, backupPath, sizeof(backupPath));
        if (CopyFileSimple(configPath, backupPath))
        {
            printf("[+] 已备份到: %S\n", backupPath);
            
            // 显示备份内容
            DWORD size;
            char* content = ReadFileAll(configPath, &size);
            if (content)
            {
                printf("\n当前配置内容:\n%s\n", content);
                free(content);
            }
        }
        else
            printf("[!] 备份失败\n");
        return 0;
    }
    
    if (doInfo)
    {
        ShowInfo(configPath);
        return 0;
    }
    
    if (doReset)
    {
        if (doVipClear)
        {
            ClearVipInfo(configPath);
        }
        
        if (ResetTrialDays(configPath, days))
        {
            printf("\n");
            ShowInfo(configPath);
        }
        else
        {
            return 1;
        }
    }
    
    printf("\n[*] 操作完成。请重启 Feige 使更改生效。\n");
    return 0;
}

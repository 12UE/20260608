// TyporaTrialReset.cpp - Typora Trial Reset Tool v1.1
//
// Analysis:
//   Trial data locations:
//   1. HKCU\Software\Typora
//      - IDate    = "5/14/2026"  (first install date)
//      - SLicense = ""           (license string, empty=trial mode)
//   2. %APPDATA%\Typora\profile.data
//      - _iD field = "5/14/2026"  (synced with registry IDate,注意大写D)
//   3. %APPDATA%\Typora\Local Storage\leveldb\
//   4. %APPDATA%\Typora\Cache\ and other cache dirs
//
// Strategy: reset date + clean trial data + disable auto-update

#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <shlobj.h>
#include <sstream>
#include <ctime>
#include <fstream>
#include <cstdlib>

#pragma comment(lib, "shell32.lib")

// ── utility functions ──────────────────────────────────

std::string getAppDataPath() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        return std::string(path) + "\\Typora";
    }
    return "";
}

bool fileExists(const std::string& path) {
    return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

// ── registry operations ────────────────────────────────

bool resetRegistry() {
    std::cout << "[REG] Processing HKCU\\Software\\Typora...\n";

    HKEY hKey;
    LSTATUS status = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Typora", 0,
        KEY_READ | KEY_WRITE, &hKey);

    if (status != ERROR_SUCCESS) {
        std::cout << "  [INFO] Registry key not found (first run?)\n";
        return true;
    }

    time_t now = time(nullptr);
    struct tm tm_now;
    localtime_s(&tm_now, &now);
    char newDate[32];
    snprintf(newDate, sizeof(newDate), "%d/%d/%d",
        tm_now.tm_mon + 1, tm_now.tm_mday, tm_now.tm_year + 1900);

    status = RegSetValueExA(hKey, "IDate", 0, REG_SZ,
        (BYTE*)newDate, (DWORD)(strlen(newDate) + 1));
    std::cout << "  [OK] IDate -> " << newDate << "\n";

    const char* emptyLicense = "";
    status = RegSetValueExA(hKey, "SLicense", 0, REG_SZ,
        (BYTE*)emptyLicense, 1);
    std::cout << "  [OK] SLicense cleared\n";

    RegCloseKey(hKey);
    return true;
}

// ── hex encode/decode ──────────────────────────────────

std::string hexDecode(const std::string& hex) {
    std::string result;
    for (size_t i = 0; i + 1 < hex.length(); i += 2) {
        char hi = hex[i];
        char lo = hex[i + 1];
        if (hi >= 'a' && hi <= 'f') hi -= 32;
        if (lo >= 'a' && lo <= 'f') lo -= 32;
        if (hi >= '0' && hi <= '9') hi -= '0';
        else if (hi >= 'A' && hi <= 'F') hi = hi - 'A' + 10;
        else continue;
        if (lo >= '0' && lo <= '9') lo -= '0';
        else if (lo >= 'A' && lo <= 'F') lo = lo - 'A' + 10;
        else continue;
        result += (char)((hi << 4) | lo);
    }
    return result;
}

std::string hexEncode(const std::string& raw) {
    static const char hexChars[] = "0123456789abcdef";
    std::string result;
    for (unsigned char c : raw) {
        result += hexChars[c >> 4];
        result += hexChars[c & 0x0F];
    }
    return result;
}

// ── profile.data update ────────────────────────────────

bool resetProfileData() {
    std::string appdataPath = getAppDataPath();
    if (appdataPath.empty()) return false;

    std::string profilePath = appdataPath + "\\profile.data";

    std::cout << "[PROFILE] Updating profile.data...\n";

    time_t now = time(nullptr);
    struct tm tm_now;
    localtime_s(&tm_now, &now);
    char newDate[32];
    snprintf(newDate, sizeof(newDate), "%d/%d/%d",
        tm_now.tm_mon + 1, tm_now.tm_mday, tm_now.tm_year + 1900);

    std::string finalContent;

    if (fileExists(profilePath)) {
        std::ifstream inFile(profilePath, std::ios::binary);
        if (inFile) {
            std::string rawContent((std::istreambuf_iterator<char>(inFile)),
                                    std::istreambuf_iterator<char>());
            inFile.close();

            bool isHex = !rawContent.empty();
            for (char c : rawContent) {
                if (c == '\n' || c == '\r' || c == ' ' || c == '\t') continue;
                if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                    isHex = false;
                    break;
                }
            }

            std::string jsonStr = isHex ? hexDecode(rawContent) : rawContent;

            // update _iD field (capital D!)
            size_t idPos = jsonStr.find("\"_iD\":\"");
            if (idPos != std::string::npos) {
                size_t valStart = idPos + 7;
                size_t valEnd = jsonStr.find('"', valStart);
                if (valEnd != std::string::npos) {
                    jsonStr.replace(valStart, valEnd - valStart, newDate);
                }
            } else {
                size_t bracePos = jsonStr.find('{');
                if (bracePos != std::string::npos) {
                    jsonStr.insert(bracePos + 1, "\"_iD\":\"" + std::string(newDate) + "\",");
                }
            }

            finalContent = isHex ? hexEncode(jsonStr) : jsonStr;
            std::cout << "  [OK] _iD -> " << newDate << " (uuid preserved)\n";
        }
    }

    if (finalContent.empty()) {
        std::ostringstream oss;
        oss << "{"
            << "\"version\":\"1.13.4\","
            << "\"initialize_ver\":\"1.0.0\","
            << "\"_iD\":\"" << newDate << "\","
            << "\"isDarkMode\":false"
            << "}";
        finalContent = hexEncode(oss.str());
        std::cout << "  [OK] _iD -> " << newDate << " (created new)\n";
    }

    std::ofstream outFile(profilePath, std::ios::binary | std::ios::trunc);
    if (!outFile) {
        std::cerr << "  [ERR] Cannot write profile.data\n";
        return false;
    }
    outFile << finalContent;
    outFile.close();

    return true;
}

// ── disable auto-update ────────────────────────────────
//
// Typora update check flow:
//   1. Startup: if enableAutoUpdate=true, calls updater.checkForUpdates via IPC
//   2. Manual:  Menu -> Help -> Check Updates -> checkUpdates() -> g.invoke("updater.checkForUpdates", true)
//   3. The IPC handler is registered in atom.compiled.dist.jsc (V8 bytecode)
//
// Patch strategy:
//   A. Patch checkUpdates() in Preferences.js to be a no-op (prevents manual check)
//   B. Write Preferences file with enableAutoUpdate=false (prevents startup check)

bool disableUpdateCheck() {
    std::string appdataPath = getAppDataPath();
    if (appdataPath.empty()) return false;

    std::cout << "[UPDATE] Disabling auto-update check...\n";

    // --- A. Patch checkUpdates in Preferences.js ---
    // Original:
    //   checkUpdates:function(){}
    // Patched:
    //   checkUpdates:function(){}

    std::string pageDistPath = appdataPath;
    // page-dist is under Program Files, not AppData
    // Need to find the actual install path
    char progPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROGRAM_FILES, NULL, 0, progPath))) {
        pageDistPath = std::string(progPath) + "\\Typora\\resources\\page-dist";
    }

    // find the Preferences.*.js file
    WIN32_FIND_DATAA fd;
    std::string searchDir = pageDistPath + "\\static\\js";
    std::string searchPattern = searchDir + "\\Preferences.*.js";
    HANDLE hFind = FindFirstFileA(searchPattern.c_str(), &fd);

    bool jsPatched = false;
    if (hFind != INVALID_HANDLE_VALUE) {
        std::string jsPath = searchDir + "\\" + fd.cFileName;
        FindClose(hFind);

        std::ifstream jsIn(jsPath, std::ios::binary);
        if (jsIn) {
            std::string jsContent((std::istreambuf_iterator<char>(jsIn)),
                                   std::istreambuf_iterator<char>());
            jsIn.close();

            // find and patch checkUpdates function
            std::string oldFunc = "checkUpdates:function(){window.isWin&&g.invoke(\"updater.checkForUpdates\",!0),window.webkit&&_(\"checkForUpdates\")}";
            std::string newFunc = "checkUpdates:function(){}";

            size_t pos = jsContent.find(oldFunc);
            if (pos != std::string::npos) {
                jsContent.replace(pos, oldFunc.length(), newFunc);

                std::ofstream jsOut(jsPath, std::ios::binary | std::ios::trunc);
                if (jsOut) {
                    jsOut << jsContent;
                    jsOut.close();
                    jsPatched = true;
                    std::cout << "  [OK] Patched " << fd.cFileName << " (checkUpdates -> no-op)\n";
                }
            } else {
                std::cout << "  [INFO] checkUpdates function not found (may already be patched)\n";
                // check if already patched
                if (jsContent.find("checkUpdates:function(){}") != std::string::npos) {
                    jsPatched = true;
                    std::cout << "  [OK] Already patched\n";
                }
            }
        }
    } else {
        std::cout << "  [WARN] Preferences JS not found\n";
    }

    // --- B. Write Preferences file with enableAutoUpdate=false ---
    // This controls the startup auto-update check
    // Note: cleanCaches() deletes Preferences, so we write it AFTER cleanup

    std::string prefsPath = appdataPath + "\\Preferences";

    // Preferences file is a JSON-like object (Electron's Preferences format)
    // We write minimal content to override just the update setting
    std::ofstream prefsOut(prefsPath, std::ios::binary | std::ios::trunc);
    if (prefsOut) {
        prefsOut << "{\"enableAutoUpdate\":false,\"autoUpdateToDev\":false,\"skipVersion\":\"999999\"}";
        prefsOut.close();
        std::cout << "  [OK] Preferences: enableAutoUpdate=false, skipVersion=999999\n";
    } else {
        std::cout << "  [WARN] Cannot write Preferences file\n";
    }

    return jsPatched;
}

// ── directory removal ──────────────────────────────────

bool removeDirectory(const std::string& dirPath) {
    if (!fileExists(dirPath)) return true;

    std::string searchPath = dirPath + "\\*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);

    if (hFind == INVALID_HANDLE_VALUE) {
        RemoveDirectoryA(dirPath.c_str());
        return true;
    }

    do {
        if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;

        std::string fullPath = dirPath + "\\" + fd.cFileName;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            removeDirectory(fullPath);
        } else {
            SetFileAttributesA(fullPath.c_str(), FILE_ATTRIBUTE_NORMAL);
            DeleteFileA(fullPath.c_str());
        }
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
    RemoveDirectoryA(dirPath.c_str());
    return true;
}

// ── cache cleanup ──────────────────────────────────────

bool cleanCaches() {
    std::string appdataPath = getAppDataPath();
    if (appdataPath.empty()) return false;

    std::vector<std::string> dirsToClean = {
        "Cache",
        "Code Cache",
        "GPUCache",
        "DawnGraphiteCache",
        "DawnWebGPUCache",
        "Local Storage",
        "Session Storage",
        "Network",
        "Shared Dictionary",
        "blob_storage",
    };

    // NOTE: Preferences is NOT in this list anymore - we handle it separately
    std::vector<std::string> filesToDelete = {
        "history.data",
        "Cookies",
        "Cookies-journal",
        "DIPS",
        "DIPS-shm",
        "DIPS-wal",
        "SharedStorage",
        "SharedStorage-wal",
        "lockfile",
        "typora.log",
    };

    for (auto& dir : dirsToClean) {
        std::string dirPath = appdataPath + "\\" + dir;
        if (fileExists(dirPath)) {
            std::cout << "[CACHE] Cleaning " << dir << "...\n";
            removeDirectory(dirPath);
        }
    }

    for (auto& file : filesToDelete) {
        std::string filePath = appdataPath + "\\" + file;
        if (fileExists(filePath)) {
            DeleteFileA(filePath.c_str());
        }
    }

    return true;
}

// ── main ───────────────────────────────────────────────

void printBanner() {
    std::cout << R"(
=====================================================
         Typora Trial Reset Tool v1.1
         + Auto-Update Disable
=====================================================
)";
}

int main() {
    printBanner();

    std::cout << "This tool will:\n";
    std::cout << "  - Reset install date to today\n";
    std::cout << "  - Clear license info\n";
    std::cout << "  - Disable auto-update check\n";
    std::cout << "  - Clean browser local storage\n";
    std::cout << "  - Clean cache and temp files\n\n";

    std::cout << "NOTE: Close Typora first!\n\n";

    std::cout << "\n======================================\n\n";

    resetRegistry();
    resetProfileData();

    // disable update BEFORE cache cleanup (Preferences.js patch is in Program Files)
    disableUpdateCheck();

    cleanCaches();

    // write Preferences AFTER cache cleanup (cleanCaches deletes it)
    // re-write to ensure enableAutoUpdate stays false
    std::string prefsPath = getAppDataPath() + "\\Preferences";
    std::ofstream prefsOut(prefsPath, std::ios::binary | std::ios::trunc);
    if (prefsOut) {
        prefsOut << "{\"enableAutoUpdate\":false,\"autoUpdateToDev\":false,\"skipVersion\":\"999999\"}";
        prefsOut.close();
    }

    std::cout << "\n======================================\n";
    std::cout << "\n[Done] Reset complete!\n";
    std::cout << "Auto-update has been disabled.\n";
    std::cout << "Restart Typora to begin a new trial.\n\n";
    std::cout << "Press Enter to exit...";
    std::cin.get();

    return 0;
}

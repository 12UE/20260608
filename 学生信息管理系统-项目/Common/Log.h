#pragma once
#include <iostream>
#include <Windows.h>
#include <string>
#define POST_QUIT PostQuitMessage(0) // 退出程序宏定义

static std::string GetErrorMsg()
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL) == 0)
    {
        return "";
    }
    std::string strMsg = (char*)lpMsgBuf;
    LocalFree(lpMsgBuf);

    return strMsg;
}

template<typename ...T>
static void logs(const char* szFmt, T... t)
{
    char szBuf[0x256] = { 0 };
    char szMarking[0x256] = "[Log] ";
    strcat(szMarking, szFmt);
    sprintf(szBuf, szMarking, t...);
    printf("%s\n", szBuf);
}

static void logs(const char* szFmt)
{
    char szBuf[0x256] = "[Log] ";
    strcat(szBuf, szFmt);
    OutputDebugStringA(szBuf);
}
#pragma once

#include <WinSock2.h>
#include <Windows.h>

#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

class SocketInit
{
public:
    SocketInit()
    {
        // ≥ı ºªØ
        WORD wVersionRequested;
        WSADATA wsaData;
        int err;

        wVersionRequested = MAKEWORD(2, 2);

        err = WSAStartup(wVersionRequested, &wsaData);
        if (err != 0) {
            // printf(, err);
            throw std::runtime_error("WSAStartup  ß∞‹");
        }
        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
            WSACleanup();
            throw std::runtime_error("Could not find a usable version of Winsock.dll\n");
        }
    }

    ~SocketInit()
    { 
        WSACleanup();
    }

private:
    static SocketInit s_init;
};

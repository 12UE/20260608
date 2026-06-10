#pragma once
#include <functional>
#include <map>
#define WIN32_LEAN_AND_MEAN
#include <Winsock2.h>
#pragma comment(lib,"Ws2_32.lib")


class CUdp
{
private:
	void WSAInit()
	{
		WORD wVersionRequested;
		int err;

		wVersionRequested = MAKEWORD(2, 2);

		err = WSAStartup(wVersionRequested, &m_wsaData);
		if (err != 0) {
			return;
		}
	}

	void WSAUnInit()
	{
		if (LOBYTE(m_wsaData.wVersion) != 2 ||
			HIBYTE(m_wsaData.wVersion) != 2) {
			WSACleanup();
			return;
		}

		WSACleanup();
	}

private:
	WSADATA m_wsaData;
	SOCKET m_Sock;

public:
	CUdp():
		m_Sock(INVALID_SOCKET)
	{
		WSAInit();
	}

	~CUdp()
	{
		WSAUnInit();
		DeleteSocket();
	}

	void DeleteSocket()
	{
		// 判断socket是否已创建
		if (m_Sock != INVALID_SOCKET)
		{
			closesocket(m_Sock);
			m_Sock = INVALID_SOCKET;
		}
	}

	bool CreateServer(char* szAddr, DWORD dwPort)
	{
		return CreateSocket(szAddr, dwPort, true);
	}

	bool CreateClient(char* szAddr, DWORD dwPort)
	{
		return CreateSocket(szAddr, dwPort, false);
	}

	bool CreateSocket(char* szAddr, DWORD dwPort, bool bServer)
	{
		// 避免重复创建socket
		if (m_Sock == INVALID_SOCKET)
		{
			m_Sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (m_Sock == INVALID_SOCKET)
			{
				printf("创建套接字失败，错误代码：%d\r\n",
					WSAGetLastError());
				return false;
			}
		}

		// 设置udp接收为非阻塞模式
		u_long bBlock = true;
		ioctlsocket(m_Sock, FIONBIO, &bBlock);

		// 绑定端口
		if (bServer)
		{
			sockaddr_in si = { 0 };
			si.sin_family = AF_INET;
			si.sin_port = htons(dwPort);
			si.sin_addr.S_un.S_addr = inet_addr(szAddr);

			int nRet = bind(m_Sock, (sockaddr*)&si, sizeof(si));

			if (nRet == SOCKET_ERROR)
			{
				printf("绑定端口失败，错误代码：%d\r\n",
					WSAGetLastError());
				DeleteSocket();
				return false;
			}
		}

		return true;
	}

	int Recv(const uint8_t* pBuff, int nSize, sockaddr_in* psi)
	{
		int nLen = sizeof(sockaddr_in);
		return recvfrom(m_Sock, (char*)pBuff, nSize, 0, (sockaddr*)psi, &nLen);
	}

	// 端口需要使用htons转换后的端口
	int Send(const uint8_t* pBuff, int nSendLen, sockaddr_in* psi)
	{
		return sendto(m_Sock, (char*)pBuff, nSendLen, 0, (sockaddr*)psi, sizeof(sockaddr_in));
	}

	template<typename T>
	int Send(const T& data, sockaddr_in* psi)
	{
		return Send((uint8_t*)&data, sizeof(data), psi);
	}

	template<typename T>
	int Recv(const T& data, sockaddr_in* psi)
	{
		return Recv((uint8_t*)&data, sizeof(data), psi);
	}
};
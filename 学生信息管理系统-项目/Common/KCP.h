#pragma once
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include "KProto.h"
#include "udp.hpp"
#include "CircleQueue.hpp"
#include"Singleton.h"
#define RECV_SZSTR (-1)
struct PakageInfo
{
public:
	PakageInfo() {}
	~PakageInfo() {}
	PakageInfo(Pakage& pkg, time_t time):
		m_pkg(pkg),
		m_time(time)
	{

	}
	Pakage m_pkg;
	time_t m_time = 0;
};

class KCP :public Singleton<KCP>
{
public:
	KCP();

	~KCP();

	bool Bind(const CHAR* pAddr, uint16_t dwPort);
	bool Connect(const CHAR* pAddr, uint16_t dwPort);

	//template<typename T>
	//void Recv(const T* pBuf, size_t nSize);

	//template<typename T>
	//void Send(const T* pBuf, size_t nSize);

	void Send(void* pBuf, size_t nSize);
	size_t Recv(void* pBuf, size_t nSize);
	void StickRecv(CHAR* pBuf, size_t nSize);
private:
	void WorkThread();

private:
	bool m_bConnected = false;	// 是否连接成功
	size_t m_nSeq = 0;			// 包的序号
	sockaddr_in m_si;			// 远程地址

	CUdp m_udp;					// UDP对象

	bool m_bWorking = false;					// 用于线程退出的判断
	std::thread						m_thread;	// 工作线程

	size_t m_nSendSeq = 0;					// 切包序号
	size_t m_nBufSeq  = 0;					// 进入缓冲区的包序号

	std::recursive_mutex			m_mtxSned;	// 发包队列的锁
	std::recursive_mutex			m_mtxBuf;	// 数据缓冲区的锁

	std::map<size_t, PakageInfo>	m_mpSend;	// 发包队列
	std::map<size_t, Pakage>		m_mpRecv;	// 收包队列
	std::vector<uint8_t>			m_vecPkg;	// 数据缓冲区

	CircleQueue<Pakage>				m_queueRecv;// 接收队列
};
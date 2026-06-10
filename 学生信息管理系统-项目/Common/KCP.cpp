#pragma once
#include "KCP.h"
#include "Log.h"

constexpr auto TIME = 2000;
constexpr auto WINDOW_SIZE = 1000;

KCP::KCP()
{
}

KCP::~KCP()
{
	m_bWorking = false;
	m_thread.join();
}

bool KCP::Bind(const CHAR* pAddr, uint16_t dwPort)
{
    if (!m_udp.CreateServer((char*)pAddr, dwPort))
    {
        return false;
    }

    // 启动工作线程
    m_bWorking = true;
    m_thread = std::thread(&KCP::WorkThread, this);

    return true;
}

bool KCP::Connect(const CHAR* pAddr, uint16_t dwPort)
{
    if (m_udp.CreateClient((char*)pAddr, dwPort))
    {
        Pakage pkg(CMD_SYN, m_nSeq);
        m_si.sin_family = AF_INET;
        m_si.sin_port = htons(dwPort);
        m_si.sin_addr.S_un.S_addr = inet_addr((char*)pAddr);
		m_mpSend[m_nSeq] = PakageInfo(pkg, 0);
		//m_nSeq++;

        // 启动工作线程
        m_bWorking = true;
        m_thread = std::thread(&KCP::WorkThread, this);

        while (!m_bConnected)
        {
            std::this_thread::yield(); // 切线程并等待连接确认
        }
		return true;
    }
    return false;
}

void KCP::Send(void* pBuf, size_t nSize)
{
    // 切包
    int nCout = (nSize % MTU == 0) ? (nSize / MTU) : (nSize / MTU + 1);
    for (size_t i = 0; i < nCout; i++)
    {
		// 等待发送队列有空余空间
        while (m_mpSend.size() >= WINDOW_SIZE)
        {
            // 发送队列已满，等待
			std::this_thread::yield();
        }

        uint32_t nLen = MTU;
        // 判断最后一次包的长度大小
        if (i == nCout - 1)
        {
            nLen = nSize - i * MTU;
        }
        // 组包放入发送队列
        Pakage pkg(CMD_PSH, m_nSeq, (uint8_t*)((char*)pBuf + i * MTU), nLen);
        PakageInfo pkgInfo(pkg, 0);
		m_mtxSned.lock();
        m_mpSend[m_nSeq] = pkgInfo;
		m_mtxSned.unlock();

        m_nSeq++;
    }
}

size_t KCP::Recv(void* pBuf, size_t nSize)
{
	size_t nRecvSize = 0; // 已接收的大小
    while (true)
    {
        if (m_queueRecv.IsEmpty())
        {
            Sleep(1);
            std::this_thread::yield();
            continue;
        }

        m_mtxBuf.lock();
        auto& pkg = m_queueRecv.Front(); // 获取队首包


		// 如果请求大小小于包大小，则调整包地址与大小
        if (nSize < pkg.m_nLen)
        {
            memcpy((char*)pBuf, pkg.m_data, nSize);

			int nNewSize = pkg.m_nLen - nSize; // 计算剩余数据大小
            int nBuffSize = sizeof(pkg.m_data);

            memcpy((char*)&pkg.m_nLen,&nNewSize,sizeof(nNewSize));

            uint8_t* pBuff = new uint8_t[nBuffSize]{0}; // 分配新的缓冲区
			memcpy(pBuff, pkg.m_data, nBuffSize); // 拷贝数据到新的缓冲区
			memset((char*)pkg.m_data, 0, nBuffSize); // 清空原有数据
            memcpy((char*)pkg.m_data, pBuff+ nSize, nNewSize); // 拷贝移动后的数据到原缓冲区
            nRecvSize = nSize;
            delete[] pBuff;
        }
        else
        {
            memcpy((char*)pBuf, pkg.m_data, pkg.m_nLen);

            m_queueRecv.Pop();
			nRecvSize = pkg.m_nLen; // 设置已接收大小为包的长度
        }

        
        //m_vecPkg.erase(m_vecPkg.begin(), m_vecPkg.begin() + nSize);

        m_mtxBuf.unlock();
        break;
    }
	return nRecvSize; // 返回已接收的大小
}

void KCP::StickRecv(CHAR* pBuf, size_t nSize)
{
    size_t nOverRecvSize = 0; // 已接收的大小
    while (true)
    {
        if (nOverRecvSize == nSize)
        {
			break; // 已接收完毕
        }

        size_t nRecvSize = Recv(pBuf + nOverRecvSize, nSize - nOverRecvSize);
        nOverRecvSize += nRecvSize; // 累加已接收大小
    }
}

void KCP::WorkThread()
{  
    while (m_bWorking)  
    {  
        // 接收数据  
        Pakage pkg{};
        sockaddr_in si{ 0 };  
        m_udp.Recv(pkg, &si);
        switch (pkg.m_nCmd)
        {
            // 回复收到确认的包  
            case CMD_ACK:  
            {  
                // 在发送包队列中删除包  
                m_mtxSned.lock();  
                m_mpSend.erase(pkg.m_nSeq);  
                m_mtxSned.unlock();

				m_bConnected = true; // 标识连接成功
                break;  
            }  
            // 连接包  
            case CMD_SYN:  
            {  
				printf("收到连接包\r\n");
                // 获取客户端si  
                m_si = si;  

                // 标识连接成功
                m_bConnected = true;

                // 回复连接包  
                Pakage pkgAck(CMD_ACK, pkg.m_nSeq);  
                m_udp.Send(pkgAck, &m_si);  

                break;  
            }  
            // 正常收包  
            case CMD_PSH:  
            {  
                if (!m_bConnected)
                {
					break; // 未连接状态不处理数据包
                }

                if (pkg.m_nSeq < m_nBufSeq || m_mpRecv.find(pkg.m_nSeq) != m_mpRecv.end())
                {
                    // 回复ACK包
                    Pakage pkgAck(CMD_ACK, pkg.m_nSeq);
                    m_udp.Send(pkgAck, &m_si);
					break; // 重复包或已处理的包不再处理
                }

                // 校验包是否正确  
                auto nCheck = calculate_checksum(pkg.m_data, pkg.m_nLen);
                if (nCheck == pkg.m_nCheck)
                {
                    // 加入收包队列
                    m_mpRecv[pkg.m_nSeq] = pkg;

                    // 回复ACK包
                    Pakage pkgAck(CMD_ACK, pkg.m_nSeq);
                    m_udp.Send(pkgAck, &m_si);

                }

                

                break;  
            }  
            // 断开连接包  
            case CMD_FIN:  
            {
                // 返回ACK
                Pakage pkgAck(CMD_ACK, pkg.m_nSeq);
                m_udp.Send(pkgAck, &m_si);

                // 清空队列
                m_mpRecv.clear();
                m_mpSend.clear();
                //m_vecPkg.clear();
				m_queueRecv.Clear();

                // 相关数据置空
                ZeroMemory(&m_si, sizeof(m_si));
                m_nSendSeq = 0;
                m_nBufSeq = 0;
                m_nSeq = 0;
                m_bWorking = false;
                
                break;
            }  
            default:  
            {  
                break;  
            }  
        }

        // 遍历发包队列
        m_mtxSned.lock();
        time_t ltime = clock();

		int nSendCount = 0;
        for (auto& pkg:m_mpSend)
        {
            if (nSendCount >= 10)
            {
                break;
            }
            else
            {
                nSendCount++;
            }
            // 未发送的包
            if (pkg.second.m_time == 0)
            {
                logs("把包发走了 seq:%d  len:%d", m_nBufSeq, pkg.second.m_pkg.m_nLen);
                pkg.second.m_time = ltime;
                m_udp.Send(pkg.second.m_pkg, &m_si);
            }
            else if (ltime - pkg.second.m_time > TIME)
            {
                // 重发
                pkg.second.m_time = ltime;
                m_udp.Send(pkg.second, &m_si);
            }
        }
        m_mtxSned.unlock();

        // 将收包队列放入缓冲区

        while (m_mpRecv.find(m_nBufSeq) != m_mpRecv.end())
        {
            m_mtxBuf.lock();
            if (m_queueRecv.IsFull())
            {
                m_mtxBuf.unlock();
                break;
            }
            logs("收到包了 seq:%d  len:%d", m_nBufSeq, m_mpRecv[m_nBufSeq].m_nLen);
            m_queueRecv.Push(m_mpRecv[m_nBufSeq]);

            m_mtxBuf.unlock();
			
            m_mpRecv.erase(m_nBufSeq); // 删除已处理的包

            m_nBufSeq++;
        }
		Sleep(1); // 休眠1毫秒，避免CPU占用过高
    }  
}
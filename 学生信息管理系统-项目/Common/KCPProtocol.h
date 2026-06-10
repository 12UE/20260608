#pragma once
#include <stdint.h>
#include <memory>
#include <string>
#include<iostream>
#define DATA_MTU (1472 - sizeof(uint32_t) * 4)
#define TIME_OUT_RESEND (1000 * 2)

template <typename... Args>
void SocketLog(const char* _fmt, Args... _args)
{
    char buffer[0x100] = { 0 };
    ::snprintf(buffer, sizeof(buffer), _fmt, _args...);
    std::cout << (std::to_string(::GetProcessId(NULL)) + buffer).c_str() << std::endl;
}

extern uint16_t InternetCheckSum(uint16_t* _data, int _length);

enum KCPPackageType
{
    KCP_SYN = 0,
    KCP_ACK = 1,
    KCP_PSH,
    KCP_FIN
};

struct KCPPackage
{
    KCPPackage() {}
    KCPPackage(uint32_t _type, uint32_t _seq) :
        m_type(_type),
        m_seq(_seq)
    {
        m_check = InternetCheckSum((uint16_t*)this, sizeof(KCPPackage));
    }

    KCPPackage(uint32_t _type, uint32_t _seq, const void* _data, uint32_t _len) :
        m_type(_type),
        m_seq(_seq),
        m_data_len(_len)
    {
        ::memcpy(m_data, _data, m_data_len);
        m_check = InternetCheckSum((uint16_t*)this, sizeof(KCPPackage));
    }

    // 要注意去掉 m_check 的计算，不然会一直算不对
    bool Check() 
    {
        // 先存一下校验值
        uint32_t temp = m_data_len;
        m_data_len = 0;
        bool res = temp == InternetCheckSum((uint16_t*)this, sizeof(KCPPackage));
        // 回置
        m_data_len = temp;
        return res;
    }

    uint32_t m_type{ 0 };
    uint32_t m_seq{ 0 };
    uint32_t m_check{ 0 };
    uint32_t m_data_len{ 0 };
    uint8_t m_data[DATA_MTU] = { 0 };
};

// 网际校验和算法
static uint16_t InternetCheckSum(uint16_t* _data, int _length)
{
    uint32_t sum = 0;

    while (_length > 1)
    {
        sum += *_data++;
        _length -= 2;
    }

    if (_length == 1)
    {
        sum += *(uint8_t*)_data;
    }

    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)(~sum);
}

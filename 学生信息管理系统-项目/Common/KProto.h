#pragma once
#include <iostream>
// 1500 - 28 - sizeof(m_nCmd) - sizeof(m_nSeq) - sizeof(m_nCheck) = 1456 - 客户数据的长度信息 = 1452
#define MTU 1452

uint16_t calculate_checksum(const uint8_t* data, size_t length);

enum KCmd
{
	// 确认包
	CMD_ACK = 1,
	// 校验并收包
	CMD_PSH,
	// 关闭连接包
	CMD_FIN,
	// 连接包
	CMD_SYN
};
#pragma pack(push,1)
struct Pakage
{
public:
	Pakage() {}
	Pakage(uint32_t nCmd, size_t nSeq):
		m_nCmd(nCmd),
		m_nSeq(nSeq)
	{}
	Pakage(
		KCmd nCmd,
		size_t nSeq,
		uint8_t* data,
		uint32_t nLen
	):
		m_nCmd(nCmd),
		m_nSeq(nSeq),
		m_nLen(nLen)
	{
		m_nCheck = calculate_checksum(data, nLen);
		memcpy(m_data, data, nLen);
	}

	// 包类型
	uint32_t m_nCmd = 0;
	// 序号
	size_t m_nSeq = 0;
	// 校验值
	uint32_t m_nCheck = 0;
	// 包数据长度
	uint32_t m_nLen = 0;
	// 包数据
	uint8_t  m_data[MTU] = { 0 };
};
#pragma pack(pop)

// 计算网际校验和（用于IPv4、TCP、UDP等）
inline uint16_t calculate_checksum(const uint8_t* data, size_t length) {
	uint32_t sum = 0;

	// 按16位累加
	for (size_t i = 0; i < length; i += 2) {
		if (i + 1 < length) {
			sum += (data[i] << 8) | data[i + 1]; // 高字节在前
		}
		else {
			sum += data[i] << 8; // 处理奇数长度，填充0作为低字节
		}
	}

	// 折叠进位
	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	// 取反得到校验和
	return (uint16_t)~sum;
}
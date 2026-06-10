// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "framework.h"
#include <chrono>
#include <string>       // std::string
#include <vector>       // std::vector
#include <map>          // std::map, std::multimap
#include <unordered_map> // std::unordered_map
#include <algorithm>    // 标准算法
#include <functional>   // std::less, std::equal_to, std::hash
#include <utility>      // std::pair
#include <memory>       // std::allocator
#include <type_traits>  // std::enable_if_t
#include <initializer_list> // std::initializer_list
#include <mutex>        // std::mutex
#include <thread>       // std::thread, std::jthread (C++20)
#include "../../Common/SocketInit.h"
#include"../../Common/Protocals.h"
#include"../../Common/KCP.h"
#include"resource.h"
#include"CClassManagerDlg.h"
#include"CCourseManagerDlg.h"
#include"CEnrollManagerDlg.h"
#include"CStudentManagerDlg.h"
#include"CCombineDlg.h"
#include"CTabSheet.hpp"
#define WM_SOCKET (WM_USER + 1)
#endif //PCH_H
inline std::string UTF8ToANSI(const std::string& utf8Str) {
	// 先转换为宽字符
	int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, nullptr, 0);
	if (wideLen == 0) {
		throw std::runtime_error("UTF-8 to WideChar conversion failed");
	}

	std::vector<wchar_t> wideBuf(wideLen);
	MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wideBuf.data(), wideLen);

	// 再从宽字符转换为 ANSI
	int ansiLen = WideCharToMultiByte(CP_ACP, 0, wideBuf.data(), -1, nullptr, 0, nullptr, nullptr);
	if (ansiLen == 0) {
		throw std::runtime_error("WideChar to ANSI conversion failed");
	}

	std::vector<char> ansiBuf(ansiLen);
	WideCharToMultiByte(CP_ACP, 0, wideBuf.data(), -1, ansiBuf.data(), ansiLen, nullptr, nullptr);

	return std::string(ansiBuf.data(), ansiLen - 1); // 减去 null 终止符
}
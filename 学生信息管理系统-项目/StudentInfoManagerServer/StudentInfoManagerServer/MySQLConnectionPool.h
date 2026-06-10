#pragma once
#include <mysql/mysql.h>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <future>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <iostream>
#include <iomanip>
#include"ThreadPool.h"

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "user32.lib")

// 字符编码转换函数
std::string utf8ToGbk(const std::string& utf8Str) {
    // 计算转换后需要的缓冲区大小
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    if (len == 0) return "";

    // 创建宽字符缓冲区
    std::vector<wchar_t> wstr(len);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wstr.data(), len);

    // 计算GBK需要的缓冲区大小
    len = WideCharToMultiByte(CP_ACP, 0, wstr.data(), -1, NULL, 0, NULL, NULL);
    if (len == 0) return "";

    // 创建GBK缓冲区并转换
    std::vector<char> gbkStr(len);
    WideCharToMultiByte(CP_ACP, 0, wstr.data(), -1, gbkStr.data(), len, NULL, NULL);

    return std::string(gbkStr.data());
}

class MySQLConnectionPool {
private:
    struct MySQLConnection {
        MYSQL* conn;
        bool inUse;

        MySQLConnection() : conn(nullptr), inUse(false) {}
        ~MySQLConnection() {
            if (conn) {
                mysql_close(conn);
                conn = nullptr;
            }
        }
    };

    std::vector<std::unique_ptr<MySQLConnection>> connections;
    std::queue<MySQLConnection*> availableConnections;
    std::mutex mutex;
    std::condition_variable cv;

    std::string host;
    std::string user;
    std::string password;
    std::string database;
    unsigned int port;
    unsigned int poolSize;
    bool initialized;
    bool pingConnection(MYSQL* conn) {
        return mysql_ping(conn) == 0;
    }

    // 重连函数
    bool reconnect(MySQLConnection* conn) {
        mysql_close(conn->conn);
        conn->conn = mysql_init(NULL);
        if (!conn->conn) return false;

        mysql_set_character_set(conn->conn, "gbk");
        return mysql_real_connect(conn->conn, host.c_str(), user.c_str(),
            password.c_str(), database.c_str(), port, NULL, 0) != NULL;
    }
public:
    // 结果集的结构体，存储查询结果
    struct QueryResult {
        std::vector<std::string> columnNames;
        std::vector<std::vector<std::string>> rows;
        bool success;
        std::string errorMessage;

        QueryResult() : success(true) {}
    };

    MySQLConnectionPool(const std::string& host = "localhost",
        const std::string& user = "root",
        const std::string& password = "root",
        const std::string& database = "mysql",
        unsigned int port = 3306,
        unsigned int poolSize = 30)
        : host(host), user(user), password(password), database(database),
        port(port), poolSize(poolSize), initialized(false) {
    }

    ~MySQLConnectionPool() {
        connections.clear(); // 智能指针会自动调用析构函数关闭连接
    }

    bool initialize() {
        std::lock_guard<std::mutex> lock(mutex);

        if (initialized) {
            return true;
        }

        for (unsigned int i = 0; i < poolSize; i++) {
            auto conn = std::make_unique<MySQLConnection>();
            conn->conn = mysql_init(NULL);

            if (!conn->conn) {
                std::cerr << "mysql_init() 失败，无法创建连接 #" << i << std::endl;
                return false;
            }

            // 设置重连选项
            bool reconnect = 1;
            mysql_options(conn->conn, MYSQL_OPT_RECONNECT, &reconnect);

            // 设置连接超时
            int timeout = 7;
            mysql_options(conn->conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

            // 设置读写超时
            int readTimeout = 7;
            int writeTimeout = 7;
            mysql_options(conn->conn, MYSQL_OPT_READ_TIMEOUT, &readTimeout);
            mysql_options(conn->conn, MYSQL_OPT_WRITE_TIMEOUT, &writeTimeout);

            mysql_set_character_set(conn->conn, "gbk");

            if (mysql_real_connect(conn->conn, host.c_str(), user.c_str(),
                password.c_str(), database.c_str(), port, NULL, 0) == NULL) {
                std::cerr << "连接 #" << i << " 失败: " << mysql_error(conn->conn) << std::endl;
                return false;
            }

            availableConnections.push(conn.get());
            connections.push_back(std::move(conn));
        }

        initialized = true;
        return true;
    }

    // 获取一个连接
    MySQLConnection* getConnection() {
        std::unique_lock<std::mutex> lock(mutex);

        while (availableConnections.empty()) {
            cv.wait(lock);
        }

        auto conn = availableConnections.front();
        availableConnections.pop();
        conn->inUse = true;

        // 添加连接检查
        if (!pingConnection(conn->conn)) {
            if (!reconnect(conn)) {
                releaseConnection(conn);
                throw std::runtime_error("Database connection lost and reconnect failed");
            }
        }

        return conn;
    }

    // 释放连接回池
    void releaseConnection(MySQLConnection* conn) {
        std::lock_guard<std::mutex> lock(mutex);

        conn->inUse = false;
        availableConnections.push(conn);

        cv.notify_one();
    }

    // 同步查询并获取结果
    QueryResult executeQuery(const std::string& query) {
        QueryResult result;
        auto conn = getConnection();

        if (mysql_query(conn->conn, query.c_str())) {
            result.success = false;
            result.errorMessage = mysql_error(conn->conn);
            releaseConnection(conn);
            return result;
        }

        MYSQL_RES* res = mysql_store_result(conn->conn);
        if (res == NULL) {
            // 可能是非SELECT查询或出错
            if (mysql_field_count(conn->conn) == 0) {
                // 是UPDATE, INSERT等非SELECT查询
                result.success = true;
            }
            else {
                // 出错了
                result.success = false;
                result.errorMessage = mysql_error(conn->conn);
            }
            releaseConnection(conn);
            return result;
        }

        // 获取列名
        unsigned int numFields = mysql_num_fields(res);
        MYSQL_FIELD* fields = mysql_fetch_fields(res);

        for (unsigned int i = 0; i < numFields; i++) {
            result.columnNames.push_back(fields[i].name);
        }

        // 获取数据行
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            std::vector<std::string> dataRow;
            for (unsigned int i = 0; i < numFields; i++) {
                dataRow.push_back(row[i] ? row[i] : "NULL");
            }
            result.rows.push_back(std::move(dataRow));
        }

        mysql_free_result(res);
        releaseConnection(conn);

        return result;
    }
    // 将 ANSI 编码的 std::string 转换为 UTF-8 编码的 std::string
    std::string ANSIToUTF8(const std::string& ansiStr) {
        // 先转换为宽字符
        int wideLen = MultiByteToWideChar(CP_ACP, 0, ansiStr.c_str(), -1, nullptr, 0);
        if (wideLen == 0) {
            throw std::runtime_error("ANSI to WideChar conversion failed");
        }

        std::vector<wchar_t> wideBuf(wideLen);
        MultiByteToWideChar(CP_ACP, 0, ansiStr.c_str(), -1, wideBuf.data(), wideLen);

        // 再从宽字符转换为 UTF-8
        int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideBuf.data(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8Len == 0) {
            throw std::runtime_error("WideChar to UTF-8 conversion failed");
        }

        std::vector<char> utf8Buf(utf8Len);
        WideCharToMultiByte(CP_UTF8, 0, wideBuf.data(), -1, utf8Buf.data(), utf8Len, nullptr, nullptr);

        return std::string(utf8Buf.data(), utf8Len - 1); // 减去 null 终止符
    }
    // 异步查询
    std::future<QueryResult> executeQueryAsync(const std::string& query) {
        //return std::async(std::launch::async, [this, query]() {
        //    return this->executeQuery(query);
        //    });
        return ThreadPool::GetInstance().enqueue([this, query]() {
            return this->executeQuery(query);
        });
    }

    // 打印查询结果，确保转换为GBK格式
    static void printQueryResult(const QueryResult& result) {
        if (!result.success) {
            std::cout << "查询失败: " << utf8ToGbk(result.errorMessage) << std::endl;
            return;
        }

        // 打印列名
        for (const auto& colName : result.columnNames) {
            std::cout << std::left << std::setw(15) << utf8ToGbk(colName) << "\t";
        }
        std::cout << std::endl;

        // 打印数据
        for (const auto& row : result.rows) {
            for (const auto& col : row) {
                std::cout << std::left << std::setw(15) << utf8ToGbk(col) << "\t";
            }
            std::cout << std::endl;
        }

        std::cout << "总共 " << result.rows.size() << " 行记录" << std::endl;
    }
};
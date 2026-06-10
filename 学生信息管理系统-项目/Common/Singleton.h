#pragma once
#pragma once
template<typename T>
class Singleton {
public:
    static T& GetInstance() {
        // C++11保证局部静态变量的初始化是线程安全的
        static T instance;
        return instance;
    }
};


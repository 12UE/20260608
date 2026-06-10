#pragma once

#include <assert.h>
#include <stdexcept>

/**
 * 循环队列
 */
template<typename T>
class CircleQueue final
{
public:
    CircleQueue(size_t _size = 100)
    {
        Create(_size);
    }

    ~CircleQueue()
    {
        Destroy();
    }

    CircleQueue(const CircleQueue& _other)
    {
        *this = _other;
    }

    CircleQueue(CircleQueue&& _other)
    {
        *this = std::move(_other);
    }

    CircleQueue& operator=(const CircleQueue& _other)
    {
        if (this != &_other)
        {
            Destroy();
            Create(_other.m_max_element_size);

            for (int i = 0; i < _other.m_element_size; i++)
            {
                size_t target_index = (_other.m_head + i) % _other.m_max_element_size;
                new (&m_queue[i]) T(_other.m_queue[target_index]);
                m_element_size++;
            }
        }

        return *this;
    }

    CircleQueue& operator=(CircleQueue&& _other)
    {
        if (this != &_other)
        {
            Destroy();

            m_queue = _other.m_queue;
            m_head = _other.m_head;
            m_element_size = _other.m_element_size;
            m_max_element_size = _other.m_max_element_size;

            _other.m_queue = 0;
            _other.m_element_size = 0;
            _other.m_max_element_size = 0;
            _other.m_head = 0;
        }

        return *this;
    }

public:
    // 判断是否队空
    bool IsEmpty() const
    {
        return m_element_size == 0;
    }
    // 判断是否队满
    bool IsFull() const
    {
        return m_element_size == m_max_element_size;
    }

    // 入队
    void Push(const T& _data)
    {
        // 判断是否队满
        if (IsFull())
        {
            throw std::runtime_error("Queue is full");
        }
        
        size_t target_index = (m_head + m_element_size) % m_max_element_size;
        // 为了执行构造函数赋值虚表
        ::new (&m_queue[target_index]) T(_data);
        
        m_element_size++;
    }

    // 出队
    const T& Pop()
    {
        // 判断是否队空
        if (IsEmpty())
        {
            throw std::runtime_error("Queue is full");
        }

        size_t target_index = m_head;
        m_head = (m_head + 1) % m_max_element_size;
        m_element_size--;
        
        return m_queue[target_index];
    }

    // 查看队头元素
    const T& Front() const
    {
        // 判断是否队空
        if (IsEmpty())
        {
            throw std::runtime_error("Queue is full");
        }

        return m_queue[m_head];
    }

    // 查看队尾元素
    const T& Back() const
    {
        // 判断是否队空
        if (IsEmpty())
        {
            throw std::runtime_error("Queue is full");
        }

        size_t target_index = (m_head + m_element_size - 1 + m_max_element_size) % m_max_element_size;
        return m_queue[target_index];
    }

    // 清空队列
    void Clear()
    {
        // 需执行有效对象析构函数
        for (int i = 0; i < m_element_size; i++)
        {
            size_t target_index = (m_head + i) % m_max_element_size;
            m_queue[target_index].~T();
        }

        m_element_size = 0;
        m_head = 0;
    }

    size_t GetElementSize() const
    {   
        return m_element_size;
    }

    size_t GetMaxElementSize() const
    {
        return m_max_element_size;
    }

    size_t GetFreeElementSize() const
    {
        return m_max_element_size - m_element_size;
    }

    // 此函数最好用于基本数据类型还有不需要深拷贝的类
    void MemoryInsert(T* _buffer, size_t _ele_cnt)
    {
        // 空间不足了
        if (m_max_element_size - m_element_size < _ele_cnt)
        {
            throw std::runtime_error("Memory is too small");
        }

        // 准备拷贝
        size_t start_index = (m_head + m_element_size) % m_max_element_size;
        size_t end_index = start_index + _ele_cnt;

        m_element_size += _ele_cnt;

        if (end_index < m_max_element_size)
        {
            // 不用分开拷贝
            ::memcpy(&m_queue[start_index], _buffer, sizeof(T) * _ele_cnt);
        }
        else
        {
            size_t first_copy_size = m_max_element_size - start_index;
            ::memcpy(&m_queue[start_index], _buffer, first_copy_size * sizeof(T));
            ::memcpy(&m_queue[0], _buffer + first_copy_size, (_ele_cnt - first_copy_size) * sizeof(T));
        }
    }


    void MemoryPopFromHead(T* _buffer, size_t _ele_cnt)
    {
        MemoryGetFromHead(_buffer, _ele_cnt);

        // 没有那么多可用元素
        if (m_element_size < _ele_cnt)
        {
            throw std::runtime_error("m_element_size < _ele_cnt");
        }
        m_head = (m_head + _ele_cnt) % m_max_element_size;
        m_element_size -= _ele_cnt;
    }


    // 从头拷贝
    void MemoryGetFromHead(T* _buffer, size_t _ele_cnt)
    {
        // 没有那么多可用元素
        if (m_element_size < _ele_cnt)
        {
            throw std::runtime_error("m_element_size < _ele_cnt");
        }
        // 准备拷贝
        size_t start_index = m_head;
        size_t end_index = start_index + _ele_cnt;

        if (end_index < m_max_element_size)
        {
            // 不用分开拷贝
            ::memcpy(_buffer, &m_queue[start_index], sizeof(T) * _ele_cnt);
        }
        else
        {
            size_t first_copy_size = m_max_element_size - start_index;
            ::memcpy(_buffer, &m_queue[start_index] , first_copy_size * sizeof(T));
            ::memcpy(_buffer + first_copy_size, &m_queue[0], (_ele_cnt - first_copy_size) * sizeof(T));
        }
    }

    // 从头移除
    void MemoryRemoveFromHead(size_t _ele_cnt)
    {
        // 没有那么多可用元素
        if (m_element_size < _ele_cnt)
        {
            throw std::runtime_error("m_element_size < _ele_cnt");
        }
        m_head = (m_head + _ele_cnt) % m_max_element_size;
        m_element_size -= _ele_cnt;
    }

private:
    // 初始化
    void Create(size_t _size)
    {
        if (m_queue)
        {
            Destroy();
        }
        m_queue = (T*)malloc(_size * sizeof(T));
        m_max_element_size = _size;
    }

    // 销毁
    void Destroy()
    {
        Clear();
        ::free(m_queue);
        m_queue = nullptr;
        m_max_element_size = 0;
        m_element_size = 0;
    }

private:
    T* m_queue = nullptr;
    size_t m_max_element_size = 0;
    size_t m_element_size = 0 ;
    size_t m_head = 0;
};
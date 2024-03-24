#pragma once

#include "hzpch.h"
#include "Hazel/Core/Allocation/Allocator/DefaultHeapAllocator.h"
#include "Hazel/Core/Allocation/Allocator/GeneralAllocator.h"
#include "Hazel/Core/Thread/ThreadUtil.h"
namespace Hazel
{

template <typename T, typename TAllocator = AlignedAllocator<T>>
class HAZEL_API RingBuffer
{
public:
    RingBuffer() : m_Capacity(10)
    {
        m_Lock = ThreadUtils::CreateCritSect();
        Reserve(m_Capacity);
    }


    RingBuffer(size_t capacity) : m_Capacity(capacity)
    {
        m_Lock = ThreadUtils::CreateCritSect();
        Reserve(capacity);
    }

    ~RingBuffer()
    {
        Clear();
        m_Allocator.Free(m_Data);
        m_Data = nullptr;
        ThreadUtils::DestroyCritSect(m_Lock);
    }

    inline void Reserve(size_t capacity)
    {
        ThreadUtils::LockCritSect(m_Lock);
        m_Capacity = capacity;
        m_Data = static_cast<T *>(
            m_Allocator.Allocate(capacity, __FILE__, __LINE__));
        ThreadUtils::UnlockCritSect(m_Lock);
    }

    inline bool Enqueue(const T &t)
    {
        bool r = false;
        ThreadUtils::LockCritSect(m_Lock);
        if (m_Count >= m_Capacity - 1)
        {
            grow();
        }

        if (m_Count < m_Capacity)
        {
            new (&m_Data[m_Tail]) T(t);
            m_Tail = (m_Tail + 1) % m_Capacity;
            m_Count++;
            r = true;
        }
        ThreadUtils::UnlockCritSect(m_Lock);
        return r;
    }

    inline bool Enqueue(T &&t)
    {
        bool r = false;
        ThreadUtils::LockCritSect(m_Lock);
        if (m_Count >= m_Capacity - 1)
            grow();

        if (m_Count < m_Capacity)
        {
            new (&m_Data[m_Tail]) T(std::forward<T>(t));
            m_Tail = (m_Tail + 1) % m_Capacity;
            m_Count++;
            r = true;
        }

        ThreadUtils::UnlockCritSect(m_Lock);
        return r;
    }

    inline bool Dequeue(T &t)
    {
        bool r = false;
        ThreadUtils::LockCritSect(m_Lock);
        if (m_Count > 0)
        {
            T ret = m_Data[m_Head];
            t = ret;
            m_Data[m_Head].~T();
            m_Head = (m_Head + 1) % m_Capacity;
            m_Count--;
            r = true;
        }

        ThreadUtils::UnlockCritSect(m_Lock);
        return r;
    }

    inline void Clear()
    {
        ThreadUtils::LockCritSect(m_Lock);
        while (m_Count > 0)
        {
            m_Data[m_Head].~T();
            m_Head = (m_Head + 1) % m_Capacity;
            m_Count--;
        }

        m_Head = 0;
        m_Tail = 0;
        m_Count = 0;
        ThreadUtils::UnlockCritSect(m_Lock);
    }

    inline size_t Count()
    {
        size_t r = 0;
        ThreadUtils::LockCritSect(m_Lock);
        r = m_Count;

        ThreadUtils::UnlockCritSect(m_Lock);
        return r;
    }

    inline bool IsEmpty()
    {
        bool r = false;
        ThreadUtils::LockCritSect(m_Lock);
        r = m_Count == 0;

        ThreadUtils::UnlockCritSect(m_Lock);
        return r;
    }

    inline size_t Capacity()
    {
        size_t r = 0;
        ThreadUtils::LockCritSect(m_Lock);
        r = m_Capacity;

        ThreadUtils::UnlockCritSect(m_Lock);
        return r;
    }

    inline bool Peek(T &t)
    {
        bool r = false;
        ThreadUtils::LockCritSect(m_Lock);
        if (m_Count > 0)
        {
            t = m_Data[m_Head];
            r = true;
        }

        ThreadUtils::UnlockCritSect(m_Lock);
        return r;
    }

private:
    T *m_Data;
    TAllocator m_Allocator;

    size_t m_Count = 0;
    size_t m_Capacity = 0;

    size_t m_Head = 0;
    size_t m_Tail = 0;


    CRIC_SECT *m_Lock;
    inline virtual void grow()
    {
        size_t prev = m_Capacity;

        m_Capacity *= 2;
        m_Data = static_cast<T *>(
            m_Allocator.Reallocate(m_Data, m_Capacity, __FILE__, __LINE__));

        //       tail        head
        //       v           v
        // | 5 | 6 | x | x | 3 | 4 |
        // | 5 | 6 | x | x | 3 | 4 |   |   |   |   |   |   <- Glow Capacity * 2
        // | 5 | 6 | x | x |   |   |   |   |   | 3 | 4 |   <- The below logic executed.

        if (m_Head > m_Tail)
        {
            size_t len = prev - m_Head;
            for (int i = 0; i < len; ++i)
                new (&m_Data[(m_Capacity - len) + i])
                    T(std::move(m_Data[m_Head + i]));

            m_Head = m_Capacity - len;
        }
    }
};
} // namespace Hazel

/*
template<typename T, size_t InlineCapacity>
class LvFixedRingBuffer : public LvRingBuffer<T, LvFiniteAllocator<T, InlineCapacity>>
{
	typedef LvRingBuffer<T, LvFiniteAllocator<T, InlineCapacity>> Base;

public:

	LvFixedRingBuffer()
		: Base(InlineCapacity)
	{

	}

private:

	LV_FORCEINLINE void grow() override
	{
	}
};
*/
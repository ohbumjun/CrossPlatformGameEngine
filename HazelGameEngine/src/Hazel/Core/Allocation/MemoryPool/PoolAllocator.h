#pragma once

#include "hzpch.h"
#include "MemoryPoolAllocator.h"
#include "Hazel/Core/DataStructure/StackLinkedList.h"

class PoolAllocator :
    public MemoryPoolAllocator
{
    friend class CMemoryPool;
private:
    struct FreeHeader {};
    typedef StackLinkedList<FreeHeader>::Node Node;
    StackLinkedList<FreeHeader> m_FreeList;
    std::stack<size_t> m_StkAddress;
    void* m_StartPtr;
    size_t m_ChunkSize;
public:
    PoolAllocator(const size_t totalSize,
        const size_t chunkSize);
    virtual ~PoolAllocator();
    virtual void* Allocate(const size_t allocateSize,
        const size_t alignment);
    virtual void Free(void* ptr) override;
    virtual void Init() override;
    virtual void Reset();
};


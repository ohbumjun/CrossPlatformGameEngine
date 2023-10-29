#include "hzpch.h"
#include "GCAllocator.h"
#include "GCObject.h"
#include "Hazel/Core/Allocation/MemoryPool/MemoryPoolManager.h"
#include "Hazel/Core/Allocation/MemoryPool/PoolAllocator.h"
#include "../../Utils/EngineUtil.h"
#include "Hazel/Core/Reflection/Reflection.h"
#include <limits>  /* limits_max */

#define INF int(1e9)

GCAllocator::GCAllocator(size_t totalSize)
	: m_TotalSize(totalSize)
{
	m_memoryPoolManager = new MemoryPoolManager();
	m_memoryPoolManager->CreatePoolAllocator<GCObject>("GCObject", 10000);
	m_GCObjectPool = dynamic_cast<PoolAllocator*>(m_memoryPoolManager->FindPoolAllocMemoryPool("GCObject"));
}

GCAllocator::~GCAllocator()
{
	delete m_StartPtr;
	delete m_memoryPoolManager;

	m_StartPtr = nullptr;
	m_memoryPoolManager = nullptr;
	m_GCObjectPool = nullptr;
}

template<typename T>
// void* GCAllocator::Allocate(const size_t allocSize)
GCObject* GCAllocator::Allocate(bool isRoot)
{
	void* dataPtr = allocate<T>();
	const std::size_t dataAddresss = (std::size_t)dataPtr;
	const std::size_t headerAddress = dataAddresss - sizeof(GCAllocationHeader);

	const GCAllocationHeader* allocHeader((GCAllocationHeader*)headerAddress);
	allocHeader->isRoot = isRoot;

	TypeInfo* typeInfo = Reflection::GetTypeInfo<T>();

	// typeInfo 가 존재해야만 한다.
	assert(typeInfo);

	void* newMemory = m_GCObjectPool->Allocate(sizeof(GCObject));
	GCObject* gcObject = new (newMemory) GCObject(newMemory, allocHeader, typeInfo);

	return gcObject;
}

void GCAllocator::Free(GCObject* gcObject)
{
	// 먼저 DataPtr 을 메모리 해제 한다.
	const void* dataPtr = gcObject->GetDataPtr();
	free(const_cast<void*>(dataPtr));

	// GCObject 메모리 해제
	m_GCObjectPool->Free(gcObject);
}

void GCAllocator::Init()
{
	if (m_StartPtr == nullptr)
		delete m_StartPtr;

	m_StartPtr = new byte[m_TotalSize];

	Reset();
}

void GCAllocator::Reset()
{
	m_Used = 0;
	m_Peak = 0;

	Node* firstNode = (Node*)m_StartPtr;
	firstNode->data.blockSize = m_TotalSize;
	firstNode->next = nullptr;
	firstNode->prev = nullptr;

	m_FreeList.m_Head = nullptr;
	m_FreeList.insert(nullptr, firstNode);

	m_SizeAVLTree.Clear();
}

void GCAllocator::coalescene(Node* prevBlock, Node* freeBlock)
{
	if (freeBlock->next != nullptr &&
		(std::size_t)freeBlock + freeBlock->data.blockSize == (size_t)freeBlock->next)
	{
		// freeBlock 정보를 update 해야 한다.
		// 따라서 기존의 freeBlock Block 정보를 Tree 에서 지워준다.
		m_SizeAVLTree.Erase(freeBlock->data.blockSize, freeBlock);

		// freeBlock->next 에 해당하는 것을 지운다.
		m_SizeAVLTree.Erase(freeBlock->next->data.blockSize, freeBlock->next);

		freeBlock->data.blockSize += freeBlock->next->data.blockSize;
		m_FreeList.remove(freeBlock, freeBlock->next);

		// update 된 새로운 freeBlock 을 넣어준다.
		m_SizeAVLTree.Insert(freeBlock->data.blockSize, freeBlock);
	}

	if (prevBlock != nullptr &&
		(size_t)prevBlock + prevBlock->data.blockSize == (std::size_t)freeBlock)
	{
		// prevBlock 정보를 update 해야 한다.
		// 따라서 기존의 prev Block 정보를 Tree 에서 지워준다.
		m_SizeAVLTree.Erase(prevBlock->data.blockSize, prevBlock);

		prevBlock->data.blockSize += freeBlock->data.blockSize;

		m_FreeList.remove(prevBlock, freeBlock);

		// update 된 새로운 prevBlock 정보를 등록해준다.
		m_SizeAVLTree.Insert(prevBlock->data.blockSize, prevBlock);

		// 한편 freeBlock 정보는 지워준다.
		m_SizeAVLTree.Erase(freeBlock->data.blockSize, freeBlock);
	}
}

void GCAllocator::find(const size_t allocSize, const size_t alignment, size_t& padding, Node*& prevNode, Node*& foundNode)
{
	findSpeed(allocSize, alignment, padding, prevNode, foundNode);
}

void GCAllocator::findSpeed(const size_t allocSize, const size_t alignment, size_t& padding, Node*& prevNode, Node*& foundNode)
{
	assert(m_SizeAVLTree.Size() > 0);

	Node* findNode = nullptr;

	size_t minValue = static_cast<size_t>(-1);

	m_SizeAVLTree.Find([&findNode, allocSize, &minValue, &padding, alignment](const AVLTreeNode<size_t, void*>* node)
		{
			// node->second 는 AVL Tree 상의 node
			// 반면, Node 는, DoublyLinkedList 상의 Node
			Node* iter = reinterpret_cast<Node*>(node->second);

			padding = EngineUtil::CalculatePaddingWithHeader((size_t)iter, alignment, sizeof(GCAllocationHeader));

			size_t requireSize = padding + allocSize;

			// 만족하는 가장 최적의 크기를 찾는다.
			if (node->first >= requireSize)
			{
				if (node->first < minValue)
				{
					minValue = node->first;
					findNode = iter;
					// 더 작은 최적의 메모리를 찾아 들어간다
					return -1;
				}
			}
			else
			{
				// node->first < requireSize
				// 더 큰 블록을 찾아들어간다.
				return 1;
			}

			return 0;
		});


	assert(findNode); // 할당하지 않은 메모리를 해제한 것일 수 있다.

	prevNode = findNode->prev;
	foundNode = findNode;
}

// freeList insert, remove 부분들을 아래 함수들로 대체해야 한다.
void GCAllocator::insertNode(Node* prev, Node* current)
{
	m_FreeList.insert(prev, current);

	m_SizeAVLTree.Insert(current->data.blockSize, current);
}

void GCAllocator::removeNode(Node* prev, Node* current)
{
	m_FreeList.remove(prev, current);

	m_SizeAVLTree.Erase(current->data.blockSize, current);
}

template<typename T>
void GCAllocator::allocate()
{
	size_t allocSize = sizeof(T);
	const size_t allocHeaderSize = sizeof(GCAllocationHeader);
	// const size_t freeHeaderSize = sizeof(FreeListAllocator::FreeHeader);

	size_t nodeSize = sizeof(Node);
	// assert(allocSize >= nodeSize);
	assert(allocSize >= 8);

	// Free Memory Block 을 돌면서
	// 최적의 Free Memory Block 을 찾는다.
	std::size_t padding;
	Node* affectedNode = nullptr, * prevNode = nullptr;

	find(allocSize, m_Alignment, padding, prevNode, affectedNode);

	// 부족할시 메모리 추가할당하기 (여러 개 Area 로 나눠서 FreeList Allocator 로직 진행하기)
	assert(affectedNode != nullptr && "Not Enough Memory");

	const size_t alignmentPadding = padding - allocHeaderSize;
	const size_t requiredSize = allocSize + padding;

	const size_t rest = affectedNode->data.blockSize - requiredSize;

	// split block into data block, and a free block of size 'rest'
	Node* newFreeNode = (Node*)((std::size_t)affectedNode + requiredSize);

	if (rest > 0)
	{
		newFreeNode->data.blockSize = rest;
		newFreeNode->next = nullptr;


		// m_FreeList.insert(affectedNode, newFreeNode);
		insertNode(affectedNode, newFreeNode);
	}

	// m_FreeList.remove(prevNode, affectedNode);
	removeNode(prevNode, affectedNode);

	// set up date block
	const std::size_t headerAddress = (size_t)affectedNode + alignmentPadding;
	const std::size_t dataAdderess = headerAddress + allocHeaderSize;

	GCAllocationHeader* HeaderPtr = (GCAllocationHeader*)headerAddress;

	HeaderPtr->blockSize = requiredSize;

	// headerAddress 로부터, padding 만큼 이전에 가면
	// 가장 마지막에 할당된 datablock 의 끝 위치가 나온다는 의미
	HeaderPtr->padding = (char)alignmentPadding;

	m_Used += requiredSize;
	m_Peak = m_Used;

	return (void*)dataAdderess;
}

void GCAllocator::free(void* ptr)
{
	// Insert It in a sorted position by the address number
	// - Data Address 
	const std::size_t currentAddresss = (std::size_t)ptr;
	const std::size_t headerAddress = currentAddresss - sizeof(GCAllocationHeader);

	const GCAllocationHeader* allocHeader((GCAllocationHeader*)headerAddress);

	/*
	Node* freeNode = (Node*)(headerAddress);
	freeNode->data.blockSize = allocHeader->blockSize + allocHeader->padding;
	*/
	Node* freeNode = (Node*)((std::size_t)headerAddress - allocHeader->padding);
	freeNode->data.blockSize = allocHeader->blockSize;

	freeNode->next = nullptr;

	Node* iter = m_FreeList.m_Head;
	Node* iterPrev = nullptr;

	while (iter != nullptr)
	{
		if (ptr < iter)
		{
			m_FreeList.insert(iterPrev, freeNode);
			break;
		}
		iterPrev = iter;
		iter = iter->next;
	}

	m_Used -= freeNode->data.blockSize;

	// Tree 정보에 새로 생성한 freeNode 정보를 넣어준다.
	m_SizeAVLTree.Insert(freeNode->data.blockSize, freeNode);

	// Merge Contiguous Nodes
	coalescene(iterPrev, freeNode);
}
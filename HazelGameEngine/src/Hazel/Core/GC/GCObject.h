#pragma once

#include "../Reflection/Reflection.h"
#include "GCInfo.h"

struct TypeInfo;

class GCObject
{
public :
	GCObject(void* data, GCAllocationHeader* header, TypeInfo* pTypeInfo);
	
	void SetIsRoot(bool isRoot);
	void SetVisit(bool visit);
	void AddRef();
	void SetNext(GCObject* current, GCObject* next);
	// nullptr ���� Ȥ�� Dangling ������ �˻��ϴ� �Լ�
	bool IsValid();
	
	inline TypeInfo* GetTypeInfo() const { return m_TypeInfo; }
	inline GCObject* GetNext() const { return m_Next; }
	inline void* GetDataPtr() const { return m_DataPtr; }
	inline bool IsVisited() { return false; }
	inline bool IsRoot() { return false; }
	inline int   GetRefCount() { return 0; }
	inline TypeId GetType() { return m_TypeInfo->m_Type; };
private:
	// _object_db_rec_ ���� ���Ḯ��Ʈ ���·� ������ ���̴�.
	// �� ���� ��忡 ���� ������
	GCObject* m_Next;

	// �Ҵ��� object �� �޸� �ּ� 
	// _object_db_ ���� �ش� object_rec �� ã�� key �� ����� ���̴�.
	void* m_DataPtr;

	// ���� HeaderAddress �� ������ ���� �� �ְ� �ؾ� �Ѵ�.
	// void* headerAddr;
	GCAllocationHeader* m_HeaderAddr;

	// �ش� object �� ���� structure_db_rec ��ü�� ����Ű�� ������
	// xalloc �� ���� "string" ���� �̿��Ͽ� struct_db ���� ã�Ƴ��� ��ũ�����ֱ�
	// struct_db_rec_t* struct_rec;
	TypeInfo* m_TypeInfo;

	// �Ʒ� �������� Header �ʿ� ��ġ�غ���.
	// bool is_visited; /*Used for Graph traversal*/
	// bool is_root;
	// int   refCount;  /*Used for Graph traversal*/
};


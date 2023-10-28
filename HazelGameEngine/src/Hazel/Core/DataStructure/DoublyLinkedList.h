#pragma once

template<typename T>
class DoublyLinkedList
{
public:
	struct Node
	{
		T data;
		Node* next = nullptr;
		Node* prev = nullptr;
	};
	Node* m_Head;

public:
	void insert(Node* prevNode, Node*& newNode);
	void remove(Node* prevNode, Node*& deleteNode);
};

template<typename T>
inline void DoublyLinkedList<T>::insert(Node* prevNode,
	Node*& newNode)
{
	if (prevNode == nullptr)
	{
		// ���� ���� newNode �� ù��° Node ���
		newNode->next = m_Head;

		if (m_Head)
		{
			m_Head->prev = newNode;
		}

		// newNode (= new Head) -> old Head
		// inserting to 'front'
		m_Head = newNode;
	}
	else
	{
		// prevNode ������ newNode �� ���� ���̴�.
		if (prevNode->next == nullptr)
		{
			prevNode->next = newNode;
			newNode->next = nullptr;

			newNode->prev = prevNode;
		}
		else
		{
			// newNode  ~ prev->next ����
			newNode->next = prevNode->next;
			prevNode->next->prev = newNode;

			// prev ~ newNode ����
			prevNode->next = newNode;
			newNode->prev = prevNode;
		}
	}
}

// deleteNode �� List �󿡼� �����ִ� ȿ���̴�.
template<typename T>
inline void DoublyLinkedList<T>::remove(Node* prevNode,
	Node*& deleteNode)
{
	if (prevNode == nullptr)
	{
		// List �� ���� �ϳ��� ���Ҹ��� ���ϰ� �ִٴ� �ǹ�
		if (deleteNode->next == nullptr)
		{
			// ���⿡���� ��ǻ� deleteNode == m_Head ���� ��
			m_Head = nullptr;
		}
		else
		{
			// List �� �������� ���Ҹ� ������ �ִٴ� �ǹ� 
			// �׸��� list ���� ���� ù��° ��带 ����� �ִ� ��쿡 �ش��Ѵ�.
			deleteNode->prev = nullptr;
			
			m_Head = deleteNode->next;
			
		}
	}
	else
	{
		// prevNode -> deleteNode -> Next
		// => (prevNode -> Next)
		prevNode->next = deleteNode->next;
		deleteNode->next->prev = prevNode;
	}
}

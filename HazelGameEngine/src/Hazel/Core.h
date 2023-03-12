#pragma once

// �Ʒ��� ��ũ�ε��� Engine �� ����
// ���� -> c++ -> ��ó����. �߰�

/*
__declspec(dllimport) : �ش� �ڵ尡 dll �κ��� import �� ���̶�� ���� ǥ��

Engine ������Ʈ�� HZ_BUILD_DLL �� ���ǵǾ� �ִ�. export �� �� ���̰�
Client �� ���� x, ���� import �� ���̴�.
*/
#ifdef HZ_PLATFORM_WINDOWS
	#ifdef HZ_BUILD_DLL
		#define HAZEL_API __declspec(dllexport)
	#else 
		#define HAZEL_API __declspec(dllimport)
	#endif
#else 
	#error Hazel only support windows
#endif

#define BIT(x) (1 << x)
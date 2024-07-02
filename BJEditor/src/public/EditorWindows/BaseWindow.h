#pragma once
#include "Editor.h"

namespace HazelEditor
{
    // ���� ���� ���������� Window Class �� Wrapping �ϴ�
    // Class �� �����ϴ� ���� ���� �� ����.
	class BaseWindow
	{
    public:
        void Init();

        void Open();

        void Close();

        void NextFrame();

        void Present();

        // Editor Run Method
        virtual void PeekEvent();
        // Editor Run Method
        virtual void Update(float deltaTime);
    protected:
        virtual void onInit();

        virtual void onUpdate(float deltatime);

        virtual void onOpen();

        virtual void onClose();

        virtual void onGUI();

        virtual void onPrepare(float deltaTime);

        virtual void onRender(float deltaTime);

        virtual void onNextFrame();

        virtual void onPresent();
        
	    void prepare(float deltaTime);

        void render(float deltaTime);
	};
};


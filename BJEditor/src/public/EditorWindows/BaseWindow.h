#pragma once
#include "Editor.h"

namespace HazelEditor
{
    // 현재 엔진 구조에서는 Window Class 를 Wrapping 하는
    // Class 로 구현하는 것이 좋을 것 같다.
	class BaseWindow
	{
    public:
        void Init();

        void Open();

        void Close();

        void NextFrame();

        void Present();

        void Prepare();

        // Editor Run Method
        virtual void PeekEvent();
        // Editor Run Method
        virtual void Update(float deltaTime);
    protected:
        inline virtual void onInit();

        inline virtual void onOpen();

        inline virtual void onClose();

        inline virtual void onGUI();

        inline virtual void onPrepare(float deltaTime);

        inline virtual void onRender(float deltaTime);

        inline virtual void onNextFrame();

        inline virtual void onPresent();
        
	    void prepare(float deltaTime);

        void render(float deltaTime);
	};
};


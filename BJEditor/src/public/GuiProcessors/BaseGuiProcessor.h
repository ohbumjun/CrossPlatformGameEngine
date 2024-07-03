#pragma once
#include "Editor.h"

namespace HazelEditor
{
// 현재 엔진 구조에서는 Window Class 를 Wrapping 하는
// Class 로 구현하는 것이 좋을 것 같다.
class BaseGuiProcessor
{
public:
    void Init();

    void Open();

    void Close();

    // @brief Update 전에 호출되는 함수
    void NextFrame();

    // @brief Frame 을 정리해주는 함수
    void EndFrame();

    void Present();

    // Editor Run Method
    virtual void Update(float deltaTime);

protected:
    virtual void onInit();

    virtual void onUpdate(float deltatime);

    virtual void onOpen();

    virtual void onEndFrame();

    virtual void onClose();

    virtual void onGUI();

    virtual void onPrepare(float deltaTime);

    virtual void onRender(float deltaTime);

    virtual void onNextFrame();

    virtual void onPresent();

    void prepare(float deltaTime);

    void render(float deltaTime);
};
}; // namespace HazelEditor

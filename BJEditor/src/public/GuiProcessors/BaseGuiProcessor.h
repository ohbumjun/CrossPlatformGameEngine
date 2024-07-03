#pragma once
#include "Editor.h"

namespace HazelEditor
{
// ���� ���� ���������� Window Class �� Wrapping �ϴ�
// Class �� �����ϴ� ���� ���� �� ����.
class BaseGuiProcessor
{
public:
    void Init();

    void Open();

    void Close();

    // @brief Update ���� ȣ��Ǵ� �Լ�
    void NextFrame();

    // @brief Frame �� �������ִ� �Լ�
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

#pragma once
#include "EditorWindows/BaseWindow.h"
#include <vector>

namespace HazelEditor
{

class GuiWindow : public BaseWindow
{
public:
    void Update(float deltatime) override;

protected:
    void onInit() override;

    void onRender(float deltaTime) override;

    void onGUI() override;

    void onNextFrame() override;

    void onPresent() override;

    virtual void onPrepareGUI();

    virtual void onFinishGUI();


    std::vector<GuiWindow *> _childs;
};
}; // namespace HazelEditor

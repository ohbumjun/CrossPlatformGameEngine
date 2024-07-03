#pragma once
#include "GuiProcessors/BaseGuiProcessor.h"
#include <vector>

namespace HazelEditor
{

class GuiProcessor : public BaseGuiProcessor
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


    std::vector<GuiProcessor *> _childs;
};
}; // namespace HazelEditor

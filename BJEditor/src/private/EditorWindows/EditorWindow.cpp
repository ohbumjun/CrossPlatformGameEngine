#include "hzpch.h"
#include "EditorWindows/EditorWindow.h"

namespace HazelEditor
{

void GuiWindow::Update(float deltatime)
{
    onUpdate(deltatime);
}
void GuiWindow::onInit()
{
}
void GuiWindow::onRender(float deltaTime)
{
}
void GuiWindow::onGUI()
{
    for (size_t i = 0; i < _childs.size(); ++i)
    {
        _childs[i]->onGUI();
    }
}
void GuiWindow::onNextFrame()
{
}
void GuiWindow::onPresent()
{
}
void GuiWindow::onPrepareGUI()
{
}
void GuiWindow::onFinishGUI()
{
    for (size_t i = 0; i < _childs.size(); ++i)
    {
        _childs[i]->onFinishGUI();
    }
}
};


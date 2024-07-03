#pragma once
#include "EditorWindows/BaseWindow.h"

namespace HazelEditor
{
void BaseWindow::Init()
{
    onInit();
}
void BaseWindow::Open()
{
    onOpen();
}
void BaseWindow::Close()
{
    onClose();
}
void BaseWindow::NextFrame()
{
    onNextFrame();
}
void BaseWindow::EndFrame()
{
    onEndFrame();
}
void BaseWindow::Present()
{
    onPresent();
}
void BaseWindow::PeekEvent()
{
}
void BaseWindow::Update(float deltaTime)
{
    prepare(deltaTime);
    onGUI();
}
void BaseWindow::onInit()
{
} 
void BaseWindow::onUpdate(float deltatime)
{
}
void BaseWindow::onOpen()
{
}
void BaseWindow::onEndFrame()
{
}
void BaseWindow::onClose()
{
}
void BaseWindow::onGUI()
{
}
void BaseWindow::onPrepare(float deltaTime)
{
}
void BaseWindow::onRender(float deltaTime)
{
}
void BaseWindow::onNextFrame()
{
}
void BaseWindow::onPresent()
{
}
void BaseWindow::prepare(float deltaTime)
{
    onPrepare(deltaTime);
}
void BaseWindow::render(float deltaTime)
{
    onRender(deltaTime);
}
};


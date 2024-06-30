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
inline void BaseWindow::onInit()
{
}
inline void BaseWindow::onUpdate(float deltatime)
{
}
inline void BaseWindow::onOpen()
{
}
inline void BaseWindow::onClose()
{
}
inline void BaseWindow::onGUI()
{
}
inline void BaseWindow::onPrepare(float deltaTime)
{
}
inline void BaseWindow::onRender(float deltaTime)
{
}
inline void BaseWindow::onNextFrame()
{
}
inline void BaseWindow::onPresent()
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


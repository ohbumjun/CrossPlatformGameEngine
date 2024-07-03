#pragma once
#include "GuiProcessors/BaseGuiProcessor.h"

namespace HazelEditor
{
void BaseGuiProcessor::Init()
{
    onInit();
}
void BaseGuiProcessor::Open()
{
    onOpen();
}
void BaseGuiProcessor::Close()
{
    onClose();
}
void BaseGuiProcessor::NextFrame()
{
    onNextFrame();
}
void BaseGuiProcessor::EndFrame()
{
    onEndFrame();
}
void BaseGuiProcessor::Present()
{
    onPresent();
}
void BaseGuiProcessor::Update(float deltaTime)
{
    prepare(deltaTime);
    onGUI();
}
void BaseGuiProcessor::onInit()
{
} 
void BaseGuiProcessor::onUpdate(float deltatime)
{
}
void BaseGuiProcessor::onOpen()
{
}
void BaseGuiProcessor::onEndFrame()
{
}
void BaseGuiProcessor::onClose()
{
}
void BaseGuiProcessor::onGUI()
{
}
void BaseGuiProcessor::onPrepare(float deltaTime)
{
}
void BaseGuiProcessor::onRender(float deltaTime)
{
}
void BaseGuiProcessor::onNextFrame()
{
}
void BaseGuiProcessor::onPresent()
{
}
void BaseGuiProcessor::prepare(float deltaTime)
{
    onPrepare(deltaTime);
}
void BaseGuiProcessor::render(float deltaTime)
{
    onRender(deltaTime);
}
};


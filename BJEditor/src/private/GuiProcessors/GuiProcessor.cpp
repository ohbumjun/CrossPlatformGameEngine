#include "GuiProcessors/GuiProcessor.h"
#include "hzpch.h"

namespace HazelEditor
{

void GuiProcessor::Update(float deltatime)
{
    onUpdate(deltatime);

    prepare(deltatime);
    render(deltatime);

    for (size_t i = 0; i < _childs.size(); ++i)
    {
        _childs[i]->Update(deltatime);
    }
}
void GuiProcessor::onInit()
{
}
void GuiProcessor::onRender(float deltaTime)
{
}
void GuiProcessor::onGUI()
{
    for (size_t i = 0; i < _childs.size(); ++i)
    {
        _childs[i]->onGUI();
    }
}
void GuiProcessor::onNextFrame()
{
    for (size_t i = 0; i < _childs.size(); ++i)
    {
        _childs[i]->NextFrame();
    }
}
void GuiProcessor::onPresent()
{
    for (size_t i = 0; i < _childs.size(); ++i)
    {
        _childs[i]->Present();
    }
}
void GuiProcessor::onPrepareGUI()
{
    /*
    ImGuiContext* context = static_cast<ImGuiContext*>(_guiContext);
	ImGuiIO& io = context->IO;

	const ImVec2 mousePosBackup = io.MousePos;
	io.MouseHoveredViewport = 0;
	
	const LvNativeInput& input = _handle->input;
	const ImGuiViewport* viewport = static_cast<ImGuiViewport*>(_viewport);

	if (io.WantSetMousePos)
	{
		lv_window_set_cursor_pos(_handle, (double)(mousePosBackup.x - viewport->Pos.x), (double)(mousePosBackup.y - viewport->Pos.y));
	}

	double mouseX, mouseY;
	lv_window_get_cursor_pos(_handle, &mouseX, &mouseY);

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		int windowX, windowY;
		lv_window_get_pos(_handle, &windowX, &windowY);
		io.MousePos = ImVec2(static_cast<float>(mouseX + windowX), static_cast<float>(mouseY + windowY));
	}
	else
	{
		io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
	}

	if (GetFocusIn())
	{
		for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); ++i)
		{
			io.MouseDown[i] = _handle->input.pointers[i] == LV_PRESS;

			if (_handle->input.pointers[i] == LV_PRESS)
			{
				if (s_root->_handle->input.currentPointers[i] == LV_RELEASE)
				{
					_handle->input.pointers[i] = LV_RELEASE;
				}
			}
		}

		io.MouseWheel += static_cast<float>(input.wheel.y);
		io.KeyCtrl = io.KeysDown[LV_KEY_LEFT_CONTROL];
		io.KeyShift = io.KeysDown[LV_KEY_LEFT_SHIFT];
		io.KeyAlt = io.KeysDown[LV_KEY_LEFT_ALT];
		io.KeySuper = io.KeysDown[LV_KEY_LEFT_SUPER];
	}
    */

    for (size_t i = 0; i < _childs.size(); ++i)
    {
        _childs[i]->onPrepareGUI();
    }
}
void GuiProcessor::onFinishGUI()
{
    for (size_t i = 0; i < _childs.size(); ++i)
    {
        _childs[i]->onFinishGUI();
    }
}
}; // namespace HazelEditor

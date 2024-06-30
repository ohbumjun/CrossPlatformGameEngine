#include "hzpch.h"

// Panel
#include "Panel/ProgressPanel.h"
#include "imgui.h"

namespace HazelEditor
{

// LV_REFLECTION_REGIST
// {
//     LvReflection::RegistBase<ProgressPanel, LvNativePanel>();
// }

void ProgressPanel::Open(const char *title,
                           const char *message,
                           float progress)
{
    _progress = progress;
    MessagePanel::Open(title, message, false);
}

void ProgressPanel::onMessageGUI()
{
    if (_message.empty())
    {
        _isOpen = false;
    }

    ImGui::ProgressBar(_progress, ImVec2(0.0f, 0.0f));

    const float width = ImGui::GetItemRectSize().x;

    char loading;
    loading = "|/-\\"[static_cast<int>(ImGui::GetTime() / 0.05f) & 3];

    const char *ellipsis = "...";
    static float ellipsisWidth = ImGui::CalcTextSize(ellipsis).x;
    std::string message;
    int64 index = _message.length() - 1;
    message.insert(0, 1, loading);
    while (true)
    {
        if (index < 0)
            break;
        message.insert(0, 1, _message[index]);

        if (width - ellipsisWidth <= ImGui::CalcTextSize(message.c_str()).x)
        {
            message = message.substr(1, message.length() - 1);
            message.insert(0,1, *ellipsis);
            break;
        }
        --index;
    }

    ImGui::Text("%s", message.c_str());
}
} 
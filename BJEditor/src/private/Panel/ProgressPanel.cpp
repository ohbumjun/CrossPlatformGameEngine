#include "hzpch.h"

// Panel
#include "Panel/ProgressPanel.h"

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
    LvMessagePanel::Open(title, message, false);
}

void ProgressPanel::onMessageGUI()
{
    if (_message.IsEmpty())
    {
        _isOpen = false;
    }

    ImGui::ProgressBar(_progress, ImVec2(0.0f, 0.0f));

    const float width = ImGui::GetItemRectSize().x;

    char loading;
    loading = "|/-\\"[static_cast<int>(ImGui::GetTime() / 0.05f) & 3];

    const char *ellipsis = "...";
    static float ellipsisWidth = ImGui::CalcTextSize(ellipsis).x;
    LvString message;
    int64 index = _message.Length() - 1;
    message.Insert(0, loading);
    while (true)
    {
        if (index < 0)
            break;
        message.Insert(0, _message[index]);

        if (width - ellipsisWidth <= ImGui::CalcTextSize(message.c_str()).x)
        {
            message = message.SubString(1, message.Length() - 1);
            message.Insert(0, ellipsis);
            break;
        }
        --index;
    }

    ImGui::Text("%s", message.c_str());
}
} 
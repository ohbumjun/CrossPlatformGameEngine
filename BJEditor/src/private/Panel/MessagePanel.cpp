#include "hzpch.h"
#include "Panel/MessagePanel.h"
#include "Hazel/Utils/StringUtil.h"
#include "imgui.h"

namespace HazelEditor
{
void MessagePanel::Open(const char *title,
                        const char *message,
                        std::function<void(int)> onClose)
{
    // LV_CHECK_MAIN_THREAD();
    setUniqueName(title);
    _message = message;
    _onClose = onClose;
}
void MessagePanel::onGUI()
{
    ImGui::OpenPopup(GetName());
    if (ImGui::BeginPopupModal(GetName(),
                               NULL,
                               ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoSavedSettings))
    {
        onMessageGUI();
    }
    ImGui::EndPopup();
}
void MessagePanel::onMessageGUI()
{
    std::string name = GetName();

    std::vector<std::string>& splitName = Utils::StringUtil::SplitString(name, "###");

    float titleSize = ImGui::CalcTextSize(splitName[0].c_str()).x;
    float messageSize = ImGui::CalcTextSize(_message.c_str()).x;

    if (_message.empty() == false)
    {
        ImGui::Text("%s", _message.c_str());
    }

   // if (messageSize < titleSize)
   // {
   //     ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
   //     ImGui::SameLineSeparator(titleSize - messageSize);
   //     ImGui::PopStyleColor();
   // }

    ImGui::Separator();
}
};
} // namespace HazelEditor
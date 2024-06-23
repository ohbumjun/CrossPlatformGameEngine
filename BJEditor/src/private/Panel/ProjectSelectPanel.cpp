#include "hzpch.h"

// Panel
#include "Panel/ProjectSelectPanel.h"

#include <filesystem>
#include <imgui.h>

namespace HazelEditor
{

void ProjectSelectPanel::OnImGuiRender()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Project"))
        {
            bool h = true;
            

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

} // namespace HazelEditor
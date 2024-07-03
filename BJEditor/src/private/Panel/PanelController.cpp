#pragma once

#include "Panel/PanelController.h"
#include "imgui.h"

// Editor 에 있는 코드이지만, Scene 에 접근할 수 있게 하기 위해
// 예외적으로 하나만 HazelEditor 가 아니라
// Hazel Editor Namespace 로 설정한다.
namespace HazelEditor
{
// PanelController::PanelController(EditorWindow *editorWindow)
PanelController::PanelController()
{
    // _editorWindow = editorWindow;
}
PanelController::~PanelController()
{
}
// Panel *PanelController::CreatePanel(const Hazel::TypeId &type,
Panel *PanelController::CreatePanel(const Hazel::TypeId &type)
{
    // 현재는 메인 쓰레드여야만 한다.
    // LV_CHECK_MAIN_THREAD();

    Panel *panel = nullptr;

    const char *name = nullptr;

   const Hazel::TypeId createType = type;
   
   const Hazel::TypeInfo *info = Hazel::Reflection::GetTypeInfo(createType);
   if (info)
   {
       if (info->constructors.eligible)
       {
           panel = static_cast<Panel *>(info->constructors.eligible(
               Hazel::Reflection::CreateTarget(createType)));
       }
       else
       {
           // 해당 패널에 기본 생성자가 없으므로 추가하여야 합니다.
           HZ_CORE_WARN("Not found default constructor.");
       }
   }
   else
   {
       // 리플렉션에 타입이 등록되어야 합니다.
       HZ_CORE_WARN("The type must be registered with reflection.");
   }

  return panel ? addPanel(panel, type) : panel;
}
std::vector<Panel *> PanelController::FindPanels(
    const Hazel::TypeId &type) const
{
    // 메인쓰레드에서만 현재 쓰레드를 실행할 수 있게 해야 한다.
    // LV_CHECK_MAIN_THREAD();

    std::vector<Panel *> result;

    for (auto it = _panels.begin(); it != _panels.end(); ++it)
    {
        Panel *panel = it->second;

        if (panel->_type == type)
        {
            result.push_back(panel);
        }
    }

    return result;
}
Panel *PanelController::FindPanel(const char *name) const
{
    for (auto it = _panels.begin(); it != _panels.end(); ++it)
    {
        Panel *panel = it->second;

        if (strcmp(panel->GetName(), name) == 0)
        {
            return panel;
        }
    }

    return nullptr;
}
Panel *PanelController::FindPanel(const Hazel::TypeId &type) const
{
    for (auto it = _panels.begin(); it != _panels.end(); ++it)
    {
        Panel *panel = it->second;

        if (panel->_type == type)
        {
            return panel;
        }
    }

    return nullptr;
}
void PanelController::Draw()
{
    // 현재는 메인 쓰레드여야만 한다.
    // LV_CHECK_MAIN_THREAD();

    DrawPanels();
}
void PanelController::DrawPanels()
{
    // 메인 스레드
    // LV_PROFILE_EDITOR();
    // LV_CHECK_MAIN_THREAD();

    while (!_panelStack.empty())
    {
        Panel *panel = _panelStack.top();
        _panelStack.pop();
        _panels.insert({panel->GetID(), panel});
    }

    const ImVec2 unitSize = ImGui::CalcTextSize(" ");

    for (auto i = _panels.begin(), end = _panels.end(); i != end; i++)
    {
        Panel *target = (*i).second;

        if (nullptr != target && target->IsOpen())
        {
            // target->unitSize.x = unitSize.x;
            // target->unitSize.y = unitSize.y;

            // if (target->IsUsingMinSize())
            // {
            //     ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize,
            //                         ImVec2(500, 500));
            // }

            ImGui::PushID(static_cast<int>(target->_id));

            target->onGUI();

            ImGui::PopID();

            // ImGuiContext *context = ImGui::GetCurrentContext();

            // if (target->IsUsingMinSize())
            // {
            //     ImGui::PopStyleVar();
            // }
        }

        if (!target->IsOpen() &&
            // (!target->IsDestroying() || !target->IsDestroyed()))
            (!target->IsDestroying()))
        {
            target->Destroy();
        }
    }
}
uint64 PanelController::makeUniquePanelId(Hazel::TypeId type)
{
    //  if (!_panelCountTable.ContainsKey(type))
    if (_panelCountTable.find(type) == _panelCountTable.end())
    {
        _panelCountTable.insert(std::make_pair(type, 0));
    }

    return (static_cast<uint64>(type) << 32) + (++_panelCountTable[type]);
}
Panel *PanelController::addPanel(Panel *panel,
                                 const uint32 &type)
{
    // switch (panel->GetPanelType())
    // {
    // default:
    // case PanelType::NATIVE_PANEL:
    // {
    //     LvNativePanel *native = static_cast<LvNativePanel *>(panel);
    //     native->_window = _editorWindow;
    // }
    // break;
    // }
    // 
    // if (0 != panel->GetMaxCount() &&
    //     panel->GetMaxCount() <= FindPanels(type).Count())
    // {
    //     panel->Destroy();
    // 
    //     LV_LOG(warning,
    //            "To create multiple Panels, you must override "
    //            "\"GetMaxCount\". (default : 1)");
    //     return nullptr;
    // }

    panel->_id = makeUniquePanelId(type);
    panel->_type = type;
    panel->Initialize();

    _panelStack.push(panel);
    return panel;
}
} // namespace HazelEditor

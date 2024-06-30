#pragma once

#include "Panel/NativePanel.h"
#include "Panel/MenuNode.h"
#include "Hazel/Core/Thread/ThreadVars.h"

/* #include "engine/thirdparty/imgui/imgui.h" */ typedef int ImGuiWindowFlags;
/* #include "engine/thirdparty/imgui/imgui.h" */ typedef int ImGuiDockNodeFlags;


// Editor 에 있는 코드이지만, Scene 에 접근할 수 있게 하기 위해
// 예외적으로 하나만 HazelEditor 가 아니라
// Hazel Editor Namespace 로 설정한다.
namespace HazelEditor
{
class PanelController;

class DockSpacePanel : public NativePanel
{
public:
    DockSpacePanel(){};
    PanelController *GetPanelController() const
    {
        return _panelController;
    }
	
    inline Hazel::TypeId GetType() const override
    {
        return Hazel::Reflection::GetTypeID<DockSpacePanel>();
    }

    void RefreshMenu()
    {

    }

protected:
    void onInitialize() override;
    void onGUI() override;
    void onDestroy() override;

    void drawControll();
    void drawControllLeft();
    void drawControllCenter();
    void drawControllRight();
    void drawDockingspace();
    void drawStatusBar();

    bool m_RefreshMenu;
    ImGuiDockNodeFlags m_DockNodeFlags = 0;
    ImGuiWindowFlags m_WindowFlags = 0;
    MenuNode *m_RootMenu = nullptr;
    PanelController *m_PanelController = nullptr;
    std::string m_StatustBarText;
    Hazel::Atomic *m_RunningProgressCount = nullptr;

private:
};
} // namespace Hazel

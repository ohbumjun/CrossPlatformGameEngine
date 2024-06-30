#pragma once

#include "Panel/NativePanel.h"

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

    ImGuiDockNodeFlags _dockNodeFlags = 0;
    ImGuiWindowFlags _windowFlags = 0;
    MenuNode *_rootMenu = nullptr;
    PanelController *_panelController = nullptr;
    std::string _statustBarText;
    Hazel::Atomic *_runningProgressCount = nullptr;

private:
};
} // namespace Hazel

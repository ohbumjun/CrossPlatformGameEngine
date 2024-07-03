#pragma once

#include "Panel/Panel.h"
#include "Hazel/Core/Reflection/TypeId.h"
#include "Hazel/Core/Reflection/Reflection.h"
#include <vector>
#include <unordered_map>

// Editor 에 있는 코드이지만, Scene 에 접근할 수 있게 하기 위해
// 예외적으로 하나만 HazelEditor 가 아니라
// Hazel Editor Namespace 로 설정한다.
namespace HazelEditor
{
// class EditorWindow;
class PanelController
{
public:
    // PanelController(EditorWindow *editorWindow);
    PanelController();
    ~PanelController();
    
	template <typename T>
    T *CreatePanel()
    {
        return static_cast<T *>(CreatePanel(Hazel::Reflection::GetTypeID<T>()));
    }

    template <typename T>
    T *FindPanel()
    {
        return static_cast<T *>(FindPanel(Hazel::Reflection::GetTypeID<T>()));
    }
    // nPanel *CreatePanel(const Hazel::TypeId &type, void *monoScriptPanel = nullptr);
    Panel *CreatePanel(const Hazel::TypeId &type);

    std::vector<Panel *> FindPanels(const Hazel::TypeId &type) const;

    Panel *FindPanel(const char *name) const;
    Panel *FindPanel(const Hazel::TypeId &type) const;

    void Draw();

    void DrawPanels();

    inline size_t GetPanelCount() const
    {
        return _panels.size();
    }
private:
    uint64 makeUniquePanelId(Hazel::TypeId type);
    Panel *addPanel(Panel *panel, const uint32 &monoType);
	
    // EditorWindow *_editorWindow;
    std::unordered_map<Hazel::TypeId, uint32> _panelCountTable;
    std::unordered_map<uint64, Panel *> _panels;
    std::stack<Panel *> _panelStack;
};
} // namespace Hazel

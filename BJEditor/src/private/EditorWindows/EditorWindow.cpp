#include "hzpch.h"
#include "EditorWindows/EditorWindow.h"
#include "Panel/PanelController.h"
#include "Hazel/Core/Thread/ThreadUtil.h"
#include "Hazel/Core/Thread/ThreadExecuter.h"

namespace HazelEditor
{
EditorWindow *EditorWindow::GetEditorWindow()
{
    return nullptr;
}

Panel* EditorWindow::CreatePanel(const Hazel::TypeId& type)
{
    bool created = false;
    Panel *result = nullptr;
    const bool useMainThead = Hazel::ThreadUtils::IsCurrentMainThread();
    if (useMainThead)
    {
        result = _panelController->CreatePanel(type);
        created = true;
    }
    else
    {
        Hazel::ThreadExecuter::Main().Sync([&]() {
            result = _panelController->CreatePanel(type);
            created = true;
        });
    }

    // @donghun 이쪽에서 걸리는 케이스가 발견되면 DispatchQueue의 Sync가 제대로 동작하지 않은것.
    HZ_CORE_ASSERT(created, "Failed to create panel");
    return result;
}
std::vector<Panel*> EditorWindow::FindPanels(const Hazel::TypeId& type) const
{

}

Panel* EditorWindow::FindPanel(const char* name) const
{

}

void EditorWindow::OpenMessagePopup(const std::string& title, const std::string& message)
{

}
void EditorWindow::DisplayProgressBar(float progress,
                                    const char *title,
                                    const char *message,
                                    const std::string &key = "")
{

}

void EditorWindow::ClearProgressBar(const std::string &key = "")
{ 

}

void EditorWindow::onInit()
{
}

void EditorWindow::onUpdate(float deltatime)
{
}

void EditorWindow::onOpen()
{
    }

void EditorWindow::onClose()
{
    }

void EditorWindow::onPrepareGUI()
{
    }

void EditorWindow::onFinishGUI()
{
    }

void EditorWindow::onGUI()
{
    }

void EditorWindow::onNextFrame()
{
    }

void EditorWindow::updateProgressBar(const ProgressData &data)
{
    }

};


#include "hzpch.h"
#include "EditorWindows/EditorWindow.h"
#include "Panel/PanelController.h"
#include "Panel/DockSpacePanel.h"
#include "imgui.h"
#include "Hazel/Core/Thread/ThreadUtil.h"
#include "Hazel/Core/Thread/ThreadExecuter.h"
#include "ProjectContext.h"
#include "Project.h"

namespace HazelEditor
{
EditorWindow *s_EditorWindow = nullptr;

EditorWindow *BJ_GetEditorWindow()
{
    return nullptr;
}

EditorWindow::EditorWindow()
{
    s_EditorWindow = this;
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
        Hazel::ThreadExecuterManager::GetMain().SyncExecute([&]() {
            result = _panelController->CreatePanel(type);
            created = true;
        });
    }

    HZ_CORE_ASSERT(created, "Failed to create panel");

    return result;
}
std::vector<Panel*> EditorWindow::FindPanels(const Hazel::TypeId& type) const
{
    return _panelController->FindPanels(type);
}

Panel* EditorWindow::FindPanel(const char* name) const
{
    return _panelController->FindPanel(name);
}

void EditorWindow::OpenMessagePopup(const std::string& title, const std::string& message)
{
    MessagePanel *panel = CreatePanel<MessagePanel>();
    panel->Open(title.c_str(), message.c_str(), [](int empty){});
}
void EditorWindow::DisplayProgressBar(float progress,
                                    const char *title,
                                    const char *message,
                                    const std::string &key = "")
{
    ProgressData data(progress, title, message);

    if (data == _lastProgressData)
        return;

    // if (!key.empty() && !_progressRemainKeys.Contains(key))
    // {
    //     _progressRemainKeys.Add(key);
    // }

    if (Hazel::ThreadUtils::IsCurrentMainThread())
    {
        updateProgressBar(data);
    }
    else
    {
        _progressQueue.Enqueue(data);
    }

    _lastProgressData = data;
}

void EditorWindow::ClearProgressBar(const std::string &key)
{ 
    if (Hazel::ThreadUtils::IsCurrentMainThread())
    {
        updateProgressBar(ProgressData::Close());
        _progressQueue.Clear();
    }
    else
    {
        _progressQueue.Enqueue(ProgressData::Close());
    }

    _progressQueue.Enqueue(ProgressData::Close());
}

void EditorWindow::onInit()
{
    GuiWindow::onInit();
}

void EditorWindow::onUpdate(float deltatime)
{
    // LV_CHECK(_engine->renderer != nullptr, "Engine renderer is nullptr");

    ProjectContext *context = BJ_GetProjectContext();

    float delta = deltatime;
    if (nullptr == context || nullptr == context->project ||
        context->project->GetPlayState() == Project::PlayState::STOP)
    {
        delta = 0;
    }

    onPrepareGUI();

    onGUI();

    onFinishGUI();

    // if (_engine->context->shouldClose)
    // {
    //     context->project->Stop();
    //     _engine->context->shouldClose = false;
    // }
}

void EditorWindow::onOpen()
{
    // LV_CHECK_MAIN_THREAD();

    _mutex = Hazel::ThreadUtils::CreateCritSect();

    // _panelController = _editorWindowPanelController = new PanelController(this);
    _panelController =  new PanelController(this);

    bool useDockSpace = true;

    if (useDockSpace)
    {
        DockSpacePanel *dockSpace = CreatePanel<DockSpacePanel>();
        dockSpace->Open();
        _panelController = dockSpace->GetPanelController();
    }
}

void EditorWindow::onClose()
{
    if (Project::IsLoaded())
    {
        Project::Close();
    }

    delete _panelController;

    Hazel::ThreadUtils::DestroyCritSect(_mutex);
}

void EditorWindow::onPrepareGUI()
{
    // LV_PROFILE_EDITOR();
    // ImGuiContext *context = static_cast<ImGuiContext *>(_guiContext);
    // ImGuiIO &io = context->IO;
    // 
    // io.DisplaySize.x = static_cast<float>(GetWidth());
    // io.DisplaySize.y = static_cast<float>(GetHeight());
    // 
    // int fx, fy;
    // lv_window_get_framebuffer_size(_handle, &fx, &fy);
    // 
    // io.DisplayFramebufferScale.x =
    //     static_cast<float>(fx) / static_cast<float>(GetWidth());
    // io.DisplayFramebufferScale.y =
    //     static_cast<float>(fy) / static_cast<float>(GetHeight());
    // 
    // io.MousePos = ImVec2(-LV_FLT_MAX, -LV_FLT_MAX);
    // memset(io.MouseDown, 0, sizeof(io.MouseDown));
    // 
    // const uint32 time = lv_time_milli();
    // const float currentTime = static_cast<float>(time) / 1000.f;
    // static float contextTime = 0.f;
    // 
    // io.DeltaTime =
    //     contextTime > 0.0f ? (currentTime - contextTime) : (1.0f / 60.0f);
    // if (io.DeltaTime <= 0.0f)
    // {
    //     io.DeltaTime = LV_EPS;
    // }

    // contextTime = currentTime;

    GuiWindow::onPrepareGUI();

    ImGui::NewFrame();
    }

void EditorWindow::onFinishGUI()
    {
    // LV_PROFILE_EDITOR();
    ImGui::Render();
    };

void EditorWindow::onGUI()
    {
    GuiWindow::onGUI();

    while (_progressQueue.Count())
    {
        ProgressData data;
        if (_progressQueue.Dequeue(data))
        {
            updateProgressBar(data);
        }
    }

    if (_presentable)
    {
        _panelController->Draw();
    }
    };

void EditorWindow::onNextFrame()
{
    //Base::onNextFrame();
    if (_presentable)
    {
       // _submitIndex = _renderContext->GetSubmitIndex(_surface->swapchain);
       // _context.renderer->NextFrame(
       //     _surface->swapchain); //editor에서 추가한 renderPath 모두 설정
       // _engine->renderer->NextFrame(_surface->swapchain);
       // LvSceneGraphInternal::NextFrame();
    }

    for (size_t i = 0; i < _childs.size(); ++i)
    {
        _childs[i]->NextFrame();
    }
}

void EditorWindow::updateProgressBar(const ProgressData &data)
{
    if (data.show)
    {
        if (nullptr == _progress)
        {
            _progress = CreatePanel<ProgressPanel>();
        }
        // LV_CHECK(0 != _progress->GetType(), "Wrong memory");

        _progress->Open(data.title.c_str(),
                        data.message.c_str(),
                        data.progress);
    }
    else
    {
        // if (nullptr != _progress)
        // {
        //     _progress->DestroyImmediate();
        // }
    }
}

};


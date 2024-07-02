#include "hzpch.h"
#include "Panel/DockSpacePanel.h"
#include "Panel/PanelController.h"
#include "ProjectContext.h"
#include "EditorContext.h"
#include "Project.h"
#include "EditorWindows/EditorWindow.h"
#include "Hazel/Core/Allocation/Allocator/EngineAllocator.h"
#include "Hazel/FileSystem/DirectorySystem.h"
#include "imgui.h"
#include "IconsFontAwesome6.h"

namespace HazelEditor
{
void DockSpacePanel::onInitialize()
{
    m_DockNodeFlags =
        ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode;

    m_WindowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                   ImGuiWindowFlags_NoBringToFrontOnFocus |
                   ImGuiWindowFlags_NoNavFocus;

    void *panelControllerPtr =
        Hazel::EngineAllocator::BJ_EngineAllocate(sizeof(PanelController));
    m_PanelController = new (panelControllerPtr) PanelController(EditorWindow::BJ_GetEditorWindow());
    m_StatustBarText = "";
}
void DockSpacePanel::onGUI()
{
    drawControll();

    drawDockingspace();

    m_PanelController->Draw();

    drawStatusBar();
}
void DockSpacePanel::onDestroy()
{
    MenuNode::Destroy(m_RootMenu);
    m_PanelController->~PanelController();
    Hazel::EngineAllocator::BJ_EngineFree(m_PanelController);
}
void DockSpacePanel::drawControll()
{
    const ImGuiStyle &style = ImGui::GetStyle();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();

    const ImVec2 mainViewportWorkPos = viewport->WorkPos;
    const ImVec2 mainViewportWorkSize = viewport->WorkSize;

    const float controllBarMinSizeY =
        (ImGui::GetFontSize() + style.FramePadding.y * 2.f) * 2.f +
        style.WindowPadding.y * 2.f;
    const ImVec2 controllBarSize(mainViewportWorkSize.x, controllBarMinSizeY);

    ImGui::SetNextWindowPos(mainViewportWorkPos);
    ImGui::SetNextWindowSize(controllBarSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    // 에디터의 상단 컨트롤 버튼 영역
    if (ImGui::Begin("Controll Bar Area",
                     &_isOpen,
                     m_WindowFlags | ImGuiWindowFlags_MenuBar))
    {
        if (m_RefreshMenu)
        {
            if (nullptr != m_RootMenu)
            {
                MenuNode::Destroy(m_RootMenu);
            }
            m_RootMenu = MenuNode::Create();
            m_RefreshMenu = false;
        }

        if (ImGui::BeginMenuBar())
        {
            m_RootMenu->Draw(true);

            ImGui::EndMenuBar();
        }

        ImGui::PopStyleVar(2);

        ImGui::Columns(3, nullptr, false);

        drawControllLeft();

        ImGui::NextColumn();

        drawControllCenter();

        ImGui::NextColumn();

        drawControllRight();
    }
    ImGui::End();
}
void DockSpacePanel::drawControllLeft()
{
}
void DockSpacePanel::drawControllCenter()
{
    // 버튼 중앙정렬
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() / 2 -
                         (ImGui::GetFontSize() * 2.2f) -
                         2 * ImGui::GetStyle().ItemSpacing.x);

    const ProjectContext *context = BJ_GetProjectContext();

    static int playMode =
        nullptr != context && nullptr != context->project
            ? static_cast<int>(context->project->GetPlayMode())
            : 0;
    const char *listboxItems[] = {"Editor", "Bundle"};
    ImGuiIO &io = ImGui::GetIO();

    const bool isReadyProject =
        nullptr != context && nullptr != context->project;

    ImVec2 size = ImGui::CalcTextSize("  " ICON_FA_PLAY ICON_FA_ANGLE_DOWN);
    size.y += ImGui::GetStyle().FramePadding.y;

    ImGui::BeginDisabled(!isReadyProject);
    {
        ImGui::PushStyleColor(ImGuiCol_Header,
                              ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                              ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
        {
            const bool isPlaying =
                nullptr != context && context->project != nullptr &&
                Project::PlayState::STOP != context->project->GetPlayState();

            // 플레이 버튼
            if (ImGui::Selectable("  " ICON_FA_PLAY ICON_FA_ANGLE_DOWN,
                                  isPlaying,
                                  ImGuiSelectableFlags_None,
                                  size))
            {
                if (isPlaying)
                {
                    context->project->Stop();
                }
                else
                {
                    if (static_cast<Project::PlayMode>(playMode) ==
                        Project::PlayMode::EDITOR)
                    {
                        // LvSceneGraphInternal::SetInitName(
                        //     LvEditorSceneManager::GetActiveAsset()
                        //         ->uuid.GetGuid()
                        //         .ToString());
                        // if (!LvEditorSceneGraph::GetNameTable().ContainsKey(
                        //         LvEditorSceneManager::GetActiveAsset()->name))
                        // {
                        //     LV_LOG(warning,
                        //            "Scene Graph에 현재 Scene을 등록해주세요.");
                        // }
                        if (false)
                        {
                        }
                        else
                        {
                            context->project->Play();
                        }
                    }
                    else
                    {
                        context->project->Play();
                    }
                }
            }
            const ImVec2 min = ImGui::GetItemRectMin();

            // if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
            //     !loadLibrary)
            // {
            //     ImGui::SetTooltip("Project not loaded");
            // }

            if ((ImGui::IsItemHovered() && io.MouseDownDuration[0] > 1.f) ||
                (ImGui::IsItemHovered() &&
                 ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
            {
                ImGui::OpenPopup("PlayMode");
            }

            if (ImGui::BeginPopup("PlayMode"))
            {
                for (int i = 0; i < IM_ARRAYSIZE(listboxItems); ++i)
                {
                    if (ImGui::Selectable(listboxItems[i], playMode == i))
                    {
                        playMode = i;
                        context->project->SetPlayMode(
                            static_cast<Project::PlayMode>(playMode));
                    }
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine();

            const bool isPause =
                nullptr != context && context->project != nullptr &&
                context->project->GetPlayState() == Project::PlayState::PAUSE;

            size = ImGui::CalcTextSize("  " ICON_FA_PAUSE "  ");
            size.y += ImGui::GetStyle().FramePadding.y;

            // 일시정지 버튼
            if (ImGui::Selectable("  " ICON_FA_PAUSE "  ",
                                  isPause,
                                  ImGuiSelectableFlags_None,
                                  size))
            {
                if (nullptr != context)
                {
                    if (isPause)
                    {
                        context->project->Play();
                    }
                    else
                    {
                        if (!isPlaying)
                        {
                            context->project->Play();
                        }
                        context->project->Pause();
                    }
                }
            }
            const float x0 = ImGui::GetItemRectMin().x;

            ImGui::SameLine();

            ImGui::BeginDisabled(!isPlaying);
            // 다음 프레임 버튼
            if (ImGui::Selectable("  " ICON_FA_FORWARD_STEP "  ",
                                  false,
                                  ImGuiSelectableFlags_None,
                                  size))
            {
                if (nullptr != context)
                {
                    context->project->NextFrame();
                }
            }
            ImGui::EndDisabled();

            const float x1 = ImGui::GetItemRectMin().x;
            const ImVec2 max = ImGui::GetItemRectMax();
            const float x2 = ImGui::GetItemRectMin().x;

            // 버튼 테두리
            const ImColor color(ImGui::GetStyleColorVec4(ImGuiCol_Separator));
            ImGui::GetWindowDrawList()->AddRect(min, max, color, 1);
            ImGui::GetWindowDrawList()->AddLine({x0, min.y},
                                                {x0, max.y},
                                                color);
            ImGui::GetWindowDrawList()->AddLine({x1, min.y},
                                                {x1, max.y},
                                                color);
        }
        ImGui::PopStyleColor(2);
    }

    ImGui::EndDisabled();
}
void DockSpacePanel::drawControllRight()
{
}
void DockSpacePanel::drawDockingspace()
{
    const ImGuiStyle &style = ImGui::GetStyle();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();

    const ImVec2 mainViewportWorkPos = viewport->WorkPos;
    const ImVec2 mainViewportWorkSize = viewport->WorkSize;

    const float statusMinSizeY = ImGui::GetFontSize() * 1.6f;
    const float controllBarMinSizeY =
        (ImGui::GetFontSize() + style.FramePadding.y * 2.f) * 2.f +
        style.WindowPadding.y * 2.f;
    const ImVec2 originalWindowPadding = ImGui::GetStyle().WindowPadding;

    // controll, status쪽의 패딩
    const ImVec2 dockSize(mainViewportWorkSize.x,
                          mainViewportWorkSize.y -
                              (controllBarMinSizeY + statusMinSizeY + 2.f));
    const ImVec2 dockPos(mainViewportWorkPos.x,
                         mainViewportWorkPos.y + (controllBarMinSizeY + 1.f));
    ImGui::SetNextWindowPos(dockPos);
    ImGui::SetNextWindowSize(dockSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    const bool collapsed =
        !ImGui::Begin("Appliation Docking Space",
                      &_isOpen,
                      m_WindowFlags | ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar(3);
    if (!collapsed)
    {
        ImGuiIO &io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            if (nullptr == BJ_GetProjectContext())
            {
                ImVec2 calculatedPadding = ImGui::GetStyle().WindowPadding;
                calculatedPadding.x *= 3.f;
                calculatedPadding.y *= 3.f;

                // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImGui::GetStyle().WindowPadding * 3.f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                                    calculatedPadding);

                ImGui::PushStyleColor(
                    ImGuiCol_ChildBg,
                    ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

                const ImVec2 size(ImGui::GetFontSize() * 27.f,
                                  ImGui::GetFontSize() * 18.f);
                const ImVec2 startWindowPos(
                    dockPos.x + dockSize.x / 2.f - size.x / 2.f,
                    dockPos.y + dockSize.y / 2.f - size.y / 2.f);

                ImGui::SetNextWindowPos(startWindowPos);
                if (ImGui::BeginChild("start window", size, true, m_WindowFlags))
                {
                    constexpr static float scale = 2.f;
                    if (ImGui::BeginChild(
                            "title",
                            ImVec2(0, ImGui::GetFontSize() * scale)))
                    {
                        ImGui::SetWindowFontScale(
                            ImGui::GetIO().FontGlobalScale * scale);

                        ImGui::Text("Lv1Engine");
                    }
                    ImGui::EndChild();

                    if (ImGui::BeginChild("body",
                                          ImVec2(0, 0),
                                          false,
                                          ImGuiWindowFlags_NoScrollbar))
                    {
                        // const LvEngineContext *engineContext =
                        //     lv_engine_get_context();
                        // const auto &version = engineContext->version;
                        // LvString versionString =
                        //     LvString::Format("%u.%u.%u",
                        //                      version.major,
                        //                      version.minor,
                        //                      version.patch);
                        // 
                        // if (!lv_editor_get_context()->revision.IsEmpty())
                        // {
                        //     LvString revision =
                        //         lv_editor_get_context()->revision;
                        //     revision.Resize(
                        //         6); // @donghun 너무 길기 때문에 6자까지만 사용
                        // 
                        //     versionString.AppendFormat(
                        //         ".%u [%s / %s]",
                        //         lv_editor_get_context()->buildNumber,
                        //         lv_editor_get_context()->branch.c_str(),
                        //         revision.c_str());
                        // }

                        // ImGui::PushStyleColor(
                        //     ImGuiCol_Text,
                        //     ImGui::GetStyleColorVec4(ImGuiCol_Separator));
                        // ImGui::Text("Version %s", versionString.c_str());
                        // ImGui::PopStyleColor();
                        
                        // ImGui::PopStyleVar();
                        
                        // if (ImGui::BeginPopupContextItem(
                        //         "Version Context Window"))
                        // {
                        //     if (ImGui::MenuItem("Copy"))
                        //     {
                        //         lv_window_clipboard_set(
                        //             lv_editor_get_window()->GetNativeWindow(),
                        //             versionString.c_str());
                        //     }
                        // 
                        //     ImGui::EndPopup();
                        // }

                        // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImGui::GetStyle().WindowPadding * 3.f);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, calculatedPadding);

                        ImGui::Spacing();

                        const ImVec4 col =
                            ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);

                        if (ImGui::BeginTable("list", 2))
                        {
                            // Text처럼 노출되는 버튼, 텍스트의 색상은 기본 버튼의 눌렸을때의 색상을 사용함
                            auto drawButton = [](const char *label) {
                                const ImVec4 col = ImGui::GetStyleColorVec4(
                                    ImGuiCol_ButtonActive);
                                ImGui::PushStyleColor(ImGuiCol_Text, col);
                                ImGui::PushStyleColor(ImGuiCol_Button,
                                                      IM_COL32_BLACK_TRANS);
                                ImGui::PushStyleColor(
                                    ImGuiCol_ButtonHovered,
                                    ImGui::GetStyleColorVec4(ImGuiCol_Tab));
                                ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                                                      ImGui::GetStyleColorVec4(
                                                          ImGuiCol_TabActive));
                                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                                                    ImVec2(0, 0));
                                const bool result = ImGui::Button(label);
                                ImGui::PopStyleVar();
                                ImGui::PopStyleColor(4);
                                return result;
                            };

                            ImGui::TableNextColumn();
                            // 왼쪽 액션버튼들 출력
                            if (ImGui::BeginChild("actions"))
                            {
                                if (drawButton("  " ICON_FA_FOLDER_PLUS
                                               "  New Project"))
                                {
                                    // lv_editor_new_project_aync();
                                }
                                if (drawButton("  " ICON_FA_FOLDER_OPEN
                                               "  Open Project"))
                                {
                                    // lv_editor_open_project_aync();
                                }

                                // const bool useSafePath =
                                //     ImGui::IsKeyDown(ImGuiKey_LeftMeta);
                                // const char *safePath =
                                //     LvCommandInternal::GetSafePath();
                                // if (drawButton("  " ICON_FA_ROBOT
                                //                "  Auto New Project"))
                                // {
                                //     if (useSafePath)
                                //     {
                                //         LvDispatchQueue("Create Project")
                                //             .Async([]() {
                                //                 LvCommand::Commit<
                                //                     LvProjectCreate>(
                                //                     LvCommandInternal::
                                //                         GetSafePath(),
                                //                     true);
                                //             });
                                //     }
                                //     else
                                //     {
                                //         lv_editor_new_project_last_open_path_aync();
                                //     }
                                // }

                                // if (useSafePath && ImGui::IsItemHovered())
                                // {
                                //     ImGui::PushStyleVar(
                                //         ImGuiStyleVar_WindowPadding,
                                //         originalWindowPadding);
                                //     ImGui::BeginTooltip();
                                //     ImGui::Text("Safe Path Mode");
                                //     ImGui::EndTooltip();
                                //     ImGui::PopStyleVar();
                                // }

                                // ImGui::SameLine();
                                // LvString path =
                                //     lv_editor_get_context()
                                //         ->settings.autoCreateProject;
                                // constexpr size_t length = 13;
                                // if (length < path.Length())
                                // {
                                //     path =
                                //         path.SubString(path.Length() - length,
                                //                        length);
                                //     path.Insert(0, "...");
                                // }
                                // ImGui::Text(path.c_str());
                                // if (ImGui::IsItemHovered())
                                // {
                                //     ImGui::PushStyleVar(
                                //         ImGuiStyleVar_WindowPadding,
                                //         originalWindowPadding);
                                //     ImGui::BeginTooltip();
                                //     ImGui::Text(
                                //         "%s",
                                //         useSafePath
                                //             ? safePath
                                //             : lv_editor_get_context()
                                //                   ->settings.autoCreateProject
                                //                   .c_str());
                                //     ImGui::EndTooltip();
                                //     ImGui::PopStyleVar();
                                // }
                                // 
                                // if (ImGui::IsItemClicked())
                                // {
                                //     LvString autoCreateProject =
                                //         lv_editor_get_context()
                                //             ->settings.autoCreateProject;
                                // 
                                //     if (!lv_directory_exist(
                                //             autoCreateProject.c_str()))
                                //     {
                                //         autoCreateProject =
                                //             lv_system_get_context()
                                //                 ->executableDirectoryPath;
                                //     }
                                // 
                                //     static char autoCreateProjectPath
                                //         [LV_CHAR_INIT_LENGTH] = {0};
                                // 
                                //     if (lv_window_folder_browser_open(
                                //             lv_system_get_context()->headWindow,
                                //             autoCreateProject.c_str(),
                                //             autoCreateProjectPath))
                                //     {
                                //         if (lv_directory_exist(
                                //                 autoCreateProjectPath))
                                //         {
                                //             lv_editor_get_context()
                                //                 ->settings.autoCreateProject =
                                //                 autoCreateProjectPath;
                                //             lv_editor_get_context()
                                //                 ->SaveSettings();
                                //             lv_editor_get_window()
                                //                 ->CreatePanel<LvMessagePanel>()
                                //                 ->Open(
                                //                     "Setting Succed",
                                //                     "Changed to the directory "
                                //                     "you specified.");
                                //         }
                                //         else
                                //         {
                                //             lv_editor_get_window()
                                //                 ->CreatePanel<LvMessagePanel>()
                                //                 ->Open("Setting Failed",
                                //                        "Invalid path");
                                //         }
                                //     }
                                // }

                                ImGui::Spacing();
                                ImGui::Text("Recent");
                                ImGui::Indent();

                                const std::vector<std::string> &paths =
                                    BJ_GetEditorContext()
                                        ->GetSettings()
                                        .GetLastOpenProjects();
                                for (size_t i = 0, max = paths.size(); i < max;
                                     ++i)
                                {
                                    // std::string parent = lv_path_name lv_path_parent(paths[i].c_str()).c_str());
                                    std::string parent = Hazel::DirectorySystem::GetPathame(Hazel::DirectorySystem::GetPathParent(paths[i].c_str()).c_str());

                                    // if (parent == nullptr)
                                    if (parent.empty())
                                    {
                                        parent = "";
                                    }

                                    // std::string name = lv_path_name(paths[i].c_str());
                                    std::string name = Hazel::DirectorySystem::GetPathame(paths[i].c_str());

                                    ImGui::PushID(i);
                                    if (drawButton(name.c_str()))
                                    {
                                        Project::Open(paths[i].c_str());
                                    }
                                    ImGui::PopStyleColor();
                                    ImGui::PopStyleVar();
                                    // if (ImGui::BeginPopupContextItem("context"))
                                    // {
                                    //     if (ImGui::Selectable(
                                    //             "Refresh CMakeLists"))
                                    //     {
                                    //         LvProjectGenerator::
                                    //             CmakeListsUpdate(
                                    //                 paths[i].c_str());
                                    //     }
                                    //     ImGui::EndPopup();
                                    // }
                                    ImGui::PushStyleVar(
                                        ImGuiStyleVar_WindowPadding,
                                        // ImGui::GetStyle().WindowPadding * 3.f);
                                        calculatedPadding);

                                    ImGui::PushStyleColor(
                                        ImGuiCol_ChildBg,
                                        ImGui::GetStyleColorVec4(
                                            ImGuiCol_WindowBg));
                                    ImGui::PopID();

                                    ImGui::SameLine();
                                    ImGui::Text(parent.c_str());
                                    if (ImGui::IsItemHovered())
                                    {
                                        ImGui::PushStyleVar(
                                            ImGuiStyleVar_WindowPadding,
                                            originalWindowPadding);
                                        ImGui::BeginTooltip();
                                        ImGui::Text("%s", paths[i].c_str());
                                        ImGui::EndTooltip();
                                        ImGui::PopStyleVar();
                                    }
                                }
                                ImGui::Unindent();
                            }
                            ImGui::EndChild();

                            ImGui::TableNextColumn();

                            // 오른쪽 외부 링크들 출력
                            // if (ImGui::BeginChild("helper"))
                            // {
                            //     ImGui::Indent();
                            //     ImGui::PushStyleColor(ImGuiCol_Text, col);
                            //     if (drawButton("What is Lv1Engine?"))
                            //     {
                            //         LvWeb::OpenUrl(
                            //             "http://10.14.0.61:8888/share/"
                            //             "37b25eb4-cd00-49ae-8cb4-a864b074ca2e");
                            //     }
                            //     if (drawButton("How to build?"))
                            //     {
                            //         LvWeb::OpenUrl(
                            //             "http://10.14.0.61:8888/share/"
                            //             "f97d465d-6b59-440b-b209-5f2136a56f05");
                            //     }
                            //     if (drawButton("Go to Repository"))
                            //     {
                            //         LvWeb::OpenUrl(
                            //             "http://devgit.com2us.com/TS/TPact/");
                            //     }
                            //     if (drawButton("Documentation"))
                            //     {
                            //         LvWeb::OpenUrl(
                            //             "http://10.14.0.61:8888/home");
                            //     }
                            //     if (drawButton("API"))
                            //     {
                            //         LvWeb::OpenUrl("http://10.14.0.165:8000/");
                            //     }
                            //     ImGui::PopStyleColor();
                            //     ImGui::Unindent();
                            // }
                            // ImGui::EndChild();

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
            }
            else
            {
                ImGui::DockSpace(ImGui::GetID("Dockspace"),
                                 ImVec2(0.0f, 0.0f),
                                 m_DockNodeFlags);
            }
        }
        else
        {
            HZ_CORE_WARN("ImGuiConfigFlags_DockingEnable disabled.");
            // LV_LOG(debug, "ImGuiConfigFlags_DockingEnable disabled.");
        }
    }
    ImGui::End();
}
void DockSpacePanel::drawStatusBar()
{
    const ImGuiStyle &style = ImGui::GetStyle();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();

    const ImVec2 mainViewportWorkPos = viewport->WorkPos;
    const ImVec2 mainViewportWorkSize = viewport->WorkSize;

    const float statusMinSizeY = ImGui::GetFontSize() * 1.6f + 1;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, {statusMinSizeY, 0});

    const ImVec2 indicatorSize(unitSize.x * 10, statusMinSizeY);
    const ImVec2 debuggerSize(unitSize.x * 10, statusMinSizeY);
    const ImVec2 statusBarSize(mainViewportWorkSize.x - indicatorSize.x -
                                   debuggerSize.x - 2,
                               statusMinSizeY);

    const float posY =
        mainViewportWorkPos.y + mainViewportWorkSize.y - statusBarSize.y + 1;
    const ImVec2 debuggerPos(mainViewportWorkPos.x + statusBarSize.x + 1, posY);
    const ImVec2 indicatorPos(debuggerPos.x + debuggerSize.x + 1, posY);
    const ImVec2 statusBarPos(mainViewportWorkPos.x, posY);

    constexpr auto statusBarWindowFlag =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::SetNextWindowPos(statusBarPos);
    ImGui::SetNextWindowSize(statusBarSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("Status Bar", &_isOpen, statusBarWindowFlag))
    {
        if (ImGui::IsWindowHovered() &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            // FocusingOrOpen<LvConsolePanel>();
        }

        // while (!_datas.IsEmpty())
        // {
        //     LvCallStackLog::Data data;
        //     if (_datas.Dequeue(data))
        //     {
        //         data.data.Replace("\n", " ");
        // 
        //         switch (data.logType)
        //         {
        //         default:
        //         case LvLog::Type::LOG_DEBUG:
        //             m_StatustBarText.FormatSelf("  " ICON_FA_COMMENT "  %s",
        //                                        data.data.c_str());
        //             break;
        //         case LvLog::Type::LOG_WARNING:
        //             m_StatustBarText.FormatSelf("  " ICON_FA_CIRCLE_EXCLAMATION
        //                                        "  %s",
        //                                        data.data.c_str());
        //             break;
        //         case LvLog::Type::LOG_ERROR:
        //             m_StatustBarText.FormatSelf("  " ICON_FA_TRIANGLE_EXCLAMATION
        //                                        "  %s",
        //                                        data.data.c_str());
        //             break;
        //         case LvLog::Type::LOG_CRASH:
        //             m_StatustBarText.FormatSelf("  " ICON_FA_BUG "  %s",
        //                                        data.data.c_str());
        //             break;
        //         }
        //     }
        // }

        ImGui::SetCursorPos(ImVec2(0.0f, 4.0f));
        ImGui::Text("%s", m_StatustBarText.c_str());
    }
    ImGui::PopStyleVar(3);
    ImGui::End(); // Status Bar

    ImGui::SetNextWindowPos(debuggerPos);
    ImGui::SetNextWindowSize(debuggerSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(ImGui::GetStyle().WindowPadding.x * 0.65f,
                               ImGui::GetStyle().WindowPadding.y * 0.65f));
    // bool isCrashRecording = LvCrashReporter::IsRecording();
    // if (isCrashRecording)
    //     ImGui::PushStyleColor(ImGuiCol_WindowBg,
    //                           ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram));

    bool isDebuggerOpen = false;
    if (ImGui::Begin("Debugger", &_isOpen, statusBarWindowFlag))
    {
        const ImVec2 iconSize = {ImGui::GetFontSize(), ImGui::GetFontSize()};
        const ImVec2 centerPos = ImVec2((debuggerSize.x - iconSize.x) * 0.5f,
                                        (debuggerSize.y - iconSize.y) * 0.5f);

        ImGui::SetCursorPos(centerPos);

        // ImGui::ImageIcon(LvIconType::GIZMO_DEBUG,
        //                  {ImGui::GetFontSize(), ImGui::GetFontSize()},
        //                  {0, 0},
        //                  {1, 1},
        //                  ImGui::GetStyleColorVec4(ImGuiCol_Text));

        if (ImGui::IsWindowHovered() &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            isDebuggerOpen = true;
        }
    }
    // if (isCrashRecording)
    //     ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
    ImGui::End(); // Debugger
    if (isDebuggerOpen)
    {
        ImGui::OpenPopup("Debugger Popup");
    }
    if (ImGui::BeginPopup("Debugger Popup"))
    {
#if defined(__WIN32__)
        ImGui::Dummy({unitSize.x * 120, 0});

        if (ImGui::Checkbox("Crash Recording", &isCrashRecording))
        {
            if (isCrashRecording)
            {
                LvCrashReporter::StartRecord();
            }
            else
            {
                LvCrashReporter::StopRecord();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Send"))
        {
            const LvReportData reportData;
            lv_platform_generate_dump(reportData.dumpFile.c_str(), nullptr);
            LvCrashReporter::Send(reportData);
        }

        ImGui::BeginDisabled(isCrashRecording);
        if (ImGui::Draw("Option", &LvCrashReporter::option))
        {
            LvEditorPreferences::SetArchivable("crashReporterOption",
                                               LvCrashReporter::option);
            LvEditorPreferences::Save();
        }
        ImGui::EndDisabled();
        ImGui::EndPopup();
#else
        ImGui::Text("Not supported");
#endif
    }

    ImGui::SetNextWindowPos(indicatorPos);
    ImGui::SetNextWindowSize(indicatorSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("Indicator", &_isOpen, statusBarWindowFlag))
    {

        if (ImGui::IsWindowHovered() &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            // LvProgress::ShowDetails();
        }

        // const int count = nullptr != _runningProgressCount
        //                       ? lv_atomic_get(_runningProgressCount)
        //                       : 0;
        // ImGuiSpinnerFlags flags = ImGuiSpinnerFlags_None;
        // if (0 < count)
        // {
        //     flags = ImGuiSpinnerFlags_Running;
        // }
        // else
        // {
        //     flags = ImGuiSpinnerFlags_Check;
        // }
        // 
        // auto cursor = ImGui::GetCursorPos();
        // cursor.x += unitSize.x * 2.75f;
        // cursor.y += unitSize.y * 0.15f;
        // 
        // ImGui::SetCursorPos(cursor);
        // ImGuiEx::Spinner("spinner",
        //                  unitSize.x * 2.5f,
        //                  2,
        //                  ImGui::GetColorU32(ImGuiCol_Text),
        //                  flags);
    }
    ImGui::PopStyleVar(3);
    ImGui::End(); // Indicator

    ImGui::PopStyleVar(1); // ImGuiStyleVar_WindowMinSize
}
} // namespace HazelEditor
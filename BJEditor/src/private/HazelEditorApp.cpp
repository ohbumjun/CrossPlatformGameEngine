﻿
#include <Hazel.h>

/*Entry Point -> Application 프로젝트 한 곳에서만 include 해야 한다.
따라서main app 이 있는 곳으로 include 처리한다.
*/
#include "EditorLayer.h"
#include "ProjectSelectLayer.h"
#include "File/FileManager.h"
#include "Hazel/Core/EntryPoint.h"
#include "Hazel/Resource/AssetManagerBase.h"
#include "Hazel/Resource/DefaultResourceManager.h"
#include "EditorAsset/EditorAssetManager.h"
#include "Hazel/Core/EngineContext.h"

namespace Hazel
{
class EditorApplication : public Hazel::Application
{
public:
    EditorApplication(Hazel::ApplicationCommandLineArgs args) 
        : Hazel::Application(Hazel::ApplicationContext{"Hazelnut", "", args})
    {
    }

    virtual void Initialize() override
    {
        // 기본 세팅
        AssetManagerController::Initialize(new HazelEditor::EditorAssetManagerController());
        DefaultResourceManager::LoadDefaultResources();

        // Application 
        Application::Initialize();

        // Editor 용
        const ApplicationContext &applicationContext = Application::Get().GetSpecification();
        const ApplicationCommandLineArgs &applicationCommandLineArgs =applicationContext.GetCommandLineArgs();
        HazelEditor::FileManager::Initialize( applicationCommandLineArgs[0].c_str());
       
         PushLayer(new HazelEditor::ProjectSelectLayer());
    }

    void Finalize()
    {

    }

    ~EditorApplication()
    {
    }
};

Application *CreateApplication(ApplicationCommandLineArgs args)
{
    Application * app = new EditorApplication(args);
    app->Initialize();
    return app;
}
} // namespace Hazel

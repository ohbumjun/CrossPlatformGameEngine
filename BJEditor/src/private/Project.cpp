#include "Project.h"
#include "Hazel/Core/Serialization/JsonSerializer.h"
#include "EditorContext.h"
#include "ProjectContext.h"
#include "Hazel/FileSystem/FileStream.h"
#include "Hazel/FileSystem/DirectorySystem.h"
#include "Hazel/FileSystem/FileModes.h"
#include "Panel/PanelController.h"

namespace HazelEditor
{

	Project::Project(const char *projectPath){};
	Project ::~Project()
	{

	};

	bool Project::IsLoaded()
	{
        return nullptr != BJ_GetProjectContext();
	}
    void Project::Open(const char *projectAbsolutePath)
    {
    }
    void Project::Close()
    {
    }
    void Project::LoadBuiltIn(std::string &state) const
    {
    }
    void Project::Play()
    {
    }
    void Project::Pause()
    {
    }
    void Project::Stop()
    {
    }
    void Project::NextFrame()
    {
    }

    void Project::SaveSettings()
    {
        // const LvEngineContext *engine = lv_engine_get_context();
        // LvProjectSettings &settings = BJ_GetProjectContext()->settings;
        // 
        // if (settings.windowLayoutFilePath.empty())
        // {
        //     settings.windowLayoutFilePath =
        //         PanelController::defaultDockingLayoutPath;
        // }
        // 
        // std::string settingJsonString;
        // Hazel::JsonSerializer archive;
        // 
        // const std::string directory = lv_path_directory(_settingsFilePath.c_str());
        // if (!Hazel::DirectorySystem::ExistDirectoryPath(directory.c_str()))
        // {
        //     Hazel::DirectorySystem::CreateDirectoryPath(directory.c_str());
        // }
        // 
        // Hazel::FileStream settingsFile(_settingsFilePath.c_str(),
        //                                Hazel::FileOpenMode::CREATE);
        // LvOutputStream outputStream(&settingsFile);
        // 
        // settings.appleDevelopmentTeamId =
        //     BJ_GetProjectContext()->settings.appleDevelopmentTeamId;
        // 
        // settings.major = engine->version.major;
        // settings.minor = engine->version.minor;
        // settings.patch = engine->version.patch;
        // 
        // archive.Serialize(Hazel::Reflection::GetTypeID<LvProjectSettings>(),
        //                   &settings);
        // settingJsonString = archive.GetResult();
        // outputStream.WriteText(settingJsonString.c_str());
        // 
        // settingsFile.FlushToFile();
        // settingsFile.End();
    }
    void Project::LoadSettings()
    {
    // Hazel::FileStream settingsFile(_settingsFilePath.c_str(),
    //                                Hazel::FileOpenMode::OPEN);
    // LvInputStream inputStream(&settingsFile);
    // 
    // const std::string settingJsonString = inputStream.ReadToEnd();
    // 
    // settingsFile.FlushToFile();
    // settingsFile.End();
    // 
    // ProjectContext *project = BJ_GetProjectContext();
    // 
    // LvProjectSettings &settings = project->settings;
    // 
    // Hazel::JsonSerializer archive(settingJsonString.c_str());
    // archive.Deserialize(Hazel::Reflection::GetTypeID<LvProjectSettings>(),
    //                     &settings);
    // 
    // LvEngineContext *engineContext = lv_engine_get_context();
    // engineContext->sceneGraphPath = settings.defaultSceneGraphFileName;
    // 
    // if (settings.windowLayoutFilePath.empty())
    // {
    //     settings.windowLayoutFilePath =
    //         LvPanelController::defaultDockingLayoutPath;
    // }
    // 
    // engineContext->isGPUskinning = settings.isGPUskinning;
    // if (settings.useHDR)
    // {
    //     engineContext->colorRenderTextureFormat =
    //         Render::LvPixelFormat::R16G16B16A16_SFLOAT;
    // }
    // else
    // {
    //     engineContext->colorRenderTextureFormat =
    //         Render::LvPixelFormat::R8G8B8A8_UNORM;
    // }
    // engineContext->debugWireFrame = settings.debugWireFrame;
    // engineContext->debugVertexNormal = settings.debugVertexNormal;
    // engineContext->debugFaceNormal = settings.debugFaceNormal;
    // engineContext->debugBone = settings.debugBone;
    // 
    // if (!settings.shaderCacheKey.IsEmpty())
    // {
    // 
    // }
    // 

    } // namespace HazelEditor

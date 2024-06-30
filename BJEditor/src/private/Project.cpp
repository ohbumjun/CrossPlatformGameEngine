#include "Project.h"

namespace HazelEditor
{

	Project::Project(const char *projectPath){};
	Project ::~Project()
	{

	};

	bool Project::IsLoaded()
	{
        return nullptr != lv_project_get_context();
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
        const LvEngineContext *engine = lv_engine_get_context();
        LvProjectSettings &settings = lv_project_get_context()->settings;

        if (LvString::IsNullOrEmpty(settings.windowLayoutFilePath))
        {
            settings.windowLayoutFilePath =
                LvPanelController::defaultDockingLayoutPath;
        }

        LvString settingJsonString;
        LvJsonDomArchive archive;

        const LvString directory = lv_path_directory(_settingsFilePath.c_str());
        if (!lv_directory_exist(directory.c_str()))
        {
            lv_directory_create(directory.c_str());
        }

        LvFileStream settingsFile(_settingsFilePath.c_str(),
                                  LvFileMode::CREATE);
        LvOutputStream outputStream(&settingsFile);

        settings.appleDevelopmentTeamId =
            lv_editor_get_context()->settings.appleDevelopmentTeamId;

        settings.major = engine->version.major;
        settings.minor = engine->version.minor;
        settings.patch = engine->version.patch;
#ifndef _DEBUG
        settings.build = engine->version.build;
#endif

        archive.Serialize(LvReflection::GetTypeId<LvProjectSettings>(),
                          &settings);
        settingJsonString = archive.GetResult();
        outputStream.WriteText(settingJsonString.c_str());

        settingsFile.Flush();
        settingsFile.Close();
    }
    void Project::LoadSettings()
    {
        LvFileStream settingsFile(_settingsFilePath.c_str(), LvFileMode::OPEN);
        LvInputStream inputStream(&settingsFile);

        const LvString settingJsonString = inputStream.ReadToEnd();

        settingsFile.Flush();
        settingsFile.Close();

        LvProjectContext *project = lv_project_get_context();

        LvProjectSettings &settings = project->settings;

        LvJsonDomArchive archive(settingJsonString.c_str());
        archive.Deserialize(LvReflection::GetTypeId<LvProjectSettings>(),
                            &settings);

        LvEngineContext *engineContext = lv_engine_get_context();
        engineContext->sceneGraphPath = settings.defaultSceneGraphFileName;

        if (LvString::IsNullOrEmpty(settings.windowLayoutFilePath))
        {
            settings.windowLayoutFilePath =
                LvPanelController::defaultDockingLayoutPath;
        }

        engineContext->isGPUskinning = settings.isGPUskinning;
        if (settings.useHDR)
        {
            engineContext->colorRenderTextureFormat =
                Render::LvPixelFormat::R16G16B16A16_SFLOAT;
        }
        else
        {
            engineContext->colorRenderTextureFormat =
                Render::LvPixelFormat::R8G8B8A8_UNORM;
        }
        engineContext->debugWireFrame = settings.debugWireFrame;
        engineContext->debugVertexNormal = settings.debugVertexNormal;
        engineContext->debugFaceNormal = settings.debugFaceNormal;
        engineContext->debugBone = settings.debugBone;

        if (!settings.shaderCacheKey.IsEmpty())
    }

    } // namespace HazelEditor

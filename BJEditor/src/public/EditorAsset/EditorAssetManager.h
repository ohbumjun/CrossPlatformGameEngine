#pragma once

#include "EditorAsset.h"
#include "Hazel/Resource/AssetManagerBase.h
#include "hzpch.h"

namespace HazelEditor
{
class EditorAssetProcessor;

class EditorAssetManager
{
    friend class Game;

    // EditorAsset 은 경로별로 guid 를 만들어준다.
    // 그리고 그 guid 를 key 로 하여, EditorAsset 들을 관리한다.
    // static GuidManager* guidManager;
public:
    // 미리 필요한 모든 EditorAsset 파일들을 Load 하고, Cache 파일로 저장하는 함수
    static void LoadEditorAsset(const std::string &relativePath);
    static void ImportEditorAsset(const std::string &relativePath);
    static void PrepareEditorAssets();
    static void Initialize();
    static void Finalize();
    static void CreateEditorAsset();

    // ex) EditorAssets/Textures/Player.png 형태의 경로 정보가 와야 한다.
    static EditorAsset *GetEditorAssetByPath(const std::string &EditorAssetPath);

    static void DeleteEditorAsset(EditorAsset *EditorAsset);

    static EditorAsset *CreateEditorAsset(Hazel::ResourceType type, const std::string &path);

    static std::string GetAbsoluteResourcePath(
        const std::string &relativeResourcePath);

private:
    static void initializeProcessors();

    static std::unordered_map<Hazel::ResourceType, EditorAssetProcessor *>
        m_Loaders;
    static std::unordered_map<std::string /*Resources/~~ 라는 상대경로 ?*/, EditorAsset *>
        m_EditorAssets;
};

} // namespace HazelEditor
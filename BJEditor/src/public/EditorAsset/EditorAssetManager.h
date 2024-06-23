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

    // EditorAsset �� ��κ��� guid �� ������ش�.
    // �׸��� �� guid �� key �� �Ͽ�, EditorAsset ���� �����Ѵ�.
    // static GuidManager* guidManager;
public:
    // �̸� �ʿ��� ��� EditorAsset ���ϵ��� Load �ϰ�, Cache ���Ϸ� �����ϴ� �Լ�
    static void LoadEditorAsset(const std::string &relativePath);
    static void ImportEditorAsset(const std::string &relativePath);
    static void PrepareEditorAssets();
    static void Initialize();
    static void Finalize();
    static void CreateEditorAsset();

    // ex) EditorAssets/Textures/Player.png ������ ��� ������ �;� �Ѵ�.
    static EditorAsset *GetEditorAssetByPath(const std::string &EditorAssetPath);

    static void DeleteEditorAsset(EditorAsset *EditorAsset);

    static EditorAsset *CreateEditorAsset(Hazel::ResourceType type, const std::string &path);

    static std::string GetAbsoluteResourcePath(
        const std::string &relativeResourcePath);

private:
    static void initializeProcessors();

    static std::unordered_map<Hazel::ResourceType, EditorAssetProcessor *>
        m_Loaders;
    static std::unordered_map<std::string /*Resources/~~ ��� ����� ?*/, EditorAsset *>
        m_EditorAssets;
};

} // namespace HazelEditor
#include "EditorAsset/ShaderEditorAssetProcessor.h"
#include "Hazel/FileSystem/DirectorySystem.h"
#include "EditorAsset/ShaderEditorAsset.h"

namespace HazelEditor
{

EditorAsset *ShaderEditorAssetProcessor::CreateEditorAsset(
    const FileId &uuid,
    const std::string &resourcePath,
    bool isBuiltIn)
{
    const std::string &name =
        Hazel::DirectorySystem::GetPathame(resourcePath.c_str());

    // SpriteAsset* asset = &LvObject::Instantiate<LvMaterialAsset, const LvUuid&, const LvString&>(name, uuid, resourcePath, isBuiltIn);
    // SpriteAsset* asset = new SpriteAsset();
    ShaderEditorAsset *asset = nullptr;

    // writeVersion(asset);

    return asset;
};

} // namespace HazelEditor
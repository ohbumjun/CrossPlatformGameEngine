#include "EditorAsset/TextureEditorAssetProcessor.h"
#include "Hazel/FileSystem/DirectorySystem.h"
#include "EditorAsset/TextureEditorAsset.h"

namespace HazelEditor
{

EditorAsset *TextureEditorAssetProcessor::CreateEditorAsset(
    const FileId &uuid,
    const std::string &resourcePath,
    bool isBuiltIn)
{
    const std::string &name =
        Hazel::DirectorySystem::GetPathame(resourcePath.c_str());

    // SpriteAsset* asset = &LvObject::Instantiate<LvMaterialAsset, const LvUuid&, const LvString&>(name, uuid, resourcePath, isBuiltIn);
    // SpriteAsset* asset = new SpriteAsset();
    TextureEditorAsset *asset = nullptr;

    // writeVersion(asset);

    return asset;
};

} // namespace HazelEditor
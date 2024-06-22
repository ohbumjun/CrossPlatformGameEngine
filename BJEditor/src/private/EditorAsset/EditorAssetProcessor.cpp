#include "hzpch.h"
#include "EditorAsset/EditorAssetProcessor.h"

namespace HazelEditor
{
bool EditorAssetProcessor::CanRecord(const std::string &resourcePath)
{
    return false;
}

void EditorAssetProcessor::ImportResource(EditorAsset &EditorAsset,
                                    const std::string &resAbsolutePath)
{
}

EditorAsset *EditorAssetProcessor::CreateEditorAsset(const FileId &fileId,
                                   const std::string &relativResourcePath,
                                   bool isBuiltIn)
{
    return nullptr;
}

}


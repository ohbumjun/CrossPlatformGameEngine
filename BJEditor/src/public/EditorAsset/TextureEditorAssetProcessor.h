#pragma once

#include "EditorAssetProcessor.h"

namespace HazelEditor
{
class TextureEditorAssetProcessor : public EditorAssetProcessor
{
public:
    EditorAsset *TextureEditorAssetProcessor::CreateEditorAsset(
        const FileId &uuid,
        const std::string &resourcePath,
        bool isBuiltIn);

	virtual void WriteResourceFile(EditorAsset &EditorAsset){};

    virtual void WriteCacheFile(EditorAsset &EditorAsset) {}
};

} // namespace HazelEditor
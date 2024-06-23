#pragma once

#include "EditorAssetProcessor.h"

namespace HazelEditor
{
class EditorAsset;

class ShaderEditorAssetProcessor : public EditorAssetProcessor
{
public:
    EditorAsset *ShaderEditorAssetProcessor::CreateEditorAsset(
        const FileId &uuid,
        const std::string &resourcePath,
        bool isBuiltIn);

	virtual void WriteResourceFile(EditorAsset &EditorAsset){};

    virtual void WriteCacheFile(EditorAsset &EditorAsset) {}
};

} // namespace HazelEditor
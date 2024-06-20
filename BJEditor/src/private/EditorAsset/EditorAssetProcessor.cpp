#include "AssetProcessor.h"

bool AssetProcessor::CanRecord(const std::string& resourcePath)
{
    return false;
}

void AssetProcessor::ImportResource(Asset& asset, const std::string& resAbsolutePath)
{
}

Asset* AssetProcessor::CreateAsset(const FileId& fileId, const std::string& relativResourcePath, bool isBuiltIn)
{
    return nullptr;
}

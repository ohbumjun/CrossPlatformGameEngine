#include "EditorAsset/EditorAsset.h"

namespace HazelEditor
{
EditorAsset::EditorAsset(const FileId& fileId, 
	const std::string& resourcePath, Hazel::ResourceType type) : 
    m_FileID(fileId), ResourceContainer(type, resourcePath)
{
}

EditorAsset::~EditorAsset()
{
}



} // namespace Hazel

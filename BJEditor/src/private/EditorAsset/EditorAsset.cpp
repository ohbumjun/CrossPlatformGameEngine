#include "EditorAsset.h"
#include "FileId.h"

namespace HazelEditor
{


EditorAsset::EditorAsset(const FileId& fileId, const std::string& resourcePath, ResourceType type) :
	fileId(fileId), resourcePath(resourcePath), EditorAssetType(type), _prototype(nullptr)
{
}

EditorAsset::~EditorAsset()
{
}

BaseObject* EditorAsset::GetData()
{
	return _prototype;
}

void EditorAsset::SetPrototype(BaseObject* prototype)
{
	_prototype = prototype;
}

BaseObject* EditorAsset::GetPrototype()
{
	return _prototype;
}

} // namespace Hazel

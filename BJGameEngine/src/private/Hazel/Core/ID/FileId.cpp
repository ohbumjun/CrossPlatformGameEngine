#include "Hazel/Core/ID/FileId.h"
#include "Hazel/Utils/SpookyHash.h"

std::unordered_map<uint32, FileId> IdentifierManager::_fileIdMap;

FileId::FileId(const FileId &FileId)
{
    // memcpy(_bytes, FileId._bytes, sizeof(unsigned char) * FileIdByteCount);
}

/*
LvFileId::LvFileId(const LvString& relationPath, uint32 localID )
{
	const uint32 fileHash = LvSpookyHash::Hash32(relationPath.c_str(), relationPath.Length(), 0);
	const uint64 hash = (static_cast<uint64>(localID) << 32) | fileHash;
	memcpy(&_bytes[0], &hash, sizeof(uint64));
}
*/

FileId::FileId(const std::string &relationPath)
{
    const uint32 fileHash =
        SpookyHash::Hash32(relationPath.c_str(), relationPath.length(), 0);
    memcpy(&_bytes[0], &fileHash, sizeof(uint32));
}

FileId IdentifierManager::GetFileId(uint32 key)
{
    assert(CheckContain(key));
    return _fileIdMap[key];
}

bool IdentifierManager::CheckContain(uint32 key)
{
    return _fileIdMap.find(key) != _fileIdMap.end();
}

void IdentifierManager::Clear()
{
    _fileIdMap.clear();
}

void IdentifierManager::createFileId(const std::string &relativePath)
{
}
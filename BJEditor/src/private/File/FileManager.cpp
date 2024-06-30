#include "File/FileManager.h"
#include "Hazel/FileSystem/DirectorySystem.h"
#include "hzpch.h"

namespace HazelEditor
{

std::string FileManager::_projectPath = "";

void FileManager::Initialize(const char *projectPath)
{
    _projectPath = projectPath;

    PathManager::initialize();
}

void FileManager::Finalize()
{

}

std::string FileManager::ToAbsolutePath(const char *relativePath)
{
    return Hazel::DirectorySystem::CombinePath(_projectPath.c_str(), relativePath);
}
} // namespace HazelEditor

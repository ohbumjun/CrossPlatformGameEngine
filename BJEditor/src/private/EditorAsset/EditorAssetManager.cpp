#include "hzpch.h"
#include "EditorAsset/EditorAssetManager.h"
#include "EditorAsset/EditorAssetExtension.h"
#include "EditorAsset/TextureEditorAssetProcessor.h"
#include "EditorAsset/ShaderEditorAssetProcessor.h"
#include "EditorContext.h"
#include "Hazel/FileSystem/DirectorySystem.h"
#include "File/FileManager.h"

namespace HazelEditor
{

    std::unordered_map<Hazel::ResourceType, EditorAssetProcessor *>EditorAssetManager::m_Loaders;
    std::unordered_map<std::string, EditorAsset *>EditorAssetManager::m_EditorAssets;

void EditorAssetManager::initializeProcessors()
{
    m_Loaders[Hazel::ResourceType::IMAGE]    = new TextureEditorAssetProcessor();
    m_Loaders[Hazel::ResourceType::SHADER] = new ShaderEditorAssetProcessor();
}

void EditorAssetManager::PrepareEditorAssets()
{
    // 미리 필요한 파일들을 일단 로드한다.

    // 그리고 Cache 파일로 저장한다.

    LoadEditorAsset("EditorAssets/Ship.png");
    LoadEditorAsset("EditorAssets/Asteroid.png");
    LoadEditorAsset("EditorAssets/Laser.png");
}

void EditorAssetManager::Initialize()
{
    EditorAssetExtension::initialize();

    initializeProcessors();
}

void EditorAssetManager::Finalize()
{
    EditorAssetExtension::finalize();

    // 각 type 별로 EditorAsset Record 객체도 해제해준다.

    for (const auto &EditorAssetInfo : m_EditorAssets)
    {
        EditorAsset *editorAsset = EditorAssetInfo.second;
        Hazel::ResourceType resourceType = editorAsset->GetResourceType();
        EditorAssetProcessor *editorAssetProcessor = m_Loaders[resourceType];
        editorAssetProcessor->DestroyEditorAsset(editorAsset);
    }

    m_EditorAssets.clear();
}

void EditorAssetManager::CreateEditorAsset()
{
}

// SDL_Texture* EditorAssetManager::LoadTexture(const std::string& relativePath)
// {
// 	return TextureEditorAssetManager::loadTexture(relativePath);
// }
//
// SDL_Texture* EditorAssetManager::GetTexture(const std::string& fileName)
// {
// 	return TextureEditorAssetManager::getTexture(fileName);
// }

EditorAsset *EditorAssetManager::GetEditorAssetByPath(const std::string &EditorAssetPath)
{
    if (m_EditorAssets.find(EditorAssetPath) == m_EditorAssets.end())
    {
        return nullptr;
    }

    return m_EditorAssets[EditorAssetPath];
}

void EditorAssetManager::DeleteEditorAsset(EditorAsset *EditorAsset)
{
    Hazel::ResourceType EditorAssetType = EditorAsset->GetResourceType();
    EditorAssetProcessor *EditorAssetProcessor = m_Loaders[EditorAssetType];
    EditorAssetProcessor->DestroyEditorAsset(EditorAsset);

    // 이 부분에 동기화를 진행해줘야 한다.
    m_EditorAssets.erase(EditorAsset->GetResourcePath());
}

EditorAsset *EditorAssetManager::CreateEditorAsset(Hazel::ResourceType type,
                                                   const std::string &path)
{
    return nullptr;
}

std::string EditorAssetManager::GetAbsoluteResourcePath(
    const std::string &relativeResourcePath)
{
    // LV_CHECK(!LvString::IsNullOrEmpty(relativeResourcePath), "relativeResourcePath is null or empty");

    // if (relativeResourcePath.StartsWith(LvEditorContext::Directories::builtinresource))
    // {
    // 	return LvFileManager::ToAbsolutePath(relativeResourcePath.c_str());
    // }

    return FileManager::ToAbsolutePath(
        Hazel::DirectorySystem::CombinePath(EditorContext::Directories::resources,
                                relativeResourcePath.c_str())
            .c_str());
}

void EditorAssetManager::LoadEditorAsset(const std::string &relativePath)
{
    // extension 을 통해 EditorAsset type 추출
    Hazel::ResourceType resourceType =
        EditorAssetExtension::GetResourceTypeByExt(relativePath);

    // "relativePath" 로 EditorAsset 가져오기


    // 해당 EditorAsset type 으로 EditorAsset record 객체 가져오기
    EditorAssetProcessor *EditorAssetProcessor = m_Loaders[resourceType];

    // 없으면 이제 Load
    // 이때 guid 랑 uuid 도 발급해주기
    // 해당 정보로 CreateEditorAsset 호출
    // EditorAsset Record 가져와서, EditorAssetRecord 의 CreateEditorAsset 함수 호출
    // (여기서부터는 나의 의지..?) 그리고 실제 prototype load 도 수행하기.
    EditorAsset *EditorAsset = nullptr;

    if (m_EditorAssets.find(relativePath) != m_EditorAssets.end())
    {
        EditorAsset = m_EditorAssets[relativePath];
    }

    // 원래대로라면 EditorAsset Info 도 따로 만들고, 최초로 만들어준 file id 및 guid 도
    // 해당 info 파일에 넣어줘야 한다.
    // 하지만 현재는 일단 빠르게 개발하기 위해 해당 단계는 건너뛴다.

    if (EditorAsset == nullptr)
    {
        FileId fileId(relativePath);

        // 기존 info 지우고
        // info 다시쓰고 ?

        EditorAsset = EditorAssetProcessor->CreateEditorAsset(fileId, relativePath);
        EditorAsset->onCreate();
    }

    const std::string resAbsPath = GetAbsoluteResourcePath(relativePath);
    EditorAssetProcessor->onLoad(EditorAsset, resAbsPath);

    m_EditorAssets[relativePath] = EditorAsset;

    /*
	LvHashtable<uint64, LvEditorAssetReference> EditorAssetDependencies;
	LvMemoryStream<> contentStream;
	{
		LvOutputStream contentOutputStream(&contentStream);
		Engine::LvBinaryTypedArchive contentArchive(contentOutputStream);
		const bool isSucceed = onImport(EditorAsset, resAbsolutePath, contentArchive, EditorAssetDependencies);
		contentArchive.Flush();
		if (isSucceed == false) return;
	}

	updateHeader(EditorAsset, resAbsolutePath);

	for (const auto& each : EditorAssetDependencies)
	{
		Editor::LvEditorAsset::Header::Dependency def;
		def.id = each.value.id;
		def.name = each.value.GetName();
		def.type = each.value.GetDependenceType();
		def.refCount = each.value.Count();
		def.uuid = each.value.GetUuid().ToString();

		EditorAsset.header.dependencies.Add(def.id, std::move(def));
	}

	LvMemoryStream<> stream;
	LvOutputStream os(&stream);

	EditorAsset.header.Write(os, contentStream.Length());

	contentStream.SetPosition(0);
	os.stream->WriteRaw(contentStream.GetMemory(), contentStream.Length());

	LvFileStream fileStream(cacheAbsolutePath.c_str(), LvFileMode::CREATE);
	fileStream.WriteRaw(stream.GetMemory(), stream.GetPosition());
	fileStream.Flush();
	fileStream.Close();
	*/
}

void EditorAssetManager::ImportEditorAsset(const std::string &relativePath)
{
}

void EditorAssetManagerController::loadAsset(const std::string &relativePath)
{
    EditorAssetManager::LoadEditorAsset(relativePath);
}

} // namespace HazelEditor
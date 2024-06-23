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
    // �̸� �ʿ��� ���ϵ��� �ϴ� �ε��Ѵ�.

    // �׸��� Cache ���Ϸ� �����Ѵ�.

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

    // �� type ���� EditorAsset Record ��ü�� �������ش�.

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

    // �� �κп� ����ȭ�� ��������� �Ѵ�.
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
    // extension �� ���� EditorAsset type ����
    Hazel::ResourceType resourceType =
        EditorAssetExtension::GetResourceTypeByExt(relativePath);

    // "relativePath" �� EditorAsset ��������


    // �ش� EditorAsset type ���� EditorAsset record ��ü ��������
    EditorAssetProcessor *EditorAssetProcessor = m_Loaders[resourceType];

    // ������ ���� Load
    // �̶� guid �� uuid �� �߱����ֱ�
    // �ش� ������ CreateEditorAsset ȣ��
    // EditorAsset Record �����ͼ�, EditorAssetRecord �� CreateEditorAsset �Լ� ȣ��
    // (���⼭���ʹ� ���� ����..?) �׸��� ���� prototype load �� �����ϱ�.
    EditorAsset *EditorAsset = nullptr;

    if (m_EditorAssets.find(relativePath) != m_EditorAssets.end())
    {
        EditorAsset = m_EditorAssets[relativePath];
    }

    // ������ζ�� EditorAsset Info �� ���� �����, ���ʷ� ������� file id �� guid ��
    // �ش� info ���Ͽ� �־���� �Ѵ�.
    // ������ ����� �ϴ� ������ �����ϱ� ���� �ش� �ܰ�� �ǳʶڴ�.

    if (EditorAsset == nullptr)
    {
        FileId fileId(relativePath);

        // ���� info �����
        // info �ٽþ��� ?

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
#pragma once

#include "EditorAsset.h"
#include <string>

namespace HazelEditor
{
class EditorAssetProcessor
{
	friend class EditorAssetManager;

protected:

	/**
	* @brief EditorAsset�� Import�Ѵ�.
	* @details Resource���Ϸκ��� Cache������ �����Ѵ�.
	* @param EditorAsset uuid�� type�� ���õ� EditorAsset
	* 
	* ���� ����
	* - �� �༮�� Resource ���Ͽ��� Load �� �ϱ�
	* - Cache �� ����� ������ �ٸ� �Լ��� ����
	* - �׸��� ��ü ���� Table �� �߰��ϴ� �Լ��� ������ �վ�α�
	*/
	virtual bool CanRecord(const std::string& resourcePath);

	virtual void WriteResourceFile(EditorAsset& EditorAsset) = 0;
	
	virtual void WriteCacheFile(EditorAsset& EditorAsset) = 0;
	
	virtual void WriteInfoFile(EditorAsset* EditorAsset) {};

	// EditorAsset ���� + Cache ���� ���� ?
	// virtual void Import(EditorAsset& EditorAsset, const std::string& resAbsolutePath, const std::string& cacheAbsolutePath);
	
	// �ϴ�, Resource ���Ͽ��� �ҷ����� ����.
	// cache �� ���� �Լ��� Load �ϴ� �͵� ���� �� ����.
	virtual void ImportResource(EditorAsset& EditorAsset, const std::string& resAbsolutePath);
	
	// virtual EditorAsset* CreateEditorAsset(const Uuid& uuid, const std::string& resourcePath, bool isBuiltIn = false) = 0;
	virtual EditorAsset* CreateEditorAsset(const FileId& uuid, const std::string& relativResourcePath, bool isBuiltIn = false) = 0;

	virtual void DestroyEditorAsset(EditorAsset* EditorAsset) {};

	virtual void onLoad(EditorAsset* EditorAsset, const std::string& resAbsolutePath) {};

	Hazel::ResourceType m_EditorAssetType;
};

} // namespace HazelEditor

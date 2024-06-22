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
	* @brief EditorAsset을 Import한다.
	* @details Resource파일로부터 Cache파일을 생성한다.
	* @param EditorAsset uuid와 type이 셋팅된 EditorAsset
	* 
	* 개선 방향
	* - 이 녀석은 Resource 파일에서 Load 만 하기
	* - Cache 로 만드는 과정은 다른 함수를 통해
	* - 그리고 전체 관리 Table 에 추가하는 함수도 별도로 뚫어두기
	*/
	virtual bool CanRecord(const std::string& resourcePath);

	virtual void WriteResourceFile(EditorAsset& EditorAsset) = 0;
	
	virtual void WriteCacheFile(EditorAsset& EditorAsset) = 0;
	
	virtual void WriteInfoFile(EditorAsset* EditorAsset) {};

	// EditorAsset 생성 + Cache 에도 저장 ?
	// virtual void Import(EditorAsset& EditorAsset, const std::string& resAbsolutePath, const std::string& cacheAbsolutePath);
	
	// 일단, Resource 파일에서 불러오는 형태.
	// cache 는 별도 함수로 Load 하는 것도 좋을 것 같다.
	virtual void ImportResource(EditorAsset& EditorAsset, const std::string& resAbsolutePath);
	
	// virtual EditorAsset* CreateEditorAsset(const Uuid& uuid, const std::string& resourcePath, bool isBuiltIn = false) = 0;
	virtual EditorAsset* CreateEditorAsset(const FileId& uuid, const std::string& relativResourcePath, bool isBuiltIn = false) = 0;

	virtual void DestroyEditorAsset(EditorAsset* EditorAsset) {};

	virtual void onLoad(EditorAsset* EditorAsset, const std::string& resAbsolutePath) {};

	Hazel::ResourceType m_EditorAssetType;
};

} // namespace HazelEditor

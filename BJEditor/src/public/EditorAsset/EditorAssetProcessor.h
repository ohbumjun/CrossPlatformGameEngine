#pragma once

#include "Asset.h"
#include <string>
#include "Uuid.h"
#include "FileId.h"

class AssetProcessor
{
	friend class AssetManager;

protected:

	/**
	* @brief Asset을 Import한다.
	* @details Resource파일로부터 Cache파일을 생성한다.
	* @param asset uuid와 type이 셋팅된 asset
	* 
	* 개선 방향
	* - 이 녀석은 Resource 파일에서 Load 만 하기
	* - Cache 로 만드는 과정은 다른 함수를 통해
	* - 그리고 전체 관리 Table 에 추가하는 함수도 별도로 뚫어두기
	*/
	virtual bool CanRecord(const std::string& resourcePath);

	virtual void WriteResourceFile(Asset& asset) = 0;
	
	virtual void WriteCacheFile(Asset& asset) = 0;
	
	virtual void WriteInfoFile(Asset* asset) {};

	// Asset 생성 + Cache 에도 저장 ?
	// virtual void Import(Asset& asset, const std::string& resAbsolutePath, const std::string& cacheAbsolutePath);
	
	// 일단, Resource 파일에서 불러오는 형태.
	// cache 는 별도 함수로 Load 하는 것도 좋을 것 같다.
	virtual void ImportResource(Asset& asset, const std::string& resAbsolutePath);
	
	// virtual Asset* CreateAsset(const Uuid& uuid, const std::string& resourcePath, bool isBuiltIn = false) = 0;
	virtual Asset* CreateAsset(const FileId& uuid, const std::string& relativResourcePath, bool isBuiltIn = false) = 0;

	virtual void DestroyAsset(Asset* asset) {};

	virtual void onLoad(Asset* asset, const std::string& resAbsolutePath) {};

	AssetType m_AssetType;
};


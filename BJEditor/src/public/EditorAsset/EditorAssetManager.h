#pragma once

#include "Asset.h"
#include "Common.h"
#include "Guid.h"
#include "TextureAssetManager.h"

class AssetProcessor;
struct SDL_Texture;

class AssetManager
{
	friend class Game;

	// Asset 은 경로별로 guid 를 만들어준다.
	// 그리고 그 guid 를 key 로 하여, Asset 들을 관리한다.
	// static GuidManager* guidManager;
public :

	// 미리 필요한 모든 asset 파일들을 Load 하고, Cache 파일로 저장하는 함수
	static void LoadAsset(const std::string& relativePath);
	static void ImportAsset(const std::string& relativePath);
	static void PrepareAssets();
	static void Initialize();
	static void Finalize();
	static void CreateAsset();

	// ex) Assets/Textures/Player.png 형태의 경로 정보가 와야 한다.
	static Asset* GetAssetByPath(const std::string& assetPath);

	static void DeleteAsset(Asset* asset);

	static Asset* CreateAsset(AssetType type, const std::string& path);

	static std::string GetAbsoluteResourcePath(const std::string& relativeResourcePath);
private :
	
	static void initializeProcessors();
	
	static std::unordered_map<AssetType, AssetProcessor*> _loaders;
	static std::unordered_map<std::string/*Resources/~~ 라는 상대경로 ?*/, Asset*> _assets;
};


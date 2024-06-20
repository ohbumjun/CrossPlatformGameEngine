#pragma once

#include "Common.h"
#include "Asset.h"
#include "AssetAttribute.h"

class AssetExtension
{
	friend class AssetManager;

	static AssetType GetAssetTypeByExtension(const std::string& extension);

private :
	static void initialize();
	static void finalize();
	static void addExtension();

	// static std::vector<AssetAttribute*> _attributies;
	// static std::unordered_map<size_t, AssetAttribute*> _attributies;
	static std::unordered_map<std::string, AssetType> _extensionMap;
};


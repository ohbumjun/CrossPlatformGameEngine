#pragma once

#include "hzpch.h"
#include "EditorAsset.h"
#include "EditorAssetAttribute.h"

namespace HazelEditor
{
	class EditorAssetExtension
{
     friend class EditorAssetManager;

	 public:
	static Hazel::ResourceType  GetResourceTypeByExt(const std::string& extension);

	static void Initialize();
	static void Finalize();
private :
	static void addExtension();

	// static std::vector<AssetAttribute*> _attributies;
	// static std::unordered_map<size_t, AssetAttribute*> _attributies;
    static std::unordered_map<std::string, Hazel::ResourceType> _extensionMap;
};

	}
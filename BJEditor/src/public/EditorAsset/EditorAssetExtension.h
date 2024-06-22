#pragma once

#include "hzpch.h"
#include "EditorAsset.h"
#include "EditorAssetAttribute.h"

namespace HazelEditor
{
	class EditorAssetExtension
{
        friend class EditorAssetManager;

	static Hazel::ResourceType  GetResourceTypeByExt(const std::string& extension);

private :
	static void initialize();
	static void finalize();
	static void addExtension();

	// static std::vector<AssetAttribute*> _attributies;
	// static std::unordered_map<size_t, AssetAttribute*> _attributies;
    static std::unordered_map<std::string, Hazel::ResourceType> _extensionMap;
};

	}
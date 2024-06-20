#pragma once

#include "Common.h"
#include "Asset.h"

// struct AssetAttribute : public LvAttribute
struct AssetAttribute 
{
	AssetType assetType;

	// LvTypeId prototype;
	size_t prototype;

	// LvTypeId recordType;
	size_t recordType;

	std::string extension;

	AssetAttribute(const AssetType& assetType, const size_t prototype, const size_t recordType, const std::string& ext)
		: assetType(assetType)
		, prototype(prototype)
		, recordType(recordType)
		, extension(ext)
	{}
};
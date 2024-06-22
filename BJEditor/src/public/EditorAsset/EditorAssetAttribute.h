#pragma once

#include "hzpch.h"
#include "EditorAsset/EditorAsset.h"

namespace HazelEditor
{

// struct EditorAssetAttribute : public LvAttribute
struct EditorAssetAttribute
{
    Hazel::ResourceType m_AssetType;

    // LvTypeId prototype;
    size_t m_PrototypeType;

    // LvTypeId recordType;
    size_t m_ProcessorType;

    std::string extension;

    EditorAssetAttribute(const Hazel::ResourceType &assetType,
                   const size_t prototype,
                   const size_t recordType,
                   const std::string &ext)
        : m_AssetType(assetType), m_PrototypeType(prototype),
          m_ProcessorType(recordType),
          extension(ext)
    {
    }
};
} // namespace HazelEditor
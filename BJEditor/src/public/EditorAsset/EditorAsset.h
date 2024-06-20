#pragma once

#include "FileId.h"
#include <cstdint>
#include "Hazel/Resource/ResourceContainer.h"


namespace HazelEditor
{

class EditorAsset : public Hazel::ResourceContainer
{
    friend class EditorEditorAssetManager;

public:
    EditorAsset(const FileId &fileId,
          const std::string &resourcePath,
          EditorAssetType type);
    ~EditorAsset();

    BaseObject *GetData();
    void SetPrototype(BaseObject *prototype);
    BaseObject *GetPrototype();
    const std::string &GetResourcePath()
    {
        return resourcePath;
    }
    EditorAssetType GetEditorAssetType()
    {
        return EditorAssetType;
    }

protected:
    virtual void onCreate(){};

private:
    ResourceType EditorAssetType;
    std::string resourcePath;
    FileId fileId;
    /**
	*EditorAsset 의 size
	*/
    uint64_t size;

    /**
	*해당 EditorAsset의 info (Unity로 치면 meta file). EditorAsset resource의 외부적으로 저장해야하는 값이나 check sum값등을 들고있다.
	*/
    class EditorAssetInfo *info;
    // guid
    // fileId
};

} // namespace Hazel
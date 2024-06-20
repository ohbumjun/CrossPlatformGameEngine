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
	*EditorAsset �� size
	*/
    uint64_t size;

    /**
	*�ش� EditorAsset�� info (Unity�� ġ�� meta file). EditorAsset resource�� �ܺ������� �����ؾ��ϴ� ���̳� check sum������ ����ִ�.
	*/
    class EditorAssetInfo *info;
    // guid
    // fileId
};

} // namespace Hazel
#pragma once

#include "Hazel/Core/ID/FileId.h"
#include <cstdint>
#include "Hazel/Resource/ResourceContainer.h"


namespace HazelEditor
{

class EditorAsset : public Hazel::ResourceContainer
{
    friend class EditorAssetManager;

public:
    EditorAsset(const FileId &fileId,
          const std::string &resourcePath,
          Hazel::ResourceType type);
    ~EditorAsset();


protected:
    virtual void onCreate(){};

private:
    FileId m_FileID;

    /**
	*해당 EditorAsset의 info (Unity로 치면 meta file). EditorAsset resource의 외부적으로 저장해야하는 값이나 check sum값등을 들고있다.
	*/
    class EditorAssetInfo *m_Info;
    // guid
    // fileId
};

} // namespace Hazel
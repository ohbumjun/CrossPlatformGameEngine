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
	*�ش� EditorAsset�� info (Unity�� ġ�� meta file). EditorAsset resource�� �ܺ������� �����ؾ��ϴ� ���̳� check sum������ ����ִ�.
	*/
    class EditorAssetInfo *m_Info;
    // guid
    // fileId
};

} // namespace Hazel
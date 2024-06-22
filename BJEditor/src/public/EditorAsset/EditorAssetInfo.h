#pragma once

#include "EditorAsset.h"
#include <string>
#include <cstdint>

namespace HazelEditor
{
class EditorAssetInfo
{
public:
    // struct SubEditorAssetInfo : LvArchivable
    struct SubEditorAssetInfo
    {
        // void Serialize(Engine::LvArchive & archive) override;
        // void Deserialize(Engine::LvArchive& archive) override;

        std::string name;
        uint32_t localId = 0;
        uint8_t EditorAssetType = 0;
    };
    friend class EditorAsset;

    EditorAssetInfo() = default;
    EditorAssetInfo(const EditorAssetInfo &rhs) = default;
    EditorAssetInfo &operator=(const EditorAssetInfo &rhs) = default;


    virtual void CopyFrom(const EditorAssetInfo *info)
    {
        // NOTICE �ڽ� ��ü�� ���� �θ�Ŭ���� �����ͷ� ������Կ��� �����(*base = *obj) Base�� operator=�� ����Ǿ� ����� ���簡 �ȵȴ�.
        // Base* base1 = new Derived();
        // Base* base2 = new Derived();
        // *base1 = *base2;
        // ���� ���� ��Ȳ���� Derived�� operaotr=�� ȣ���ϱ� ���� 2���� ����� �ִ�.
        // 1. virtual void CopyFrom(Base* rhs) �����Ͽ� override
        // 2. virtual Base& operator=(Base& rhs) �����Ͽ� �ڽİ�ü�� ���� �ñ״����� �޼��带 override
        // ���⼭ 1���� ����� ����Ͽ� �����Ѵ�. (TODO operator=�� �ϴ� ���� ���� ������ ���Ŀ� �������� ���� �����ʿ�)

        //LvObject�� ���Ѱ� �������� �ʴ´�. Why? LvObject::Instantiate�� �� �� �־��ֱ� ������. ������ ���ú����� ����� ����������.
        version = info->version;

        // guid�� �������� �ʴ´�!
        //guid = info->guid;
    }

    // TODO ��Ʈ �������� �ٲ���.
    enum class State : uint8_t
    {
        NONE = 0,
        FILE_NOT_EXIST, // .Inf�� �������� ����.
        GUID_NOT_MATCH,
        CHECKSUM_NOT_MATCH
    };

    constexpr static const char *FileExtension = ".inf";

    /**
	* EditorAsset info �� serialize ����
	*/
    std::string version;

    /**
	* ����� EditorAsset�� guid
	*/
    std::string guid;

    /**
	*EditorAssetInfo�� �����մϴ�.
	* type �����Ϸ��� EditorAssetInfo Type
	* @param path EditorAsset info�� ��ġ
	*/
    // static EditorAssetInfo* Create(LvTypeId type, const std::string& path);

    /**
	* EditorAsset�� �ش��ϴ� .inf�� ����ϴ�.
	* EditorAsset .inf�� ������ target EditorAsset
	* @return .Inf ���� �������θ� ��ȯ�մϴ�.
	*/
    inline static bool Save(const EditorAsset *EditorAsset)
    {
        // if (!(EditorAsset != nullptr && EditorAsset->info != nullptr))
        // {
        // 	return false;
        // }
        // return SaveFile(EditorAsset);
    }
};

} // namespace HazelEditor
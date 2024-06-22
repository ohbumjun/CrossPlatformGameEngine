#pragma once

#include "hzpch.h"

// File IDs: Internal identifier for faster lookups within a project (like call numbers).
// File경로마다 고유한 Guid 가 만들어진다.
class FileId
{
    friend class IdentifierManager;

public:
    // 아래 2줄은 Guid 가 local ID 까지 포함했을 때의 코드이다.
    // static constexpr size_t FileIdByteCount = 8;
    // static constexpr size_t FileIdStringCount = 16;
    // explicit Guid(const LvString& relationPath, uint32 localID);

    static constexpr size_t FileIdStringCount = 8;
    static constexpr size_t FileIdByteCount = 4;

    struct Hash96
    {
        uint32 hash1;
        uint64 hash2;
    };

    FileId() = default;
    // 여기 기본 생성자로 들어오면 안되는데
    FileId(const FileId &FileId);
    FileId(const std::string &relativePath);
    ~FileId() = default;

    inline std::string ToString() const
    {
        char base16[FileIdStringCount + 1];
        toBase16(_bytes, base16);
        return std::string(base16, FileIdStringCount);
    }
    inline uint32 GetHash() const
    {
        uint32 hash1 = 0;
        memcpy(&hash1, &_bytes[0], sizeof(unsigned char) * FileIdByteCount);
        return hash1;
    }
    inline uint32 GetHash32() const
    {
        uint32 hash1 = 0;
        memcpy(&hash1, &_bytes[0], sizeof(unsigned char) * 4);
        return hash1;
    }
    // inline uint64 GetHash64() const
    // {
    // 	uint64 hash2 = 0;
    // 	memcpy(&hash2, &_bytes[4], sizeof(unsigned char) * 8);
    // 	return hash2;
    // }
    // inline Hash96 GetHash96() const
    // {
    // 	Hash96 r;
    // 	r.hash1 = GetHash32();
    // 	r.hash2 = GetHash64();
    //
    // 	return r;
    // }
private:
    // binary check sum 을 base16 으로 변환한다.
    // 참고 : base16 이란 16진수 형태로 표현한다는 것을 의미한다.
    inline static void toBase16(const unsigned char checksum[FileIdByteCount],
                                char out[FileIdStringCount + 1])
    {
        /*
		* % 02 는, unsigned char 를 2개의 16진수로 표현하도록 한다.
		* 
		* 2  : 최소 2자리를 출력하도록 한다.
		* 0  : 빈자리를 0 으로 채운다. space 로 채우는 것이 아니다.
		* 02 : 최소 2자리로 출력하고, 빈자리는 없이 0 으로 채운다.
		* 
		* ex) unsigned char value = 15;
		*	  printf("%02x\n", value);  // Prints: 0f
		*	 value = 255;
		*	 printf("%02x\n", value);  // Prints: ff
		*/

        const auto error = snprintf(
            out,
            FileIdStringCount + 1
            // , "%02x%02x%02x%02x%02x%02x%02x%02x"
            ,
            "%02x%02x%02x%02x"
            // , checksum[0], checksum[1], checksum[2], checksum[3], checksum[4], checksum[5], checksum[6], checksum[7]);
            ,
            checksum[0],
            checksum[1],
            checksum[2],
            checksum[3]);

        // snprintf 가 실패하면 -1 을 리턴한다.
        // assert(-1 != error, "buffer or format is nullptr!!");
        assert(-1 != error);
    }

    unsigned char _bytes[FileIdByteCount] = {
        0,
    };
};


class IdentifierManager
{
    friend class AssetManager;

public:
    static FileId GetFileId(uint32 key);
    static bool CheckContain(uint32 key);
    static void Clear();

private:
    //key 를 guid 로 해야 한다.
    static void createFileId(const std::string &relativePath);
    static std::unordered_map<uint32, FileId> _fileIdMap;
};

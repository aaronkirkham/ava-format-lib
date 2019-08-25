#pragma once

#include <cstdint>
#include <map>
#include <vector>

namespace ava::AvalancheDataFormat
{
static constexpr uint32_t ADF_MAGIC = 0x41444620; // ADF

enum class EAdfType : uint32_t {
    SCALAR       = 0,
    STRUCTURE    = 1,
    POINTER      = 2,
    ARRAY        = 3,
    INLINE_ARRAY = 4,
    STRING       = 5,
    UNUSED       = 6,
    BITFIELD     = 7,
    ENUM         = 8,
    STRING_HASH  = 9,
    DEFERRED     = 10,
};

enum class ScalarType : uint16_t {
    SIGNED   = 0,
    UNSIGNED = 1,
    FLOAT    = 2,
};

enum EHeaderFlags {
    RELATIVE_OFFSETS_EXISTS        = (1 << 0),
    HAS_INLINE_ARRAY_WITH_POINTERS = (1 << 1),
};

#pragma pack(push, 8)
template <typename T> struct SAdfArray {
    T*       m_Data;
    uint32_t m_Count;
};

struct SAdfDeferredPtr {
    void*    m_Ptr;
    uint32_t m_Type;
};

static_assert(sizeof(SAdfArray<void>) == 0x10, "SAdfArray alignment is wrong!");
static_assert(sizeof(SAdfDeferredPtr) == 0x10, "SAdfDeferredPtr alignment is wrong!");
#pragma pack(pop)

#pragma pack(push, 1)
struct AdfHeader {
    uint32_t m_Magic = ADF_MAGIC;
    uint32_t m_Version;
    uint32_t m_InstanceCount;
    uint32_t m_FirstInstanceOffset;
    uint32_t m_TypeCount;
    uint32_t m_FirstTypeOffset;
    uint32_t m_StringHashCount;
    uint32_t m_FirstStringHashOffset;
    uint32_t m_StringCount;
    uint32_t m_FirstStringDataOffset;
    uint32_t m_FileSize;
    uint32_t unknown;
    uint32_t m_Flags;
    uint32_t m_IncludedLibraries;
    uint32_t : 32;
    uint32_t : 32;
    const char* m_Description;
};

struct AdfInstance {
    uint32_t m_NameHash;
    uint32_t m_TypeHash;
    uint32_t m_PayloadOffset;
    uint32_t m_PayloadSize;
    uint64_t m_Name;
};

struct AdfMember {
    uint64_t m_Name;
    uint32_t m_TypeHash;
    uint32_t m_Align;
    uint32_t m_Offset : 24;
    uint32_t m_BitOffset : 8;
    uint32_t m_Flags;
    uint64_t m_DefaultValue;
};

struct AdfEnum {
    uint64_t m_Name;
    int32_t  m_Value;
};

// warning C4200: nonstandard extension used: zero-sized array in struct/union
#pragma warning(push)
#pragma warning(disable : 4200)

struct AdfType {
    EAdfType   m_Type;
    uint32_t   m_Size;
    uint32_t   m_Align;
    uint32_t   m_TypeHash;
    uint64_t   m_Name;
    uint16_t   m_Flags;
    ScalarType m_ScalarType;
    uint32_t   m_SubTypeHash;
    uint32_t   m_ArraySizeOrBitCount;
    uint32_t   m_MemberCount;
    AdfMember  m_Members[0];

    AdfEnum& Enum(uint32_t i)
    {
        return ((AdfEnum*)(void*)&m_Members[0])[i];
    }

    const AdfEnum& Enum(uint32_t i) const
    {
        return ((const AdfEnum*)(void*)&m_Members[0])[i];
    }

    const size_t DataSize() const
    {
        return (sizeof(AdfType) + ((m_Type != EAdfType::ENUM ? sizeof(AdfMember) : sizeof(AdfEnum)) * m_MemberCount));
    }
};

#pragma warning(pop)

struct SInstanceInfo {
    uint32_t    m_NameHash;
    uint32_t    m_TypeHash;
    const char* m_Name;
    const void* m_Instance;
    uint32_t    m_InstanceSize;
};
#pragma pack(pop)

static_assert(sizeof(AdfHeader) == 0x48, "AdfHeader alignment is wrong.");
static_assert(sizeof(AdfInstance) == 0x18, "AdfInstance alignment is wrong.");
static_assert(sizeof(AdfMember) == 0x20, "AdfMember alignment is wrong.");
static_assert(sizeof(AdfEnum) == 0xC, "AdfEnum alignment is wrong.");
static_assert(sizeof(AdfType) == 0x28, "AdfType alignment is wrong.");
static_assert(sizeof(SInstanceInfo) == 0x1C, "SInstanceInfo alignment is wrong.");

void ParseHeader(const std::vector<uint8_t>& buffer, AdfHeader* out_header, const char** out_description = nullptr);

class ADF
{
  private:
    std::vector<uint8_t>            m_Buffer;
    AdfHeader*                      m_Header;
    std::vector<AdfType*>           m_Types;
    std::vector<std::string>        m_Strings;
    std::map<uint32_t, std::string> m_StringHashes;

    void     AddBuiltInType(EAdfType type, ScalarType scalar_type, uint32_t size, const char* name, uint16_t flags = 3);
    AdfType* FindType(const uint32_t type_hash);

    const char* GetString(uint64_t index, const AdfHeader* header, const std::vector<uint8_t>& buffer)
    {
        const char* data = (const char*)buffer.data();

        const char*    strings = &data[header->m_FirstStringDataOffset + header->m_StringCount];
        const uint8_t* lengths = (const uint8_t*)&data[header->m_FirstStringDataOffset];

        uint64_t offset = 0;
        for (uint64_t i = 0; i < index; ++i) {
            offset += (lengths[i] + 1);
        }

        const auto rel_index =
            std::distance(m_Strings.begin(), std::find(m_Strings.begin(), m_Strings.end(), &strings[offset]));
        return m_Strings[rel_index].c_str();
    }

    uint64_t GetStringIndex(const char* string)
    {
        return std::distance(m_Strings.begin(), std::find(m_Strings.begin(), m_Strings.end(), string));
    }

  public:
    ADF(const std::vector<uint8_t>& buffer);
    virtual ~ADF();

    void AddTypes(const std::vector<uint8_t>& buffer);

    void GetInstance(uint32_t index, SInstanceInfo* out_instance_info);
    void ReadInstance(uint32_t name_hash, uint32_t type_hash, void** out_instance);
    void ReadInstance(const SInstanceInfo& instance_info, void** out_instance);

    const char* HashLookup(const uint32_t hash)
    {
        const auto it = m_StringHashes.find(hash);
        if (it == m_StringHashes.end()) {
            return "";
        }

        return it->second.c_str();
    }
};
}; // namespace ava::AvalancheDataFormat

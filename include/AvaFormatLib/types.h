#ifndef JCMR_AVAFORMATLIB_TYPES_H_HEADER_GUARD
#define JCMR_AVAFORMATLIB_TYPES_H_HEADER_GUARD

#include <cstdint>
#include <string>

namespace ava
{
#pragma pack(push, 1)
struct SObjectNameHash {
    uint16_t m_First  = 0;
    uint16_t m_Second = 0;
    uint16_t m_Third  = 0;

    const bool valid() const { return ((m_Third | ((m_Second | (m_First << 0x10)) << 0x10)) & 0xFFFFFFFFFFFF) != 0; }

    uint64_t to_uint64() { return (m_Third | ((m_Second | (m_First << 0x10)) << 0x10)) << 0x10; }

    std::string to_string()
    {
        char buf[13] = {0};
        sprintf_s(buf, "%012llx", to_uint64());
        return buf;
    }
};

struct SObjectID {
    SObjectNameHash m_Hash;
    uint16_t        m_UserData = 0;

    SObjectID() = default;
    SObjectID(const uint64_t object_id)
    {
        m_UserData      = (uint16_t)object_id;
        m_Hash.m_First  = (object_id >> 0x30) & 0xFFFF;
        m_Hash.m_Second = (object_id >> 0x20) & 0xFFFF;
        m_Hash.m_Third  = (object_id >> 0x10) & 0xFFFF;
    }

    SObjectID(const uint64_t object_id, uint16_t user_data)
    {
        m_UserData      = user_data;
        m_Hash.m_First  = (object_id >> 0x30) & 0xFFFF;
        m_Hash.m_Second = (object_id >> 0x20) & 0xFFFF;
        m_Hash.m_Third  = (object_id >> 0x10) & 0xFFFF;
    }

    uint64_t to_uint64() const
    {
        return m_UserData
               | ((m_Hash.m_Third | ((m_Hash.m_Second | ((uint64_t)m_Hash.m_First << 0x10)) << 0x10)) << 0x10);
    }

    // NOTE : same as to_uint64(), but endian flipped for saving back to binary files
    uint64_t to_binary_uint64() const
    {
        return m_Hash.m_First
               | ((m_Hash.m_Second | ((m_Hash.m_Third | ((uint64_t)m_UserData << 0x10)) << 0x10)) << 0x10);
    }

    std::string to_string()
    {
        char buf[17] = {0};
        sprintf_s(buf, "%016llX", to_uint64());
        return buf;
    }
};
#pragma pack(pop)

using SEventID = SObjectID;

#pragma pack(push, 1)
struct SPackedAttribute {
    int32_t m_Format;
    float   m_Scale;
    float   m_UV0Extent[2];
    float   m_UV1Extent[2];
    float   m_ColorExtent;
    uint8_t m_Color[4];
};
#pragma pack(pop)

static_assert(sizeof(SPackedAttribute) == 0x20, "SPackedAttribute alignment is wrong!");
} // namespace ava

#endif // JCMR_AVAFORMATLIB_TYPES_H_HEADER_GUARD

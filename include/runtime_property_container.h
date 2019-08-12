#pragma once

#include <cstdint>
#include <vector>

namespace ava::RuntimePropertyContainer
{
static constexpr uint32_t RTPC_MAGIC = 0x43505452; // RTPC

enum class PropertyType : uint8_t {
    UNASSIGNED = 0,
    INTEGER,
    FLOAT,
    STRING,
    VEC2,
    VEC3,
    VEC4,
    MAT4X4 = 8,
    INTEGER_LIST,
    FLOAT_LIST,
    BYTE_LIST,
    OBJECT_ID = 13,
    EVENT,
    NUM_TYPES,
};

#pragma pack(push, 1)
struct RtpcHeader {
    uint32_t m_Magic;
    uint32_t m_Version;
};

struct RtpcNode {
    uint32_t m_Namehash;
    uint32_t m_DataOffset;
    uint16_t m_PropertyCount;
    uint16_t m_InstanceCount;
};

struct RtpcProperty {
    uint32_t     m_Namehash;
    uint32_t     m_DataOffset;
    PropertyType m_Type;
};
#pragma pack(pop)

static_assert(sizeof(RtpcHeader) == 0x8, "RtpcHeader alignment is wrong!");
static_assert(sizeof(RtpcNode) == 0xC, "RtpcNode alignment is wrong!");
static_assert(sizeof(RtpcProperty) == 0x9, "RtpcProperty alignment is wrong!");

void Parse(const std::vector<uint8_t>& buffer);
}; // namespace ava::RuntimePropertyContainer

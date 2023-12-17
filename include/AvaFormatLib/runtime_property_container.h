#pragma once

#include "error.h"

#include <any>
#include <cstdint>
#include <vector>

namespace ava::RuntimePropertyContainer
{
static constexpr uint32_t RTPC_MAGIC        = 0x43505452; // RTPC
static constexpr uint32_t RTPC_PADDING_BYTE = 0x50;

enum EVariantType : uint8_t {
    T_VARIANT_UNASSIGNED    = 0x0,
    T_VARIANT_INTEGER       = 0x1,
    T_VARIANT_FLOAT         = 0x2,
    T_VARIANT_STRING        = 0x3,
    T_VARIANT_VEC2          = 0x4,
    T_VARIANT_VEC3          = 0x5,
    T_VARIANT_VEC4          = 0x6,
    T_VARIANT__DO_NOT_USE_1 = 0x7,
    T_VARIANT_MAT4x4        = 0x8,
    T_VARIANT_VEC_INTS      = 0x9,
    T_VARIANT_VEC_FLOATS    = 0xA,
    T_VARIANT_VEC_BYTES     = 0xB,
    T_VARIANT__DO_NOT_USE_2 = 0xC,
    T_VARIANT_OBJECTID      = 0xD,
    T_VARIANT_VEC_EVENTS    = 0xE,
};

static const char* VariantTypeToString(EVariantType type)
{
    switch (type) {
        case T_VARIANT_UNASSIGNED: return "T_VARIANT_UNASSIGNED";
        case T_VARIANT_INTEGER: return "T_VARIANT_INTEGER";
        case T_VARIANT_FLOAT: return "T_VARIANT_FLOAT";
        case T_VARIANT_STRING: return "T_VARIANT_STRING";
        case T_VARIANT_VEC2: return "T_VARIANT_VEC2";
        case T_VARIANT_VEC3: return "T_VARIANT_VEC3";
        case T_VARIANT_VEC4: return "T_VARIANT_VEC4";
        case T_VARIANT__DO_NOT_USE_1: return "T_VARIANT__DO_NOT_USE_1";
        case T_VARIANT_MAT4x4: return "T_VARIANT_MAT4x4";
        case T_VARIANT_VEC_INTS: return "T_VARIANT_VEC_INTS";
        case T_VARIANT_VEC_FLOATS: return "T_VARIANT_VEC_FLOATS";
        case T_VARIANT_VEC_BYTES: return "T_VARIANT_VEC_BYTES";
        case T_VARIANT__DO_NOT_USE_2: return "T_VARIANT__DO_NOT_USE_2";
        case T_VARIANT_OBJECTID: return "T_VARIANT_OBJECTID";
        case T_VARIANT_VEC_EVENTS: return "T_VARIANT_VEC_EVENTS";
    }

    return "";
}

#pragma pack(push, 1)
struct RtpcHeader {
    uint32_t m_Magic   = RTPC_MAGIC;
    uint32_t m_Version = 1;
};

struct RtpcContainer {
    uint32_t m_Key;
    uint32_t m_DataOffset;
    uint16_t m_NumVariants;
    uint16_t m_NumContainers;
};

struct RtpcContainerVariant {
    uint32_t     m_Key;
    uint32_t     m_DataOffset;
    EVariantType m_Type;
};
#pragma pack(pop)

static_assert(sizeof(RtpcHeader) == 0x8, "RtpcHeader alignment is wrong!");
static_assert(sizeof(RtpcContainer) == 0xC, "RtpcContainer alignment is wrong!");
static_assert(sizeof(RtpcContainerVariant) == 0x9, "RtpcContainerVariant alignment is wrong!");

struct Variant {
    uint32_t     m_NameHash = 0xFFFFFFFF;
    EVariantType m_Type;
    std::any     m_Value;

    static Variant invalid() { return Variant(); }

    Variant() = default;
    Variant(uint32_t namehash, EVariantType type)
        : m_NameHash(namehash)
        , m_Type(type)
    {
    }

    template <typename T> T&       as() { return std::any_cast<T&>(m_Value); }
    template <typename T> const T& as() const { return std::any_cast<T&>(m_Value); }

    const bool valid() const { return m_NameHash != 0xFFFFFFFF; }
};

struct Container {
    uint32_t               m_NameHash = 0xFFFFFFFF;
    std::vector<Container> m_Containers;
    std::vector<Variant>   m_Variants;

    static Container invalid() { return Container(); }

    Container() = default;
    Container(uint32_t namehash)
        : m_NameHash(namehash)
    {
    }

    const Container& GetContainer(uint32_t namehash, bool look_in_child_containers = true);
    Variant&         GetVariant(uint32_t namehash, bool look_in_child_containers = true);

    const bool valid() const { return m_NameHash != 0xFFFFFFFF; }
};

/**
 * Parse an RTPC file
 *
 * @param buffer Input buffer containing a raw RTPC file buffer
 * @param out_containers Pointer to a Container of the root node
 */
Result Parse(const std::vector<uint8_t>& buffer, Container* out_root_container);

/**
 * Write an RTPC file
 *
 * @param container Input container for the RTPC file
 * @param out_buffer Pointer to a byte buffer to write the RTPC file data to
 * @param flags Writer flags (EWriteFlags)
 */
Result Write(const Container& root_container, std::vector<uint8_t>* out_buffer);
}; // namespace ava::RuntimePropertyContainer

#pragma once

#include <cstdint>
#include <vector>

namespace ava::RuntimePropertyContainer
{
static constexpr uint32_t RTPC_MAGIC = 0x43505452; // RTPC

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

#pragma pack(push, 1)
struct RtpcHeader {
    uint32_t m_Magic;
    uint32_t m_Version;
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

class RuntimeContainer
{
  private:
    RtpcContainer* m_Container = nullptr;

  public:
    RuntimeContainer(const std::vector<uint8_t>& buffer);
    virtual ~RuntimeContainer();

    void GetContainer(const uint32_t key);
    void GetVariant(const uint32_t key);
};
}; // namespace ava::RuntimePropertyContainer

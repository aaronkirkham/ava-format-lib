#pragma once

#include <cstdint>
#include <vector>

namespace ava::RuntimePropertyContainer
{
static constexpr uint32_t RTPC_MAGIC = 0x43505452; // RTPC

enum PropertyType : uint8_t {
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

struct RtpcContainer {
    uint32_t m_Key;
    uint32_t m_DataOffset;
    uint16_t m_VariantCount;
    uint16_t m_ContainerCount;
};

struct RtpcContainerVariant {
    uint32_t     m_Key;
    uint32_t     m_DataOffset;
    PropertyType m_Type;
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

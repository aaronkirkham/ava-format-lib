#include <runtime_property_container.h>

#include <util/byte_array_buffer.h>
#include <util/hashlittle.h>

namespace ava::RuntimePropertyContainer
{
RTPC::RTPC(const std::vector<uint8_t>& buffer)
{
    if (buffer.empty()) {
        // E_INVALID_ARGUMENT;
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    RtpcHeader header{};
    stream.read((char*)&header, sizeof(RtpcHeader));
    if (header.m_Magic != RTPC_MAGIC) {
        // E_RTPC_INVALID_MAGIC;
    }

    //
    m_Container = (RtpcContainer*)(buffer.data() + sizeof(RtpcHeader));
}

RTPC::~RTPC()
{
    //
}

void RTPC::GetContainer(const uint32_t key)
{
    //
    uintptr_t ptr;
    if (m_Container && m_Container->m_NumContainers) {
        ptr = (0x0 + 3 + (m_Container->m_NumVariants * sizeof(RtpcContainerVariant)) + m_Container->m_DataOffset);
    }
}

void RTPC::GetVariant(const uint32_t key)
{
    //
}
}; // namespace ava::RuntimePropertyContainer

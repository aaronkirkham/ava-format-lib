#include "../include/runtime_property_container.h"

#include "../include/util/byte_array_buffer.h"
#include "../include/util/hashlittle.h"

#include <queue>

namespace ava::RuntimePropertyContainer
{
RuntimeContainer::RuntimeContainer(const std::vector<uint8_t>& buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("RTPC input buffer can't be empty!");
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    RtpcHeader header{};
    stream.read((char*)&header, sizeof(RtpcHeader));
    if (header.m_Magic != RTPC_MAGIC) {
        throw std::runtime_error("Invalid RTPC header magic!");
    }

    //
    m_Container = (RtpcContainer*)(buffer.data() + sizeof(RtpcHeader));
}

RuntimeContainer::~RuntimeContainer()
{
    //
}

void RuntimeContainer::GetContainer(const uint32_t key)
{
    //
    uintptr_t ptr;
    if (m_Container && m_Container->m_NumContainers) {
        ptr = (0x0 + 3 + (m_Container->m_NumVariants * sizeof(RtpcContainerVariant)) + m_Container->m_DataOffset);
    }
}

void RuntimeContainer::GetVariant(const uint32_t key)
{
    //
}
}; // namespace ava::RuntimePropertyContainer

#include "../include/runtime_property_container.h"

#include "../include/util/byte_array_buffer.h"
#include "../include/util/hashlittle.h"

#include <queue>

namespace ava::RuntimePropertyContainer
{
void Parse(const std::vector<uint8_t>& buffer)
{
    byte_array_buffer buf(buffer.data(), buffer.size());
    std::istream      stream(&buf);

    // read header
    RtpcFileHeader header;
    stream.read((char*)&header, sizeof(RtpcFileHeader));
    if (header.m_Magic != RTPC_MAGIC) {
        throw std::runtime_error("Invalid RTPC header magic!");
    }

    // read the root node
    RtpcFileNode root_node;
    stream.read((char*)&root_node, sizeof(RtpcFileNode));

    std::queue<RtpcFileNode>     instance_queue;
    std::queue<RtpcFileProperty> property_queue;

    instance_queue.push(root_node);

    // read all instance properties
    while (!instance_queue.empty()) {
        const RtpcFileNode& node = instance_queue.front();
        stream.seekg(node.m_DataOffset);

        // read all node properties
        for (uint16_t i = 0; i < node.m_PropertyCount; ++i) {
            RtpcFileProperty prop;
            stream.read((char*)&prop, sizeof(RtpcFileProperty));

            property_queue.push(std::move(prop));
        }

        // 4 byte boundary alignment
        // @TODO

        // read all node instances
        for (uint16_t i = 0; i < node.m_InstanceCount; ++i) {
            RtpcFileNode node;
            stream.read((char*)&node, sizeof(RtpcFileNode));

            instance_queue.push(std::move(node));
        }

        instance_queue.pop();
    }
}
}; // namespace ava::RuntimePropertyContainer

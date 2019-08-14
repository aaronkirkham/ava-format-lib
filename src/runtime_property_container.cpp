#include "../include/runtime_property_container.h"

#include "../include/util/byte_array_buffer.h"
#include "../include/util/hashlittle.h"

#include <queue>

namespace ava::RuntimePropertyContainer
{
void Parse(const std::vector<uint8_t>& buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("RTPC input buffer can't be empty!");
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    RtpcHeader header;
    stream.read((char*)&header, sizeof(RtpcHeader));
    if (header.m_Magic != RTPC_MAGIC) {
        throw std::runtime_error("Invalid RTPC header magic!");
    }

    // read the root node
    RtpcNode root_node;
    stream.read((char*)&root_node, sizeof(RtpcNode));

    std::queue<RtpcNode>     instance_queue;
    std::queue<RtpcProperty> property_queue;

    instance_queue.push(root_node);

    // read all instance properties
    while (!instance_queue.empty()) {
        const RtpcNode& node = instance_queue.front();
        stream.seekg(node.m_DataOffset);

        // read all node properties
        for (uint16_t i = 0; i < node.m_PropertyCount; ++i) {
            RtpcProperty prop;
            stream.read((char*)&prop, sizeof(RtpcProperty));

            property_queue.push(std::move(prop));
        }

        // 4 byte boundary alignment
        // @TODO

        // read all node instances
        for (uint16_t i = 0; i < node.m_InstanceCount; ++i) {
            RtpcNode node;
            stream.read((char*)&node, sizeof(RtpcNode));

            instance_queue.push(std::move(node));
        }

        instance_queue.pop();
    }
}
}; // namespace ava::RuntimePropertyContainer

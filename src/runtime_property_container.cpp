#include <runtime_property_container.h>

#include <types.h>
#include <util/byte_array_buffer.h>
#include <util/byte_vector_stream.h>
#include <util/hashlittle.h>
#include <util/math.h>

#include <algorithm>
#include <array>
#include <unordered_map>

namespace ava::RuntimePropertyContainer
{
static Container invalid_container = Container::invalid();
static Variant   invalid_variant   = Variant::invalid();

Container read_container(std::istream& stream)
{
    // read the container
    RtpcContainer container;
    stream.read((char*)&container, sizeof(RtpcContainer));

    Container result(container.m_Key);

    // read all container variants
    for (uint16_t i = 0; i < container.m_NumVariants; ++i) {
        stream.seekg(container.m_DataOffset + (i * sizeof(RtpcContainerVariant)));

        RtpcContainerVariant variant;
        stream.read((char*)&variant, sizeof(RtpcContainerVariant));

        Variant variant_wrap(variant.m_Key, variant.m_Type);

        // NOTE: 4 byte primitive type data will be store in the m_DataOffset.
        if (variant.m_Type != T_VARIANT_INTEGER && variant.m_Type != T_VARIANT_FLOAT) {
            stream.seekg(variant.m_DataOffset);
        }

        // read variant data
        switch (variant.m_Type) {
            // inlined values
            case T_VARIANT_INTEGER: variant_wrap.m_Value = *(int32_t*)&variant.m_DataOffset; break;
            case T_VARIANT_FLOAT: variant_wrap.m_Value = *(float*)&variant.m_DataOffset; break;

            case T_VARIANT_STRING: {
                std::string str_value;
                std::getline(stream, str_value, '\0');
                variant_wrap.m_Value = std::move(str_value);
                break;
            }

            case T_VARIANT_VEC2: {
                std::array<float, 2> value;
                stream.read((char*)&value, sizeof(value));
                variant_wrap.m_Value = std::move(value);
                break;
            }

            case T_VARIANT_VEC3: {
                std::array<float, 3> value;
                stream.read((char*)&value, sizeof(value));
                variant_wrap.m_Value = std::move(value);
                break;
            }

            case T_VARIANT_VEC4: {
                std::array<float, 4> value;
                stream.read((char*)&value, sizeof(value));
                variant_wrap.m_Value = std::move(value);
                break;
            }

            case T_VARIANT_MAT4x4: {
                std::array<float, 16> value;
                stream.read((char*)&value, sizeof(value));
                variant_wrap.m_Value = std::move(value);
                break;
            }

            case T_VARIANT_VEC_INTS: {
                int32_t count;
                stream.read((char*)&count, sizeof(int32_t));

                std::vector<int32_t> int_values(count);
                stream.read((char*)int_values.data(), (count * sizeof(int32_t)));
                variant_wrap.m_Value = std::move(int_values);
                break;
            }

            case T_VARIANT_VEC_FLOATS: {
                int32_t count;
                stream.read((char*)&count, sizeof(int32_t));

                std::vector<float> float_values(count);
                stream.read((char*)float_values.data(), (count * sizeof(float)));
                variant_wrap.m_Value = std::move(float_values);
                break;
            }

            case T_VARIANT_VEC_BYTES: {
                int32_t count;
                stream.read((char*)&count, sizeof(int32_t));

                std::vector<uint8_t> byte_values(count);
                stream.read((char*)byte_values.data(), (count * sizeof(uint8_t)));
                variant_wrap.m_Value = std::move(byte_values);
                break;
            }

            case T_VARIANT_OBJECTID: {
                SObjectID value;
                stream.read((char*)&value, sizeof(SObjectID));
                variant_wrap.m_Value = value;
                break;
            }

            case T_VARIANT_VEC_EVENTS: {
                int32_t count;
                stream.read((char*)&count, sizeof(int32_t));

                std::vector<SObjectID> value(count);
                stream.read((char*)value.data(), (count * sizeof(SObjectID)));
                variant_wrap.m_Value = std::move(value);
                break;
            }
        }

        result.m_Variants.emplace_back(std::move(variant_wrap));
    }

    // read all child containers
    for (uint16_t i = 0; i < container.m_NumContainers; ++i) {
        // seek to the containers data position (4 byte aligned)
        auto pos = math::align(container.m_DataOffset + (container.m_NumVariants * sizeof(RtpcContainerVariant)))
                   + (i * sizeof(RtpcContainer));
        stream.seekg(pos);

        auto child = read_container(stream);
        result.m_Containers.emplace_back(std::move(child));
    }

    return result;
}

Result Parse(const std::vector<uint8_t>& buffer, Container* out_root_container)
{
    if (buffer.empty() || !out_root_container) {
        return E_INVALID_ARGUMENT;
    }

    byte_array_buffer buf(buffer);
    std::istream      stream(&buf);

    // read header
    RtpcHeader header{};
    stream.read((char*)&header, sizeof(RtpcHeader));
    if (header.m_Magic != RTPC_MAGIC) {
        return E_RTPC_INVALID_MAGIC;
    }

    // read the root container
    *out_root_container = read_container(stream);
    return E_OK;
}

static RtpcContainer to_native_container(const Container& container, uint32_t data_offset = 0xFFffFFff)
{
    RtpcContainer native_container{};
    native_container.m_Key           = container.m_NameHash;
    native_container.m_DataOffset    = data_offset;
    native_container.m_NumVariants   = (uint16_t)container.m_Variants.size();
    native_container.m_NumContainers = (uint16_t)container.m_Containers.size();
    return native_container;
}

void write_variants_data_buffer(std::vector<Variant>& variants, uint32_t data_offset,
                                std::unordered_map<std::string, uint32_t>& strings, std::vector<uint8_t>* out_buffer,
                                std::vector<RtpcContainerVariant>* out_native_variants)
{
    utils::ByteVectorStream stream(out_buffer);
    out_native_variants->reserve(variants.size());

    for (auto& variant : variants) {
        auto       pos  = (data_offset + (uint32_t)stream.tellp());
        const auto type = variant.m_Type;

        // padding
        if (type != T_VARIANT_INTEGER && type != T_VARIANT_FLOAT && type != T_VARIANT_STRING) {
            uint32_t alignment = 4;

            if (type == T_VARIANT_VEC4 || type == T_VARIANT_MAT4x4) {
                alignment = 16;
            }

            // write padding bytes
            const auto padding = math::align_distance(pos, alignment);
            stream.write((char*)&RTPC_PADDING_BYTE, 1, padding);
            pos += padding;
        }

        RtpcContainerVariant native_variant{};
        native_variant.m_Key        = variant.m_NameHash;
        native_variant.m_DataOffset = pos;
        native_variant.m_Type       = type;

        switch (type) {
            case T_VARIANT_INTEGER: {
                const auto value            = variant.as<int32_t>();
                native_variant.m_DataOffset = (uint32_t)value;
                break;
            }

            case T_VARIANT_FLOAT: {
                const auto value            = variant.as<float>();
                native_variant.m_DataOffset = *(uint32_t*)&value;
                break;
            }

            case T_VARIANT_STRING: {
                const auto& value = variant.as<std::string>();

                auto it = strings.find(value);
                if (it == strings.end()) {
                    // cache the string location and write to the stream
                    strings[value] = native_variant.m_DataOffset;

                    stream.write(value.c_str(), value.length());
                    stream.write('\0');
                } else {
                    // point the variant string data to the previously cached location
                    native_variant.m_DataOffset = (*it).second;
                }

                break;
            }

            case T_VARIANT_VEC2: {
                const auto& value = variant.as<std::array<float, 2>>();
                stream.write(value);
                break;
            }

            case T_VARIANT_VEC3: {
                const auto& value = variant.as<std::array<float, 3>>();
                stream.write(value);
                break;
            }

            case T_VARIANT_VEC4: {
                const auto& value = variant.as<std::array<float, 4>>();
                stream.write(value);
                break;
            }

            case T_VARIANT_MAT4x4: {
                const auto& value = variant.as<std::array<float, 16>>();
                stream.write(value);
                break;
            }

            case T_VARIANT_VEC_INTS: {
                const auto& value = variant.as<std::vector<int32_t>>();
                stream.write((uint32_t)value.size());
                stream.write(value.data(), (value.size() * sizeof(int32_t)));
                break;
            }

            case T_VARIANT_VEC_FLOATS: {
                const auto& value = variant.as<std::vector<float>>();
                stream.write((uint32_t)value.size());
                stream.write(value.data(), (value.size() * sizeof(float)));
                break;
            }

            case T_VARIANT_VEC_BYTES: {
                const auto& value = variant.as<std::vector<uint8_t>>();
                stream.write((uint32_t)value.size());
                stream.write(value.data(), (value.size() * sizeof(uint8_t)));
                break;
            }

            case T_VARIANT_OBJECTID: {
                const auto& value = variant.as<SObjectID>();
                stream.write(value.to_binary_uint64());
                break;
            }

            case T_VARIANT_VEC_EVENTS: {
                const auto& value = variant.as<std::vector<SObjectID>>();
                stream.write((uint32_t)value.size());
                stream.write(value.data(), (value.size() * sizeof(SObjectID)));
                break;
            }
        }

        out_native_variants->emplace_back(std::move(native_variant));
    }
}

uint32_t write_container(utils::ByteVectorStream& stream, const Container& container, uint32_t version,
                         uint32_t data_offset, std::unordered_map<std::string, uint32_t>& strings)
{
    auto variants   = container.m_Variants;
    auto containers = container.m_Containers;

    const auto native_variants_size   = (uint32_t)(variants.size() * sizeof(RtpcContainerVariant));
    const auto native_containers_size = (uint32_t)(containers.size() * sizeof(RtpcContainer));

    const auto variant_data_offset = math::align((uint32_t)data_offset + native_variants_size) + native_containers_size;

    // write native variant data buffer
    std::vector<uint8_t>              variants_data_buffer;
    std::vector<RtpcContainerVariant> native_variants;
    write_variants_data_buffer(variants, variant_data_offset, strings, &variants_data_buffer, &native_variants);
    stream.setp(variant_data_offset);
    stream.write(variants_data_buffer);

    // write native variants
    {
        const auto native_variants_pos = math::align(data_offset);
        stream.setp(native_variants_pos);
        stream.write(native_variants.data(), native_variants_size);

        // write padding bytes after the native variants
        stream.write((char*)&RTPC_PADDING_BYTE, 1, math::align_distance(native_variants_pos + native_variants_size));
    }

    // write padding bytes at the end of this container and the start of the next
    const auto next_offset_unaligned = (uint32_t)(variant_data_offset + variants_data_buffer.size());
    {
        stream.setp(next_offset_unaligned);
        stream.write((char*)&RTPC_PADDING_BYTE, 1, math::align_distance(next_offset_unaligned));
    }

    const auto native_container_offset = math::align(data_offset + native_variants_size);
    auto       next_data_offset        = math::align(next_offset_unaligned);

    // write child containers
    for (size_t i = 0; i < containers.size(); ++i) {
        const auto& child = containers[i];

        // write native container at it's offset
        stream.setp(native_container_offset + (i * sizeof(RtpcContainer)));
        stream.write(to_native_container(child, next_data_offset));

        // write container data
        next_data_offset = write_container(stream, child, version, next_data_offset, strings);
    }

    return next_data_offset;
}

Result Write(const Container& root_container, uint32_t version, std::vector<uint8_t>* out_buffer)
{
    if (!out_buffer) {
        return E_INVALID_ARGUMENT;
    }

    utils::ByteVectorStream stream(out_buffer);

    // write header
    RtpcHeader header;
    header.m_Version = version;
    stream.write(header);

    const auto data_offset = (uint32_t)(sizeof(RtpcHeader) + sizeof(RtpcContainer));

    // write the native container
    auto native_container = to_native_container(root_container, data_offset);
    stream.write(native_container);

    std::unordered_map<std::string, uint32_t> _tmp_strings_cache;
    write_container(stream, root_container, version, data_offset, _tmp_strings_cache);
    return E_OK;
}

const Container& Container::GetContainer(uint32_t namehash, bool look_in_child_containers)
{
    for (auto& container : m_Containers) {
        if (container.m_NameHash == namehash) {
            return container;
        }

        if (look_in_child_containers) {
            if (auto& child = container.GetContainer(namehash); child.valid()) {
                return child;
            }
        }
    }

    return invalid_container;
}

Variant& Container::GetVariant(uint32_t namehash, bool look_in_child_containers)
{
    for (auto& variant : m_Variants) {
        if (variant.m_NameHash == namehash) {
            return variant;
        }
    }

    if (look_in_child_containers) {
        for (auto& container : m_Containers) {
            if (auto& child = container.GetVariant(namehash); child.valid()) {
                return child;
            }
        }
    }

    return invalid_variant;
}
}; // namespace ava::RuntimePropertyContainer

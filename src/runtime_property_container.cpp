#include <runtime_property_container.h>

#include <types.h>
#include <util/byte_array_buffer.h>
#include <util/hashlittle.h>
#include <util/math.h>

#include <algorithm>
#include <array>

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

Container& Container::GetContainer(uint32_t namehash, bool look_in_child_containers)
{
    auto iter = std::find_if(m_Containers.begin(), m_Containers.end(),
                             [namehash](const Container& container) { return container.m_NameHash == namehash; });
    if (iter != m_Containers.end()) {
        return (*iter);
    }

    return invalid_container;
}

Variant& Container::GetVariant(uint32_t namehash, bool look_in_child_containers)
{
    auto iter = std::find_if(m_Variants.begin(), m_Variants.end(),
                             [namehash](const Variant& variant) { return variant.m_NameHash == namehash; });

    if (iter != m_Variants.end()) {
        return (*iter);
    }

    if (look_in_child_containers) {
        // TODO
    }

    return invalid_variant;
}
}; // namespace ava::RuntimePropertyContainer

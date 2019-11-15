#include "../include/avalanche_data_format.h"

#include "../include/util/hashlittle.h"
#include "../include/util/math.h"

#include <algorithm>

namespace ava::AvalancheDataFormat
{
void ParseHeader(const std::vector<uint8_t>& buffer, AdfHeader* out_header, const char** out_description)
{
    if (buffer.empty() || buffer.size() < 24) {
        throw std::invalid_argument("ADF input buffer isn't big enough!");
    }

    const AdfHeader* header = (AdfHeader*)buffer.data();

    if (header->m_Magic != ADF_MAGIC) {
        throw std::runtime_error("Invalid ADF header magic!");
    }

    *out_header = *header;

    // write description if wanted
    if (out_description && header->m_Description) {
        *out_description = (const char*)&header->m_Description;
    }
}

ADF::ADF(const std::vector<uint8_t>& buffer)
    : m_Header((AdfHeader*)buffer.data())
    , m_Buffer(buffer)
{
    if (buffer.empty()) {
        throw std::invalid_argument("ADF input buffer can't be empty!");
    }

    if (m_Header->m_Magic != ADF_MAGIC) {
        throw std::runtime_error("Invalid ADF header magic!");
    }

    // add built in primitive types
    AddBuiltInType(ADF_TYPE_SCALAR, ADF_SCALARTYPE_UNSIGNED, sizeof(uint8_t), "uint8");
    AddBuiltInType(ADF_TYPE_SCALAR, ADF_SCALARTYPE_SIGNED, sizeof(int8_t), "int8");
    AddBuiltInType(ADF_TYPE_SCALAR, ADF_SCALARTYPE_UNSIGNED, sizeof(uint16_t), "uint16");
    AddBuiltInType(ADF_TYPE_SCALAR, ADF_SCALARTYPE_SIGNED, sizeof(int16_t), "int16");
    AddBuiltInType(ADF_TYPE_SCALAR, ADF_SCALARTYPE_UNSIGNED, sizeof(uint32_t), "uint32");
    AddBuiltInType(ADF_TYPE_SCALAR, ADF_SCALARTYPE_SIGNED, sizeof(int32_t), "int32");
    AddBuiltInType(ADF_TYPE_SCALAR, ADF_SCALARTYPE_UNSIGNED, sizeof(uint64_t), "uint64");
    AddBuiltInType(ADF_TYPE_SCALAR, ADF_SCALARTYPE_SIGNED, sizeof(int64_t), "int64");
    AddBuiltInType(ADF_TYPE_SCALAR, ADF_SCALARTYPE_FLOAT, sizeof(float), "float");
    AddBuiltInType(ADF_TYPE_SCALAR, ADF_SCALARTYPE_FLOAT, sizeof(double), "double");
    AddBuiltInType(ADF_TYPE_STRING, ADF_SCALARTYPE_SIGNED, 8, "String", 0);
    AddBuiltInType(ADF_TYPE_DEFERRED, ADF_SCALARTYPE_SIGNED, 16, "void", 0);

    // add internal types from this buffer
    AddTypes(buffer);
}

ADF::~ADF()
{
    for (auto& type : m_Types) {
        std::free(type);
    }
}

void ADF::AddBuiltInType(EAdfType type, EAdfScalarType scalar_type, uint32_t size, const char* name, uint16_t flags)
{
    char type_name[64];
    snprintf(type_name, sizeof(type_name), "%s%u%u%u", name, (uint32_t)type, size, size);

    uint32_t type_hash = hashlittle(type_name);
    uint32_t alignment = size;

    if (type == ADF_TYPE_DEFERRED) {
        type_hash = 0xDEFE88ED;
        alignment = 8;
    }

    // push the type name into the strings vector if it doesn't exist
    if (std::find(m_Strings.begin(), m_Strings.end(), name) == m_Strings.end()) {
        m_Strings.push_back(name);
    }

    // create type definition
    AdfType* def       = new AdfType;
    def->m_Type        = type;
    def->m_Size        = size;
    def->m_Align       = alignment;
    def->m_TypeHash    = type_hash;
    def->m_Name        = GetStringIndex(name);
    def->m_Flags       = flags;
    def->m_ScalarType  = scalar_type;
    def->m_SubTypeHash = 0;
    def->m_ArraySize   = 0;
    def->m_MemberCount = 0;
    m_Types.push_back(def);
}

void ADF::LoadInlineOffsets(const AdfType* type, char* payload, const uint32_t offset)
{
    static auto DoesTypeNeedLoading = [](const EAdfType type) {
        return (type == ADF_TYPE_STRUCT || type == ADF_TYPE_POINTER || type == ADF_TYPE_ARRAY
                || type == ADF_TYPE_DEFERRED || type == ADF_TYPE_STRING);
    };

    switch (type->m_Type) {
        case ADF_TYPE_STRUCT: {
            uint32_t member_offset = 0;
            for (uint32_t i = 0; i < type->m_MemberCount; ++i) {
                const AdfMember& member      = type->m_Members[i];
                const AdfType*   member_type = FindType(member.m_TypeHash);

                const uint32_t payload_offset = (offset + member_offset);
                const uint32_t alignment      = ava::math::align_distance(payload_offset, member_type->m_Align);

                if (DoesTypeNeedLoading(member_type->m_Type)) {
                    LoadInlineOffsets(member_type, payload, (payload_offset + alignment));
                }

                member_offset += (member_type->m_Size + alignment);
            }

            break;
        }

        case ADF_TYPE_POINTER:
        case ADF_TYPE_DEFERRED: {
            const uint32_t real_offset = *(uint32_t*)&payload[offset];
            if (real_offset) {
                *(uint64_t*)&payload[offset] = (uint64_t)((char*)payload + real_offset);

                const uint32_t type_hash =
                    (type->m_Type == ADF_TYPE_POINTER ? type->m_SubTypeHash : *(uint32_t*)&payload[offset + 8]);
                const AdfType* ptr_type = FindType(type_hash);
                if (ptr_type) {
                    LoadInlineOffsets(ptr_type, payload, real_offset);
                }
            }

            break;
        }

        case ADF_TYPE_ARRAY: {
            const uint32_t real_offset = *(uint32_t*)&payload[offset];
            if (real_offset) {
                *(uint64_t*)&payload[offset] = (uint64_t)((char*)payload + real_offset);

                const AdfType* subtype = FindType(type->m_SubTypeHash);
                if (subtype && DoesTypeNeedLoading(subtype->m_Type)) {
                    const uint32_t count = *(uint32_t*)&payload[offset + 8];
                    for (uint32_t i = 0; i < count; ++i) {
                        LoadInlineOffsets(subtype, payload, (real_offset + (subtype->m_Size * i)));
                    }
                }
            }

            break;
        }

        case ADF_TYPE_STRING: {
            const uint32_t real_offset = *(uint32_t*)&payload[offset];
            if (real_offset) {
                *(uint64_t*)&payload[offset] = (uint64_t)((char*)payload + real_offset);
            }

            break;
        }
    }
}

void ADF::AddTypes(const std::vector<uint8_t>& buffer)
{
    AdfHeader header;
    ParseHeader(buffer, &header);

    // read string hashes
    {
        uint64_t    offset = 0;
        const char* hashes = (const char*)&buffer[header.m_FirstStringHashOffset];
        for (uint32_t i = 0; i < header.m_StringHashCount; ++i) {
            const char*    str    = &hashes[offset];
            const auto     length = (strlen(str) + 1);
            const uint64_t hash   = *(uint64_t*)&hashes[offset + length];

            // @NOTE: hashes are stored as uint64, but only 32bits are used.

            m_StringHashes[(uint32_t)hash] = str;
            offset += (length + sizeof(hash));
        }
    }

    // preallocate the strings
    m_Strings.reserve(m_Strings.size() + header.m_StringCount);

    // read strings
    uint32_t             offset  = 0;
    const char*          strings = (const char*)&buffer[header.m_FirstStringDataOffset];
    std::vector<uint8_t> lengths(strings, &strings[header.m_StringCount]);
    for (uint32_t i = 0; i < header.m_StringCount; ++i) {
        const auto length = (lengths[i] + 1);
        auto       str    = std::unique_ptr<char[]>(new char[length]);
        strcpy_s(str.get(), length, &strings[header.m_StringCount + offset]);

        m_Strings.push_back(str.get());

        offset += length;
    }

    // read types
    const char* types_data = (const char*)&buffer[header.m_FirstTypeOffset];
    for (uint32_t i = 0; i < header.m_TypeCount; ++i) {
        const AdfType* current = (AdfType*)types_data;
        const size_t   size    = current->DataSize();

        // do we already have this type?
        if (FindType(current->m_TypeHash)) {
            types_data += size;
            continue;
        }

        // copy the type and its members
        auto type = (AdfType*)std::malloc(size);
        std::memcpy(type, current, size);

        // reindex the type name
        type->m_Name = GetStringIndex(GetString(type->m_Name, &header, buffer));

        // reindex all member type names
        if (type->m_Type == ADF_TYPE_STRUCT || type->m_Type == ADF_TYPE_ENUM) {
            const bool is_enum = (type->m_Type == ADF_TYPE_ENUM);
            for (uint32_t x = 0; x < type->m_MemberCount; ++x) {
                const void*    member            = (is_enum ? (void*)&type->Enum(x) : (void*)&type->m_Members[x]);
                const uint64_t member_name_index = *(uint64_t*)member;
                *(uint64_t*)member               = GetStringIndex(GetString(member_name_index, &header, buffer));
            }
        }

        m_Types.push_back(type);
        types_data += size;
    }
}

AdfType* ADF::FindType(const uint32_t type_hash)
{
    const auto it = std::find_if(m_Types.begin(), m_Types.end(),
                                 [type_hash](const AdfType* type) { return type->m_TypeHash == type_hash; });
    return (it != m_Types.end() ? *it : nullptr);
}

void ADF::GetInstance(uint32_t index, SInstanceInfo* out_instance_info)
{
    if (!out_instance_info) {
        throw std::invalid_argument("ADF GetInstance output instance can't be nullptr!");
    }

    const AdfInstance* instance =
        (AdfInstance*)&m_Buffer[m_Header->m_FirstInstanceOffset + (sizeof(AdfInstance) * index)];
    if (!instance) {
        throw std::runtime_error("ADF instance was nullptr! (invalid instance index?)");
    }

    out_instance_info->m_NameHash = instance->m_NameHash;
    out_instance_info->m_TypeHash = instance->m_TypeHash;
    out_instance_info->m_Name     = GetString(instance->m_Name, m_Header, m_Buffer);

    const AdfType* type = FindType(instance->m_TypeHash);
    if (type) {
        out_instance_info->m_Instance     = &m_Buffer[instance->m_PayloadOffset];
        out_instance_info->m_InstanceSize = instance->m_PayloadSize;
        return;
    }

    out_instance_info->m_Instance     = nullptr;
    out_instance_info->m_InstanceSize = 0;
}

void ADF::ReadInstance(uint32_t name_hash, uint32_t type_hash, void** out_instance)
{
    // find the instance
    AdfInstance* current_instance = nullptr;
    auto         instance_buffer  = &m_Buffer[m_Header->m_FirstInstanceOffset];
    for (uint32_t i = 0; i < m_Header->m_InstanceCount; ++i) {
        current_instance = (AdfInstance*)instance_buffer;
        if (current_instance->m_NameHash == name_hash && current_instance->m_TypeHash == type_hash) {
            break;
        }

        instance_buffer += sizeof(AdfInstance);
    }

    if (!current_instance) {
        throw std::runtime_error("Can't find instance!");
    }

    const AdfType* type    = FindType(current_instance->m_TypeHash);
    auto           payload = &m_Buffer[current_instance->m_PayloadOffset];

    // alloc the memory for the result
    auto mem = std::malloc(current_instance->m_PayloadSize);
    std::memcpy(mem, payload, current_instance->m_PayloadSize);

    bool has_32bit_inline_arrays = ~LOBYTE(m_Header->m_Flags) & E_ADF_HEADER_FLAG_RELATIVE_OFFSETS_EXISTS;
    if (has_32bit_inline_arrays) {
        LoadInlineOffsets(type, (char*)mem);
    } else {
        // adjust the relative offsets
        uint64_t current_offset = 0;
        uint64_t v72            = 0;
        for (auto size = *(uint32_t*)&payload[current_instance->m_PayloadSize]; size;
             *(uint64_t*)((uint32_t)(current_offset - 4) + (uint64_t)mem) = (uint64_t)mem + v72) {

            current_offset = (current_offset + size);
            size           = *(uint32_t*)(current_offset + (uint64_t)mem);
            v72            = *(uint32_t*)((uint32_t)(current_offset - 4) + (uint64_t)mem);

            if (v72 == 1) {
                v72 = 0;
            }
        }
    }

    *out_instance = mem;
}

void ADF::ReadInstance(const SInstanceInfo& instance_info, void** out_instance)
{
    return ReadInstance(instance_info.m_NameHash, instance_info.m_TypeHash, out_instance);
}
}; // namespace ava::AvalancheDataFormat

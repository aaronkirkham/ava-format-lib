#include "../include/avalanche_data_format.h"

#include "../include/util/hashlittle.h"

#include <algorithm>

namespace ava::AvalancheDataFormat
{
/**
 * Parse ADF buffer header
 *
 * @param buffer Buffer containing the ADF header data
 * @param out_header Pointer to AdfHeader where the data will be written
 * @param out_description Pointer to a string where the header description will be written (if available)
 */
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
    AddBuiltInType(EAdfType::SCALAR, ScalarType::UNSIGNED, sizeof(uint8_t), "uint8");
    AddBuiltInType(EAdfType::SCALAR, ScalarType::SIGNED, sizeof(int8_t), "int8");
    AddBuiltInType(EAdfType::SCALAR, ScalarType::UNSIGNED, sizeof(uint16_t), "uint16");
    AddBuiltInType(EAdfType::SCALAR, ScalarType::SIGNED, sizeof(int16_t), "int16");
    AddBuiltInType(EAdfType::SCALAR, ScalarType::UNSIGNED, sizeof(uint32_t), "uint32");
    AddBuiltInType(EAdfType::SCALAR, ScalarType::SIGNED, sizeof(int32_t), "int32");
    AddBuiltInType(EAdfType::SCALAR, ScalarType::UNSIGNED, sizeof(uint64_t), "uint64");
    AddBuiltInType(EAdfType::SCALAR, ScalarType::SIGNED, sizeof(int64_t), "int64");
    AddBuiltInType(EAdfType::SCALAR, ScalarType::FLOAT, sizeof(float), "float");
    AddBuiltInType(EAdfType::SCALAR, ScalarType::FLOAT, sizeof(double), "double");
    AddBuiltInType(EAdfType::STRING, ScalarType::SIGNED, 8, "String", 0);
    AddBuiltInType(EAdfType::DEFERRED, ScalarType::SIGNED, 16, "void", 0);

    // add internal types from this buffer
    AddTypes(buffer);
}

ADF::~ADF()
{
    for (auto& type : m_Types) {
        std::free(type);
    }
}

void ADF::AddBuiltInType(EAdfType type, ScalarType scalar_type, uint32_t size, const char* name, uint16_t flags)
{
    char type_name[64];
    snprintf(type_name, sizeof(type_name), "%s%u%u%u", name, (uint32_t)type, size, size);

    uint32_t type_hash = hashlittle(type_name);
    uint32_t alignment = size;

    if (type == EAdfType::DEFERRED) {
        type_hash = 0xDEFE88ED;
        alignment = 8;
    }

    // push the type name into the strings vector if it doesn't exist
    if (std::find(m_Strings.begin(), m_Strings.end(), name) == m_Strings.end()) {
        m_Strings.push_back(name);
    }

    // create type definition
    AdfType* def               = new AdfType;
    def->m_Type                = type;
    def->m_Size                = size;
    def->m_Align               = alignment;
    def->m_TypeHash            = type_hash;
    def->m_Name                = GetStringIndex(name);
    def->m_Flags               = flags;
    def->m_ScalarType          = scalar_type;
    def->m_SubTypeHash         = 0;
    def->m_ArraySizeOrBitCount = 0;
    def->m_MemberCount         = 0;
    m_Types.push_back(def);
}

AdfType* ADF::FindType(const uint32_t type_hash)
{
    const auto it = std::find_if(m_Types.begin(), m_Types.end(),
                                 [type_hash](const AdfType* type) { return type->m_TypeHash == type_hash; });
    return (it != m_Types.end() ? *it : nullptr);
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
        const bool is_enum = (type->m_Type == EAdfType::ENUM);
        for (uint32_t x = 0; x < type->m_MemberCount; ++x) {
            const void*    member            = (is_enum ? (void*)&type->Enum(x) : (void*)&type->m_Members[x]);
            const uint64_t member_name_index = *(uint64_t*)member;
            *(uint64_t*)member               = GetStringIndex(GetString(member_name_index, &header, buffer));
        }

        m_Types.push_back(type);
        types_data += size;
    }
}

/**
 * Get an instance from an ADF buffer
 *
 * @param index Index of the instance to read from the ADF buffer
 * @param out_instance_info Pointer to SInstanceInfo where the instance data will be written
 */
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

/**
 * Read an instance from an ADF buffer
 *
 * @param name_hash Name hash of the instance to read from the ADF buffer
 * @param type_hash Type hash of the instance to read from the ADF buffer
 * @param out_instance Pointer to an instance where the data will be written
 */
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

    bool has_32bit_inline_arrays = ~LOBYTE(m_Header->m_Flags) & EHeaderFlags::RELATIVE_OFFSETS_EXISTS;
    if (has_32bit_inline_arrays) {
        // LoadInlineOffsets(type, (char*)mem);
        throw std::runtime_error("has_32bit_inline_arrays");
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

/**
 * Read an instance from an ADF buffer
 *
 * @param instance_info Instance info returned from GetInstance
 * @param out_instance Pointer to an instance where the data will be written
 */
void ADF::ReadInstance(const SInstanceInfo& instance_info, void** out_instance)
{
    return ReadInstance(instance_info.m_NameHash, instance_info.m_TypeHash, out_instance);
}
}; // namespace ava::AvalancheDataFormat

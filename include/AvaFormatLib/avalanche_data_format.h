#pragma once

#include "error.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ava
{
static constexpr uint32_t ADF_MAGIC = 0x41444620; // ADF

enum EAdfType : uint32_t {
    ADF_TYPE_SCALAR       = 0x0,
    ADF_TYPE_STRUCT       = 0x1,
    ADF_TYPE_POINTER      = 0x2,
    ADF_TYPE_ARRAY        = 0x3,
    ADF_TYPE_INLINE_ARRAY = 0x4,
    ADF_TYPE_STRING       = 0x5,
    ADF_TYPE_RECURSIVE    = 0x6,
    ADF_TYPE_BITFIELD     = 0x7,
    ADF_TYPE_ENUM         = 0x8,
    ADF_TYPE_STRING_HASH  = 0x9,
    ADF_TYPE_DEFERRED     = 0xA,
};

enum EAdfScalarType : uint16_t {
    ADF_SCALARTYPE_SIGNED   = 0x0,
    ADF_SCALARTYPE_UNSIGNED = 0x1,
    ADF_SCALARTYPE_FLOAT    = 0x2,
};

enum EAdfHeaderFlags {
    E_ADF_HEADER_FLAG_RELATIVE_OFFSETS_EXISTS             = 0x1,
    E_ADF_HEADER_FLAG_CONTAINS_INLINE_ARRAY_WITH_POINTERS = 0x2,
};

enum EAdfTypeFlags {
    E_ADF_TYPE_FLAGS_DEFAULT         = 0x0,
    E_ADF_TYPE_FLAG_SIMPLE_POD_READ  = 0x1,
    E_ADF_TYPE_FLAG_SIMPLE_POD_WRITE = 0x2,
    E_ADF_TYPE_FLAG_NO_STRING_HASHES = 0x4,
    E_ADF_TYPE_FLAG_IS_FINALIZED     = 0x8000,
};

#pragma pack(push, 8)
template <typename T> struct SAdfArray {
    T*       m_Data;
    uint32_t m_Count;

    const T* data() const { return m_Data[0]; }
    T*       begin() const { return m_Data; }
    T*       end() const { return m_Data ? m_Data + m_Count : nullptr; }

    const T& operator[](uint32_t index) const { return m_Data[index]; }
    T&       operator[](uint32_t index) { return m_Data[index]; }

    const char* c_str() const { return (const char*)&m_Data[0]; }
};

struct SAdfDeferredPtr {
    void*    m_Ptr;
    uint32_t m_Type;
};

static_assert(sizeof(SAdfArray<char>) == 0x10, "SAdfArray alignment is wrong!");
static_assert(sizeof(SAdfDeferredPtr) == 0x10, "SAdfDeferredPtr alignment is wrong!");
#pragma pack(pop)

namespace AvalancheDataFormat
{
#pragma pack(push, 1)
    struct AdfHeader {
        uint32_t m_Magic = ADF_MAGIC;
        uint32_t m_Version;
        uint32_t m_InstanceCount;
        uint32_t m_FirstInstanceOffset;
        uint32_t m_TypeCount;
        uint32_t m_FirstTypeOffset;
        uint32_t m_StringHashCount;
        uint32_t m_FirstStringHashOffset;
        uint32_t m_StringCount;
        uint32_t m_FirstStringDataOffset;
        uint32_t m_FileSize;
        uint32_t m_MetaDataOffset;
        uint32_t m_Flags;
        uint32_t m_IncludedLibraries;
        uint32_t : 32;
        uint32_t : 32;
        const char* m_Description;
    };

    struct AdfInstance {
        uint32_t m_NameHash;
        uint32_t m_TypeHash;
        uint32_t m_PayloadOffset;
        uint32_t m_PayloadSize;
        uint64_t m_Name;
    };

    struct AdfMember {
        uint64_t m_Name;
        uint32_t m_TypeHash;
        uint32_t m_Align;
        uint32_t m_Offset : 24;
        uint32_t m_BitOffset : 8;
        uint32_t m_Flags;
        uint64_t m_DefaultValue;
    };

    struct AdfEnum {
        uint64_t m_Name;
        int32_t  m_Value;
    };

// warning C4200: nonstandard extension used: zero-sized array in struct/union
#pragma warning(push)
#pragma warning(disable : 4200)
    struct AdfType {
        EAdfType       m_Type;
        uint32_t       m_Size;
        uint32_t       m_Align;
        uint32_t       m_TypeHash;
        uint64_t       m_Name;
        uint16_t       m_Flags;
        EAdfScalarType m_ScalarType;
        uint32_t       m_SubTypeHash;
        union {
            uint32_t m_BitCount;  // Only for ADF_TYPE_BITFIELD
            uint32_t m_ArraySize; // Only for ADF_TYPE_INLINE_ARRAY
        };
        union {
            uint32_t m_MemberCount; // Only for ADF_TYPE_STRUCT and ADF_TYPE_ENUM
            uint32_t m_DataAlign;
        };
        AdfMember m_Members[0];

        AdfEnum&       Enum(uint32_t i) { return ((AdfEnum*)(void*)&m_Members[0])[i]; }
        const AdfEnum& Enum(uint32_t i) const { return ((const AdfEnum*)(void*)&m_Members[0])[i]; }

        const size_t DataSize() const
        {
            uint32_t member_count = 0;
            uint32_t member_size  = sizeof(AdfMember);
            if (m_Type == ADF_TYPE_STRUCT || m_Type == ADF_TYPE_ENUM) {
                member_count = m_MemberCount;

                if (m_Type == ADF_TYPE_ENUM) {
                    member_size = sizeof(AdfEnum);
                }
            }

            return (sizeof(AdfType) + (member_count * member_size));
        }
    };
#pragma warning(pop)

    struct SInstanceInfo {
        uint32_t    m_NameHash;
        uint32_t    m_TypeHash;
        const char* m_Name;
        const void* m_Instance;
        uint32_t    m_InstanceSize;
    };
#pragma pack(pop)

    static_assert(sizeof(AdfHeader) == 0x48, "AdfHeader alignment is wrong.");
    static_assert(sizeof(AdfInstance) == 0x18, "AdfInstance alignment is wrong.");
    static_assert(sizeof(AdfMember) == 0x20, "AdfMember alignment is wrong.");
    static_assert(sizeof(AdfEnum) == 0xC, "AdfEnum alignment is wrong.");
    static_assert(sizeof(AdfType) == 0x28, "AdfType alignment is wrong.");
    static_assert(sizeof(SInstanceInfo) == 0x1C, "SInstanceInfo alignment is wrong.");

    /**
     * Parse ADF buffer header
     *
     * @param buffer Buffer containing the ADF header data
     * @param out_header Pointer to AdfHeader where the data will be written
     * @param out_description Pointer to a string where the header description will be written (if available)
     */
    Result ParseHeader(const std::vector<uint8_t>& buffer, AdfHeader* out_header,
                       const char** out_description = nullptr);

    class ADF
    {
      protected:
        std::vector<uint8_t>            m_Buffer;
        AdfHeader*                      m_Header = nullptr;
        std::vector<AdfType*>           m_Types;
        std::vector<AdfType*>           m_InternalTypes;
        std::vector<std::string>        m_Strings;
        std::map<uint32_t, std::string> m_StringHashes;

      private:
        void AddBuiltInType(EAdfType type, EAdfScalarType scalar_type, uint32_t size, const char* name,
                            uint16_t flags = 3);
        void AddBuiltInTypes()
        {
            // primitive types
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
        }
        void LoadInlineOffsets(const AdfType* type, char* payload, const uint32_t offset = 0);

        const char* GetString(uint64_t index, const AdfHeader* header, const std::vector<uint8_t>& buffer)
        {
            const char* data = (const char*)buffer.data();

            const char*    strings = &data[header->m_FirstStringDataOffset + header->m_StringCount];
            const uint8_t* lengths = (const uint8_t*)&data[header->m_FirstStringDataOffset];

            uint64_t offset = 0;
            for (uint64_t i = 0; i < index; ++i) {
                offset += (lengths[i] + 1);
            }

            const auto rel_index =
                std::distance(m_Strings.begin(), std::find(m_Strings.begin(), m_Strings.end(), &strings[offset]));
            return m_Strings[rel_index].c_str();
        }

        uint64_t GetStringIndex(const char* string)
        {
            return std::distance(m_Strings.begin(), std::find(m_Strings.begin(), m_Strings.end(), string));
        }

      public:
        ADF();
        ADF(const std::vector<uint8_t>& buffer);
        virtual ~ADF();

        void AddTypes(const std::vector<uint8_t>& buffer);

        /**
         * Find a type from its hash
         *
         * @param type_hash Type name hash of the type to find
         */
        AdfType* FindType(const uint32_t type_hash);

        /**
         * Get an instance from an ADF buffer
         *
         * @param index Index of the instance to read from the ADF buffer
         * @param out_instance_info Pointer to SInstanceInfo where the instance data will be written
         */
        bool GetInstance(uint32_t index, SInstanceInfo* out_instance_info);

        /**
         * Read an instance from an ADF buffer
         *
         * @param name_hash Name hash of the instance to read from the ADF buffer
         * @param type_hash Type hash of the instance to read from the ADF buffer
         * @param out_instance Pointer to an instance where the data will be written
         */
        bool ReadInstance(uint32_t name_hash, uint32_t type_hash, void** out_instance);

        /**
         * Read an instance from an ADF buffer
         *
         * @param instance_info Instance info returned from GetInstance
         * @param out_instance Pointer to an instance where the data will be written
         */
        bool ReadInstance(const SInstanceInfo& instance_info, void** out_instance);

        /**
         * Read an instance from an ADF buffer
         *
         * @param index Index of the instance to read from the ADF buffer
         * @param out_instance Pointer to an instance where the data will be written
         */
        bool ReadInstance(uint32_t index, void** out_instance);

        /**
         * Name hash lookup
         *
         * @param hash Name hash to lookup
         */
        const char* HashLookup(const uint32_t hash)
        {
            const auto it = m_StringHashes.find(hash);
            if (it == m_StringHashes.end()) {
                return "";
            }

            return it->second.c_str();
        }

        const std::vector<uint8_t>* GetBuffer() { return &m_Buffer; }
        const AdfHeader&            GetHeader() const { return *m_Header; }
        const std::string&          GetString(const uint64_t index) { return m_Strings[index]; }

        const std::vector<AdfType*>& GetTypes(bool only_internal = true) const
        {
            if (only_internal)
                return m_InternalTypes;
            else
                return m_Types;
        }
    };
} // namespace AvalancheDataFormat
} // namespace ava

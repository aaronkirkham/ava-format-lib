#pragma once

#include "avalanche_data_format.h"
#include "error.h"

#include <cstdint>
#include <vector>

namespace ava::StringLookup
{
#pragma pack(push, 8)
struct SHashProperties {
    uint32_t m_Hash;
    uint32_t m_TextOffset;
    uint32_t m_NameOffset;
};

struct SSubtitle {
    uint32_t m_LineHash;
    float    m_Start;
    float    m_Duration;
};

struct SDialogueLine {
    uint32_t             m_Hash;
    uint32_t             m_NameOffset;
    SAdfArray<SSubtitle> m_Subtitles;
    uint32_t             m_FMODEvent;
    uint32_t             m_IncomingCall;
    uint32_t             m_ConversationStarter;
    uint32_t             m_ResponseVocalEvent;
    uint32_t             m_CharacterName;
    uint32_t             m_PackageOffset;
    uint32_t             m_Flags;
};

struct SStringLookup {
    SAdfArray<SHashProperties> m_SortedPairs;
    SAdfArray<SDialogueLine>   m_SortedDialogueLines;
    SAdfArray<char>            m_Text;
};
#pragma pack(pop)

/**
 * Parse a .stringlookup ADF buffer
 *
 * @param buffer Input buffer containing a raw stringlookup ADF file buffer
 * @param out_adf Pointer to an ADF struct where the ADF instance will be stored (You're responsible for calling delete
 * on this pointer the memory once you're done with it)
 * @param out_string_lookup Pointer to a SStringLookup struct where the data will be stored once parsed (You're
 * responsible for calling free() on this pointer once you're done with it)
 */
Result Parse(const std::vector<uint8_t>& buffer, AvalancheDataFormat::ADF** out_adf, SStringLookup** out_string_lookup);
} // namespace ava::StringLookup
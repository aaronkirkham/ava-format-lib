#pragma once

#include "../avalanche_data_format.h"
#include "../error.h"

#include <cstdint>
#include <vector>

namespace ava::legacy::StringLookup
{
enum ELocale : uint32_t {
    LOCALE_ENGLISH   = 0x0,
    LOCALE_FRENCH    = 0x1,
    LOCALE_GERMAN    = 0x2,
    LOCALE_ITALIAN   = 0x3,
    LOCALE_SPANISH   = 0x4,
    LOCALE_RUSSIAN   = 0x5,
    LOCALE_POLISH    = 0x6,
    LOCALE_JAPANESE  = 0x7,
    LOCALE_BRAZILIAN = 0x8,
    LOCALE_MEXICAN   = 0x9,
    LOCALE_ARABIC    = 0xA,
    NOF_LOCALES      = 0xB,
};

#pragma pack(push, 8)
struct SHashPropertiesPair {
    uint32_t m_Hash;
    uint32_t m_TextOffset;
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
    uint32_t             m_CharacterName;
    uint32_t             m_Flags;
};

struct SLanguageStringLookup {
    SAdfArray<SHashPropertiesPair> m_SortedPairs;
    SAdfArray<SDialogueLine>       m_SortedDialogueLines;
};

struct SStringLookup {
    SAdfArray<SLanguageStringLookup> m_Languages;
    SAdfArray<char>                  m_Text;
};
#pragma pack(pop)

/**
 * Parse a legacy .stringlookup ADF buffer
 *
 * @param buffer Input buffer containing a raw stringlookup ADF file buffer
 * @param out_adf Pointer to an ADF struct where the ADF instance will be stored (You're responsible for calling delete
 * on this pointer the memory once you're done with it)
 * @param out_string_lookup Pointer to a SStringLookup struct where the data will be stored once parsed (You're
 * responsible for calling free() on this pointer once you're done with it)
 */
Result Parse(const std::vector<uint8_t>& buffer, AvalancheDataFormat::ADF** out_adf, SStringLookup** out_string_lookup);
} // namespace ava::legacy::StringLookup
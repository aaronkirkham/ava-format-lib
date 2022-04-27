#pragma once

#include <cstdint>
#include <limits>
#include <string>

namespace ava::math
{
/**
 * Pack a float
 *
 * @param value value
 */
template <typename T> static inline T pack(float value)
{
#undef max
    return static_cast<T>(value / 1.0f * std::numeric_limits<T>::max());
}

/**
 * Unpack a packed float
 *
 * @param value value
 */
template <typename T> static inline float unpack(T value)
{
#undef max
    return (value * 1.0f / std::numeric_limits<T>::max());
}

/**
 * Return the T aligned value
 *
 * @param value value to align
 * @param alignment size of the alignment
 */
template <typename T> inline static T align(T value, uint32_t alignment = sizeof(uint32_t))
{
    if ((value % alignment) != 0) {
        return (value + (alignment - (value % alignment)));
    }

    return value;
}

/**
 * Return the distance between the value and the T boundary
 *
 * @param value value to check the distance of
 * @param alignment size of the alignment
 */
template <typename T> inline static uint32_t align_distance(T value, uint32_t alignment = sizeof(uint32_t))
{
    if ((value % alignment) != 0) {
        return (alignment - (value % alignment));
    }

    return 0;
}

/**
 * Return the length of an aligned string
 *
 * @param string string to check length of
 * @param alignment size of the alignment
 * @param out_padding total padding added to the string length
 */
static uint32_t aligned_string_len(const std::string& string, uint32_t alignment = sizeof(uint32_t),
                                   uint32_t* out_padding = nullptr)
{
    const size_t   length  = string.length();
    const uint32_t padding = align_distance(length, alignment);

    if (out_padding) {
        *out_padding = padding;
    }

    return static_cast<uint32_t>(length + padding);
}
}; // namespace ava::math

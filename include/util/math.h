#pragma once

#include <cstdint>
#include <limits>

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
}; // namespace ava::math

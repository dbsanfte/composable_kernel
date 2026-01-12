// Copyright (c) Advanced Micro Devices, Inc., or its affiliates.
// SPDX-License-Identifier: MIT

#pragma once

// Compatibility header for <bit>
// Provides std::bit functions for compilers that don't support C++20

#if __has_include(<bit>)
#include <bit>
#else
#include <cstring>
#include <type_traits>
#include <limits>

namespace std {
// bit_cast implementation for pre-C++20 compilers
template <typename To, typename From>
__host__ __device__
    typename std::enable_if<sizeof(To) == sizeof(From) && std::is_trivially_copyable<From>::value &&
                                std::is_trivially_copyable<To>::value,
                            To>::type
    bit_cast(const From& src) noexcept
{
    static_assert(
        std::is_trivially_constructible<To>::value,
        "This implementation requires the destination type to be trivially constructible");

    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

// popcount implementation
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, int>::type popcount(T x) noexcept
{
    int count = 0;
    while(x)
    {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

// Optimized versions using compiler intrinsics
inline int popcount(unsigned int x) noexcept { return __builtin_popcount(x); }

inline int popcount(unsigned long x) noexcept { return __builtin_popcountl(x); }

inline int popcount(unsigned long long x) noexcept { return __builtin_popcountll(x); }

// countr_zero - count trailing zeros
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, int>::type countr_zero(T x) noexcept
{
    if(x == 0)
        return std::numeric_limits<T>::digits;

    int count = 0;
    while((x & 1) == 0)
    {
        x >>= 1;
        ++count;
    }
    return count;
}

// Optimized versions using compiler intrinsics
inline int countr_zero(unsigned int x) noexcept
{
    return x == 0 ? std::numeric_limits<unsigned int>::digits : __builtin_ctz(x);
}

inline int countr_zero(unsigned long x) noexcept
{
    return x == 0 ? std::numeric_limits<unsigned long>::digits : __builtin_ctzl(x);
}

inline int countr_zero(unsigned long long x) noexcept
{
    return x == 0 ? std::numeric_limits<unsigned long long>::digits : __builtin_ctzll(x);
}

// countl_zero - count leading zeros
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, int>::type countl_zero(T x) noexcept
{
    if(x == 0)
        return std::numeric_limits<T>::digits;

    int count = 0;
    T mask    = T(1) << (std::numeric_limits<T>::digits - 1);
    while((x & mask) == 0)
    {
        ++count;
        mask >>= 1;
    }
    return count;
}

// Optimized versions using compiler intrinsics
inline int countl_zero(unsigned int x) noexcept
{
    return x == 0 ? std::numeric_limits<unsigned int>::digits : __builtin_clz(x);
}

inline int countl_zero(unsigned long x) noexcept
{
    return x == 0 ? std::numeric_limits<unsigned long>::digits : __builtin_clzl(x);
}

inline int countl_zero(unsigned long long x) noexcept
{
    return x == 0 ? std::numeric_limits<unsigned long long>::digits : __builtin_clzll(x);
}

// has_single_bit - check if value is a power of 2
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, bool>::type has_single_bit(T x) noexcept
{
    return x != 0 && (x & (x - 1)) == 0;
}

// bit_width - number of bits needed to represent the value
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, int>::type bit_width(T x) noexcept
{
    return std::numeric_limits<T>::digits - countl_zero(x);
}

// bit_ceil - smallest power of 2 >= x
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, T>::type bit_ceil(T x) noexcept
{
    if(x <= 1)
        return T(1);
    return T(1) << bit_width(T(x - 1));
}

// bit_floor - largest power of 2 <= x
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, T>::type bit_floor(T x) noexcept
{
    if(x == 0)
        return 0;
    return T(1) << (bit_width(x) - 1);
}

// rotl - rotate left
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, T>::type rotl(T x, int s) noexcept
{
    constexpr int digits = std::numeric_limits<T>::digits;
    const int r          = s % digits;
    if(r == 0)
        return x;
    if(r > 0)
        return (x << r) | (x >> (digits - r));
    return rotr(x, -r);
}

// rotr - rotate right
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value, T>::type rotr(T x, int s) noexcept
{
    constexpr int digits = std::numeric_limits<T>::digits;
    const int r          = s % digits;
    if(r == 0)
        return x;
    if(r > 0)
        return (x >> r) | (x << (digits - r));
    return rotl(x, -r);
}

} // namespace std

#endif // __has_include(<bit>)

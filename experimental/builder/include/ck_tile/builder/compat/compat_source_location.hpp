// Copyright (c) Advanced Micro Devices, Inc., or its affiliates.
// SPDX-License-Identifier: MIT

#pragma once

// Compatibility header for <source_location>
// Provides std::source_location functionality for compilers that don't support C++20

#if __has_include(<source_location>)
#include <source_location>
#else
#include <cstdint>

namespace std {
// Fallback implementation of source_location for pre-C++20 compilers
struct source_location
{
    public:
    static constexpr source_location current(const char* file           = __builtin_FILE(),
                                             const char* function       = __builtin_FUNCTION(),
                                             std::uint_least32_t line   = __builtin_LINE(),
                                             std::uint_least32_t column = 0) noexcept
    {
        source_location loc;
        loc.file_     = file;
        loc.function_ = function;
        loc.line_     = line;
        loc.column_   = column;
        return loc;
    }

    constexpr source_location() noexcept
        : file_("unknown"), function_("unknown"), line_(0), column_(0)
    {
    }

    constexpr const char* file_name() const noexcept { return file_; }
    constexpr const char* function_name() const noexcept { return function_; }
    constexpr std::uint_least32_t line() const noexcept { return line_; }
    constexpr std::uint_least32_t column() const noexcept { return column_; }

    private:
    const char* file_;
    const char* function_;
    std::uint_least32_t line_;
    std::uint_least32_t column_;
};
} // namespace std

#endif // __has_include(<source_location>)

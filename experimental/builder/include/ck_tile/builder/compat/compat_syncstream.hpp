// Copyright (c) Advanced Micro Devices, Inc., or its affiliates.
// SPDX-License-Identifier: MIT

#pragma once

// Compatibility header for <syncstream>
// Provides std::osyncstream for compilers that don't have the <syncstream> header

#if __has_include(<syncstream>)
#include <syncstream>
#else
// SLES 15 with g++ 7.5 doesn't have <syncstream> header
// We need to provide a minimal implementation ourselves
#include <ostream>
#include <streambuf>

namespace std {

// Simplified osyncstream that doesn't actually provide synchronization
// This is acceptable for compatibility purposes - the code will work,
// just without thread-safety guarantees for output
template <typename CharT,
          typename Traits    = std::char_traits<CharT>,
          typename Allocator = std::allocator<CharT>>
class basic_osyncstream : public std::basic_ostream<CharT, Traits>
{
    private:
    std::basic_streambuf<CharT, Traits>* wrapped_buf;

    public:
    using char_type      = CharT;
    using traits_type    = Traits;
    using allocator_type = Allocator;
    using int_type       = typename Traits::int_type;
    using pos_type       = typename Traits::pos_type;
    using off_type       = typename Traits::off_type;

    // Constructor that wraps an existing streambuf
    explicit basic_osyncstream(std::basic_streambuf<CharT, Traits>* buf)
        : std::basic_ostream<CharT, Traits>(buf), wrapped_buf(buf)
    {
    }

    // Constructor that wraps an existing ostream
    explicit basic_osyncstream(std::basic_ostream<CharT, Traits>& os)
        : std::basic_ostream<CharT, Traits>(os.rdbuf()), wrapped_buf(os.rdbuf())
    {
    }

    // Destructor - in real osyncstream this would emit the buffered content
    // For our simple implementation, everything is already written
    ~basic_osyncstream() override = default;

    // Get the wrapped streambuf
    std::basic_streambuf<CharT, Traits>* get_wrapped() const { return wrapped_buf; }

    // Emit - in real osyncstream this would flush the internal buffer
    // For our simple implementation, just flush the underlying stream
    void emit() { this->flush(); }
};

// Standard typedefs
using osyncstream  = basic_osyncstream<char>;
using wosyncstream = basic_osyncstream<wchar_t>;

} // namespace std

#endif // __has_include(<syncstream>)

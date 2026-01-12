// Copyright (c) Advanced Micro Devices, Inc., or its affiliates.
// SPDX-License-Identifier: MIT

#pragma once

// Compatibility header for <span>
// Provides std::span for compilers that don't have the <span> header

#if __has_include(<span>)
#include <span>
#else
// SLES 15 with g++ 7.5 doesn't have <span> header
// We need to provide a minimal implementation ourselves
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <array>
#include <limits>

namespace std {

// Special value for dynamic extent
inline constexpr size_t dynamic_extent = numeric_limits<size_t>::max();

// Minimal implementation of std::span
template <typename T, size_t Extent = dynamic_extent>
class span
{
    public:
    // Member types
    using element_type           = T;
    using value_type             = remove_cv_t<T>;
    using size_type              = size_t;
    using difference_type        = ptrdiff_t;
    using pointer                = T*;
    using const_pointer          = const T*;
    using reference              = T&;
    using const_reference        = const T&;
    using iterator               = pointer;
    using const_iterator         = const_pointer;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    static constexpr size_type extent = Extent;

    private:
    pointer data_;
    size_type size_;

    public:
    // Constructors
    constexpr span() noexcept : data_(nullptr), size_(0)
    {
        static_assert(Extent == 0 || Extent == dynamic_extent,
                      "Cannot default construct a span with static extent > 0");
    }

    constexpr span(pointer ptr, size_type count) : data_(ptr), size_(count)
    {
        static_assert(Extent == dynamic_extent || Extent == count, "Size must match static extent");
    }

    constexpr span(pointer first, pointer last)
        : data_(first), size_(static_cast<size_type>(last - first))
    {
    }

    template <size_t N>
    constexpr span(element_type (&arr)[N]) noexcept : data_(arr), size_(N)
    {
        static_assert(Extent == dynamic_extent || Extent == N,
                      "Array size must match static extent");
    }

    template <size_t N>
    constexpr span(array<value_type, N>& arr) noexcept : data_(arr.data()), size_(N)
    {
        static_assert(Extent == dynamic_extent || Extent == N,
                      "Array size must match static extent");
    }

    template <size_t N>
    constexpr span(const array<value_type, N>& arr) noexcept : data_(arr.data()), size_(N)
    {
        static_assert(Extent == dynamic_extent || Extent == N,
                      "Array size must match static extent");
    }

    // Container constructor (for vector, etc.)
    template <
        typename Container,
        typename = enable_if_t<!is_array_v<Container> && !is_same_v<remove_cv_t<Container>, span> &&
                               is_convertible_v<decltype(declval<Container>().data()), pointer>>>
    constexpr span(Container& cont) : data_(cont.data()), size_(cont.size())
    {
    }

    template <typename Container,
              typename = enable_if_t<
                  !is_array_v<Container> && !is_same_v<remove_cv_t<Container>, span> &&
                  is_convertible_v<decltype(declval<const Container>().data()), pointer>>>
    constexpr span(const Container& cont) : data_(cont.data()), size_(cont.size())
    {
    }

    // Copy constructor
    constexpr span(const span& other) noexcept = default;

    // Assignment
    constexpr span& operator=(const span& other) noexcept = default;

    // Iterators
    constexpr iterator begin() const noexcept { return data_; }
    constexpr iterator end() const noexcept { return data_ + size_; }
    constexpr const_iterator cbegin() const noexcept { return data_; }
    constexpr const_iterator cend() const noexcept { return data_ + size_; }
    constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }
    constexpr reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(cend());
    }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(cbegin());
    }

    // Element access
    constexpr reference operator[](size_type idx) const { return data_[idx]; }
    constexpr reference front() const { return data_[0]; }
    constexpr reference back() const { return data_[size_ - 1]; }
    constexpr pointer data() const noexcept { return data_; }

    // Observers
    constexpr size_type size() const noexcept { return size_; }
    constexpr size_type size_bytes() const noexcept { return size_ * sizeof(element_type); }
    constexpr bool empty() const noexcept { return size_ == 0; }

    // Subviews
    template <size_t Count>
    constexpr span<element_type, Count> first() const
    {
        return span<element_type, Count>(data_, Count);
    }

    constexpr span<element_type, dynamic_extent> first(size_type count) const
    {
        return span<element_type, dynamic_extent>(data_, count);
    }

    template <size_t Count>
    constexpr span<element_type, Count> last() const
    {
        return span<element_type, Count>(data_ + (size_ - Count), Count);
    }

    constexpr span<element_type, dynamic_extent> last(size_type count) const
    {
        return span<element_type, dynamic_extent>(data_ + (size_ - count), count);
    }

    template <size_t Offset, size_t Count = dynamic_extent>
    constexpr auto subspan() const
    {
        constexpr size_t new_extent = (Count != dynamic_extent)    ? Count
                                      : (Extent != dynamic_extent) ? (Extent - Offset)
                                                                   : dynamic_extent;
        return span<element_type, new_extent>(data_ + Offset,
                                              Count != dynamic_extent ? Count : size_ - Offset);
    }

    constexpr span<element_type, dynamic_extent> subspan(size_type offset,
                                                         size_type count = dynamic_extent) const
    {
        return span<element_type, dynamic_extent>(data_ + offset,
                                                  count == dynamic_extent ? size_ - offset : count);
    }
};

// Deduction guides
template <typename T, size_t N>
span(T (&)[N]) -> span<T, N>;

template <typename T, size_t N>
span(array<T, N>&) -> span<T, N>;

template <typename T, size_t N>
span(const array<T, N>&) -> span<const T, N>;

template <typename Container>
span(Container&) -> span<typename Container::value_type>;

template <typename Container>
span(const Container&) -> span<const typename Container::value_type>;

// Helper function to create spans
template <typename T, size_t N>
constexpr span<T, N> make_span(T (&arr)[N]) noexcept
{
    return span<T, N>(arr);
}

template <typename T, size_t N>
constexpr span<T, N> make_span(array<T, N>& arr) noexcept
{
    return span<T, N>(arr);
}

template <typename T, size_t N>
constexpr span<const T, N> make_span(const array<T, N>& arr) noexcept
{
    return span<const T, N>(arr);
}

template <typename Container>
constexpr span<typename Container::value_type> make_span(Container& cont)
{
    return span<typename Container::value_type>(cont);
}

template <typename Container>
constexpr span<const typename Container::value_type> make_span(const Container& cont)
{
    return span<const typename Container::value_type>(cont);
}

} // namespace std

#endif // __has_include(<span>)

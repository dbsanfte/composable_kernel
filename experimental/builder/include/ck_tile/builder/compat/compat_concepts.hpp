// Copyright (c) Advanced Micro Devices, Inc., or its affiliates.
// SPDX-License-Identifier: MIT

#pragma once

// Compatibility header for <concepts>
// Provides std::concepts for compilers that don't have the <concepts> header

#if __has_include(<concepts>)
#include <concepts>
#else
// Clang 18 supports concepts, but SLES 15 with g++ 7.5 doesn't have <concepts> header
// We need to provide the standard concepts ourselves
#include <type_traits>

namespace std {
// same_as concept
template <typename T, typename U>
concept same_as = is_same_v<T, U> && is_same_v<U, T>;

// derived_from concept
template <typename Derived, typename Base>
concept derived_from =
    is_base_of_v<Base, Derived> && is_convertible_v<const volatile Derived*, const volatile Base*>;

// convertible_to concept
template <typename From, typename To>
concept convertible_to =
    is_convertible_v<From, To> && requires { static_cast<To>(declval<From>()); };

// integral concept
template <typename T>
concept integral = is_integral_v<T>;

// signed_integral concept
template <typename T>
concept signed_integral = integral<T> && is_signed_v<T>;

// unsigned_integral concept
template <typename T>
concept unsigned_integral = integral<T> && !signed_integral<T>;

// floating_point concept
template <typename T>
concept floating_point = is_floating_point_v<T>;

// assignable_from concept
template <typename LHS, typename RHS>
concept assignable_from = is_lvalue_reference_v<LHS> && requires(LHS lhs, RHS&& rhs) {
    { lhs = forward<RHS>(rhs) } -> same_as<LHS>;
};

// swappable concept (simplified)
template <typename T>
concept swappable = is_move_constructible_v<T> && is_move_assignable_v<T>;

// destructible concept
template <typename T>
concept destructible = is_nothrow_destructible_v<T>;

// constructible_from concept
template <typename T, typename... Args>
concept constructible_from = destructible<T> && is_constructible_v<T, Args...>;

// default_initializable concept
template <typename T>
concept default_initializable = constructible_from<T> && requires { T{}; } && requires { ::new T; };

// move_constructible concept
template <typename T>
concept move_constructible = constructible_from<T, T> && convertible_to<T, T>;

// copy_constructible concept
template <typename T>
concept copy_constructible =
    move_constructible<T> && constructible_from<T, T&> && convertible_to<T&, T> &&
    constructible_from<T, const T&> && convertible_to<const T&, T> &&
    constructible_from<T, const T> && convertible_to<const T, T>;

// equality_comparable concept
template <typename T>
concept equality_comparable = requires(const T& a, const T& b) {
    { a == b } -> convertible_to<bool>;
    { a != b } -> convertible_to<bool>;
};

// totally_ordered concept
template <typename T>
concept totally_ordered = equality_comparable<T> && requires(const T& a, const T& b) {
    { a < b } -> convertible_to<bool>;
    { a > b } -> convertible_to<bool>;
    { a <= b } -> convertible_to<bool>;
    { a >= b } -> convertible_to<bool>;
};

// movable concept
template <typename T>
concept movable = is_object_v<T> && move_constructible<T> && assignable_from<T&, T> && swappable<T>;

// copyable concept
template <typename T>
concept copyable = copy_constructible<T> && movable<T> && assignable_from<T&, T&> &&
                   assignable_from<T&, const T&> && assignable_from<T&, const T>;

// semiregular concept
template <typename T>
concept semiregular = copyable<T> && default_initializable<T>;

// regular concept
template <typename T>
concept regular = semiregular<T> && equality_comparable<T>;

// invocable concept
template <typename F, typename... Args>
concept invocable =
    requires(F&& f, Args&&... args) { invoke(forward<F>(f), forward<Args>(args)...); };

// predicate concept
template <typename F, typename... Args>
concept predicate = invocable<F, Args...> && requires(F&& f, Args&&... args) {
    { forward<F>(f)(forward<Args>(args)...) } -> convertible_to<bool>;
};

// remove_cvref (C++20 type trait)
template <typename T>
struct remove_cvref
{
    using type = remove_cv_t<remove_reference_t<T>>;
};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

} // namespace std

#endif // __has_include(<concepts>)

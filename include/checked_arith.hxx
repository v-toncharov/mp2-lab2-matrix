#pragma once
#include <new>
#include <stdexcept>

template<typename T>
bool ret_checked_add(T& dst, T a, T b) { return !__builtin_add_overflow(a, b, &dst); }
template<typename T>
bool ret_checked_sub(T& dst, T a, T b) { return !__builtin_sub_overflow(a, b, &dst); }
template<typename T>
bool ret_checked_mul(T& dst, T a, T b) { return !__builtin_mul_overflow(a, b, &dst); }

template<typename T>
[[nodiscard]] T checked_add(T a, T b) {
    T ret;
    if (!ret_checked_add(ret, a, b)) throw std::overflow_error("overflow in checked addition");
    return ret;
}
template<> inline size_t checked_add<size_t>(size_t a, size_t b) {
    size_t ret;
    if (!ret_checked_add(ret, a, b)) throw std::bad_array_new_length();
    return ret;
}
template<typename T>
[[nodiscard]] T checked_sub(T a, T b) {
    T ret;
    if (!checked_sub(ret, a, b)) throw std::overflow_error("overflow in checked subtracition");
    return ret;
}
template<> inline size_t checked_sub<size_t>(size_t a, size_t b) {
    size_t ret;
    if (!ret_checked_sub(ret, a, b)) throw std::bad_array_new_length();
    return ret;
}
template<typename T>
[[nodiscard]] T checked_mul(T a, T b) {
    T ret;
    if(!ret_checked_mul(ret,a, b))throw std::overflow_error("overflow in checked multiplication");
    return ret;
}
template<> inline size_t checked_mul<size_t>(size_t a, size_t b) {
    size_t ret;
    if (!ret_checked_mul(ret, a, b)) throw std::bad_array_new_length();
    return ret;
}

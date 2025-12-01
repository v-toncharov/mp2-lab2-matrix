#pragma once
#include "fwd.hxx"
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <type_traits>

struct vec_out_of_bounds : std::out_of_range {
    vec_out_of_bounds(size_t i) : std::out_of_range(mk_errmsg(i)) {}
private:
    static std::string mk_errmsg(size_t i) {
        std::stringstream s;
        s << "index " << i << " is out of bounds for the vector";
        return s.str();
    }
};
struct bad_vec_slicing : std::out_of_range {
    bad_vec_slicing(std::optional<size_t> from, std::optional<size_t> to)
        : std::out_of_range(mk_errmsg(from, to)) {}
private:
    static std::string mk_errmsg(std::optional<size_t> from, std::optional<size_t> to) { // NOLINT
        std::stringstream s;
        s << "range ";
        if (from) s << *from;
        s << "..";
        if (to) s << *to;
        s << " is out of bounds for the vector";
        return s.str();
    }
};
struct vec_len_mismatch : std::runtime_error {
    vec_len_mismatch(size_t a, size_t b) : std::runtime_error(mk_errmsg(a, b)) {}
private:
    static std::string mk_errmsg(size_t a, size_t b) {
        std::stringstream s;
        s << "vector length mismatch (" << a << " and " << b << ')';
        return s.str();
    }
};

template<bool has_stride> struct size_if_stride;
template<> struct size_if_stride<false> {
    size_if_stride(size_t) noexcept {}
    [[nodiscard]] operator size_t() const noexcept { return 1; }
};
template<> struct size_if_stride<true> {
    using stride_t = size_t;
    size_if_stride() = delete;
    size_if_stride(size_t stride) noexcept : strid(stride) {}
    [[nodiscard]] operator size_t() const noexcept { return strid; }
private:
    size_t strid;
};

template<typename T, bool has_stride>
struct vec {
protected:
    friend vec<T const, has_stride>;

    /// --- fields ---
    T*                         _ptr;
    size_t                     _len;
    size_if_stride<has_stride> _stride;
    /// --- end fields ---

    template<typename OT, bool ohs>
    void assert_len_eq(vec<OT, ohs> o) const
        { if (_len != o._len) throw vec_len_mismatch(_len, o._len); }

public:
    static constexpr bool is_mutable = !std::is_const_v<T>;

    /// --- constructors and conversions ---
    vec() noexcept : _ptr(nullptr), _len(0), _stride(1) {}
    explicit vec(T* ptr, size_t len) noexcept : _ptr(ptr), _len(len), _stride(1) {}
    explicit vec(T* ptr, size_t len, size_t stride) noexcept // NOLINT obvious
        : _ptr(ptr), _len(len), _stride(stride) {}
    /// --- end constructors and conversions ---

    /// --- mutability and length conversions ---
    [[nodiscard]] vec<T      , has_stride> as_ref  () const noexcept { return *this; }
    [[nodiscard]] vec<T const, has_stride> as_const() const noexcept
        { return vec<T const, has_stride>(_ptr,_len); }
    [[nodiscard]] operator vec<T const, has_stride>() const noexcept { return as_const(); }

    [[nodiscard]] vec slice_from_nocklen(size_t start) const noexcept
        { return vec(_ptr + (start * _stride), _len - start, _stride); }
    [[nodiscard]] vec slice_to_nocklen  (size_t end  ) const noexcept
        { return vec(_ptr, end, _stride); }
    [[nodiscard]] vec slice_from(size_t start) const {
        if (start >= _len) throw bad_vec_slicing(start, std::nullopt);
        return slice_from_nocklen(start);
    }
    [[nodiscard]] vec slice_to(size_t end) const {
        if (end > _len) throw bad_vec_slicing(std::nullopt, end);
        return slice_to_nocklen(end);
    }

    [[nodiscard]] vec slice_nocklen(size_t start, size_t end) const noexcept
        { return vec(_ptr + (start * _stride), end - start, _stride); }
    [[nodiscard]] vec slice(size_t start, size_t end) const {
        if (end > start || start >= _len || end > _len) throw bad_vec_slicing(start, end);
        return slice_nocklen(start, end);
    }
    /// --- end mutability and length conversions ---

    /// --- accessors ---
    [[nodiscard]] T*     ptr()    const noexcept { return _ptr   ; }
    [[nodiscard]] size_t len()    const noexcept { return _len   ; }
    [[nodiscard]] size_t stride() const noexcept { return _stride; }

    [[nodiscard]] T& operator [](size_t i) const {
        if (i >= _len) throw vec_out_of_bounds(i);
        return _ptr[i * _stride];
    }
    /// --- end accessors ---

    /// --- comparison ---
    template<typename OT, bool ohs>
    bool operator ==(vec<OT, ohs> o) const noexcept {
        if (_len != o._len) return false;
        return eq_nocklen(o);
    }
    template<typename OT, bool ohs>
    bool eq_nocklen(vec<OT, ohs> o) const noexcept {
        for (size_t i = 0; i < _len; i++) if (_ptr[i] != o._ptr[i]) return false;
        return true;
    }
    /// --- end comparison ---

    /// --- in-place modification ---
    void zero_fill() const noexcept requires is_mutable { memset(_ptr, 0, _len * sizeof(T)); }

    template<typename OT, bool ohs>
    vec const& copy_from_nocklen(vec<OT, ohs> o) const noexcept requires is_mutable
        { for (size_t i = 0; i < _len; i++) (*this)[i] = o[i]; return *this; }

    template<typename OT, bool ohs>
    vec const& copy_from(vec<OT, ohs> o, bool zero_rest = true)
    const noexcept requires is_mutable {
        if (_ptr == o.ptr()) return *this;
        size_t copylen = std::min(_len, o._len);
        slice_to(copylen).copy_from_nocklen(o);
        if (zero_rest) for (size_t i = copylen; i < _len; i++) (*this)[i] = T {};
        return *this;
    }
    /// --- end in-place modification ---

    /// --- in-place arithmetic with length unsafety ---
    template<typename OT, bool ohs>
    vec const& add_assign_nocklen(vec<OT, ohs> o) const noexcept requires is_mutable {
        for (size_t i = 0; i < _len; i++) (*this)[i] += o[i];
        return *this;
    }
    template<typename OT, bool ohs>
    vec const& sub_assign_nocklen(vec<OT, ohs> o) const noexcept requires is_mutable {
        for (size_t i = 0; i < _len; i++) (*this)[i] -= o[i];
        return *this;
    }
    template<typename OT, bool ohs>
    T dot_nocklen(vec<OT, ohs> o) const noexcept {
        std::remove_const_t<T> rslt {};
        for (size_t i = 0; i < _len; i++) rslt += _ptr[i] * o._ptr[i];
        return rslt;
    }
    /// --- end in-place arithmetic with length unsafety ---

    /// --- in-place arithmetic ---
    template<typename OT, bool ohs>
    vec const& operator +=(vec<OT, ohs> o) const requires is_mutable
        { assert_len_eq(o); return add_assign_nocklen(o); }
    template<typename OT, bool ohs>
    vec const& operator -=(vec<OT, ohs> o) const requires is_mutable
        { assert_len_eq(o); return sub_assign_nocklen(o); }

    template<typename OT, bool ohs> vec& operator +(vec<OT, ohs>) = delete;
    template<typename OT, bool ohs> vec& operator -(vec<OT, ohs>) = delete;
    vec& operator +(T) = delete;
    vec& operator -(T) = delete;
    vec& operator *(T) = delete;
    vec& operator /(T) = delete;

    vec const& operator +=(T v) const noexcept requires is_mutable
        { for (size_t i = 0; i < _len; i++) _ptr[i] += v; return *this; }
    vec const& operator -=(T v) const noexcept requires is_mutable
        { for (size_t i = 0; i < _len; i++) _ptr[i] -= v; return *this; }
    vec const& operator *=(T v) const noexcept requires is_mutable
        { for (size_t i = 0; i < _len; i++) _ptr[i] *= v; return *this; }
    vec const& operator /=(T v) const noexcept requires is_mutable
        { for (size_t i = 0; i < _len; i++) _ptr[i] /= v; return *this; }

    template<typename OT, bool ohs>
    T operator *(vec<OT, ohs> o) const { assert_len_eq(o); return dot_nocklen(o); }
    /// --- end in-place arithmetic ---
};

/// --- printing ---
template<typename T, bool hs> std::ostream& operator <<(std::ostream& o, vec<T const, hs> v) {
    for (size_t i = 0; i < v.len(); i++) o << std::setw(6) << v[i];
    return o;
}
template<typename T, bool hs> std::ostream& operator <<(std::ostream& o, vec<T, hs> v)
    { return o << static_cast<vec<T const, hs>>(v); }
/// --- end printing ---

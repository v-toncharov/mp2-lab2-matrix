#pragma once
#include "vec.hxx"

template<typename T>
struct owned_vec : vec<T, false> {
    using vec = vec<T, false>;
    static_assert(vec::is_mutable);

public:
    /// --- resource management ---
    owned_vec() noexcept = default;
    explicit owned_vec(size_t len, bool zero_mem = true) : vec(nullptr, len) {
        vec::_ptr = new T[len]; // NOLINT we own this
        if (zero_mem) vec::zero_fill();
    }
    void forget() noexcept { vec::_ptr = nullptr; vec::_len = 0; }

    operator vec() && = delete;

    // Copy constructor
    owned_vec(vec const& o) : owned_vec(o.len(), false)
        { memcpy(vec::_ptr, o.ptr(), o.len() * sizeof(T)); }
    owned_vec(owned_vec const& o) : owned_vec(static_cast<vec const&>(o)) {}

    // Copy assignment
    owned_vec& operator =(vec const& o) {
        if (vec::_ptr == o.ptr()) return *this;
        if (vec::_len < o.len()) {
            this->~owned_vec();
            new(this) owned_vec(o.len(), false);
        }
        vec::_len = o.len();
        memcpy(vec::_ptr, o.ptr(), o.len() * sizeof(T));
        return *this;
    }
    owned_vec& operator =(owned_vec const& o) // NOLINT self-assignment
        { return *this = static_cast<vec const&>(o); } // NOLINT return *this

    template<typename U, bool ohs>
    void copy_from(::vec<U, ohs> const& o, bool zero_rest = true) &
        { vec::template copy_from<U, ohs>(o, zero_rest); }
    template<typename U, bool ohs>
    void copy_from(::vec<U, ohs> const& o) && = delete;

    // Move construction and assignment
    owned_vec(owned_vec&& o) noexcept : vec(o._ptr, o._len) { o.forget(); }
    owned_vec& operator =(owned_vec&& o) noexcept {
        if (this == &o) return *this;
        ~owned_vec();
        new(this) vec(o._ptr, o._len);
        o.forget();
    }

    ~owned_vec() { delete[] vec::_ptr; } // NOLINT gsl::owner
    /// --- end resource management ---

    /// --- non-in-place arithmetic ---
    template<typename OT, bool ohs> owned_vec operator +(::vec<OT, ohs> o) const
        { owned_vec rslt = *this; rslt += o; return rslt; }
    template<typename OT, bool ohs> owned_vec operator -(::vec<OT, ohs> o) const
        { owned_vec rslt = *this; rslt -= o; return rslt; }

    owned_vec operator +(T v) const { owned_vec rslt = *this; rslt += v; return rslt; }
    owned_vec operator -(T v) const { owned_vec rslt = *this; rslt -= v; return rslt; }
    owned_vec operator *(T v) const { owned_vec rslt = *this; rslt *= v; return rslt; }
    owned_vec operator /(T v) const { owned_vec rslt = *this; rslt /= v; return rslt; }

    template<typename OT, bool ohs> T operator *(::vec<OT, ohs> o) const
        { return vec::as_const() * o; }
    /// --- end non-in-place arithmetic ---
};

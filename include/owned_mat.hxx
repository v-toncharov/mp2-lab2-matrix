#pragma once
#include "checked_arith.hxx"
#include "mat.hxx"

template<typename T, mat_maj maj> requires (!std::is_const_v<T>)
struct owned_mat final : mat<T, maj> {
    using mat = mat<T, maj>;

    /// --- resource management ---
    explicit owned_mat(size_t rows, size_t cols, bool zero_mem = true)
        : mat(nullptr, rows, cols) {
        size_t num_cells = checked_mul(rows, cols);
        mat::elems = new T[num_cells]; // NOLINT, we own this
        if (zero_mem) mat::zero_fill();
    }
    void forget() noexcept { mat::elems = nullptr; mat::rows = mat::cols = 0; }

    operator mat() && = delete;

    // Copy constructor
    owned_mat(mat const& o) : owned_mat(o.n_rows(), o.n_cols(), false)
        { memcpy(mat::elems, o.base_ptr(), o.n_rows() * o.n_cols() * sizeof(T)); }
    owned_mat(owned_mat const& o) : owned_mat(static_cast<mat const&>(o)) {}

    // Copy assignment
    owned_mat& operator =(mat const& o) {
        if (mat::elems == o.base_ptr()) return *this;
        if (mat::rows < o.n_rows() || mat::cols < o.n_cols()) {
            this->~owned_mat();
            new(this) owned_mat(o.n_rows(), o.n_cols(), false);
        }
        mat::rows = o.n_rows();
        mat::cols = o.n_cols();
        memcpy(mat::elems, o.base_ptr(), o.n_rows() * o.n_cols() * sizeof(T));
        return *this;
    }
    owned_mat& operator =(owned_mat const& o) // NOLINT self-assignment
        { return *this = static_cast<mat const&>(o); } // NOLINT return *this

    template<typename U, mat_maj om>
    void copy_from(::mat<U, om> const& o, bool zero_rest = true) &
        { mat::template copy_from<T, maj>(o, zero_rest); }
    template<typename U, mat_maj om>
    void copy_from(::mat<U, om> const& o) && = delete;

    // Move construction and assignment
    owned_mat(owned_mat&& o) noexcept : mat(o.elems, o.rows, o.cols) { o.forget(); }
    owned_mat& operator =(owned_mat&& o) noexcept {
        if (this == &o) return *this;
        this->~owned_mat();
        new(this) mat(o.elems, o.rows, o.cols);
        o.forget();
        return *this;
    }

    ~owned_mat() { delete[] mat::elems; } // NOLINT gsl::owner
    /// --- end resource management ---

    /// --- non-in-place arithmetic ---
    template<typename OT, mat_maj om>
    owned_mat operator +(owned_mat<OT, om> const& o) const {
        mat::assert_size_eq(o);
        owned_mat rslt {*this};
        rslt.add_assign_nocklen(o);
        return rslt;
    }
    template<typename OT, mat_maj om>
    owned_mat operator -(owned_mat<OT, om> const& o) const {
        mat::assert_size_eq(o);
        owned_mat rslt {*this};
        rslt.sub_assign_nocklen(o);
        return rslt;
    }
    /// --- end non-in-place arithmetic ---
};
template<typename T> using owned_col_mat = owned_mat<T, mat_maj::col>;
template<typename T> using owned_row_mat = owned_mat<T, mat_maj::row>;

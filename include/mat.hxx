#pragma once
#include "fwd.hxx"
#include "vec.hxx"
#include <cstring>
#include <iostream>

struct mat_coord final { size_t row, col; };

struct mat_out_of_bounds : std::out_of_range {
    mat_out_of_bounds(size_t i, bool col) : std::out_of_range(mk_errmsg(i, col)) {}
private:
    static std::string mk_errmsg(size_t i, bool col) {
        std::stringstream s;
        s << "matrix " << (col ? "column" : "row") << " index " << i << " is out of bounds";
        return s.str();
    }
};
struct mat_size_mismatch : std::runtime_error {
    mat_size_mismatch(size_t ra, size_t ca, size_t rb, size_t cb)
        : std::runtime_error(mk_errmsg(ra, ca, rb, cb)) {}
private:
    static std::string mk_errmsg(size_t ra, size_t ca, size_t rb, size_t cb) {
        std::stringstream s;
        s << "mismatch of matrix dimensions ("<<ra<<'x'<<ca<<" and "<<rb<<'x'<<cb<<')';
        return s.str();
    }
};

/// --- dispatch of matrix indexing to strideful and strideless vector types ---
template<typename T, mat_maj maj> struct matvecdisp {};
template<typename T, mat_maj maj> using row_vec = typename matvecdisp<T, maj>::trow;
template<typename T, mat_maj maj> using col_vec = typename matvecdisp<T, maj>::tcol;
template<typename T, mat_maj maj> using maj_vec = typename matvecdisp<T, maj>::tmaj;
template<typename T, mat_maj maj> using min_vec = typename matvecdisp<T, maj>::tmin;

template<typename T> struct matvecdisp<T, mat_maj::row> {
    using trow = vec<T, false>;
    using tcol = vec<T, true >;
    using tmaj = trow;
    using tmin = tcol;
private:
    using tmat = mat<T, mat_maj::row>; friend tmat;
    static trow row(tmat m, size_t i) {
        if (i >= m.n_rows()) throw mat_out_of_bounds(i, false);
        return trow(m.row_ptr(i), m.n_cols());
    }
    static tcol col(tmat m, size_t i) {
        if (i >= m.n_cols()) throw mat_out_of_bounds(i, true);
        return tcol(m.col_ptr(i), m.n_rows(), m.row_stride());
    }
    static vec<T, false> majvec(tmat m, size_t i) { return row(m, i); }
    static vec<T, true > minvec(tmat m, size_t i) { return col(m, i); }
};
template<typename T> struct matvecdisp<T, mat_maj::col> {
    using trow = vec<T, true >;
    using tcol = vec<T, false>;
    using tmaj = tcol;
    using tmin = trow;
private:
    using tmat = mat<T, mat_maj::col>; friend tmat;
    static trow row(tmat m, size_t i) {
        if (i >= m.n_rows()) throw mat_out_of_bounds(i, false);
        return trow(m.row_ptr(i), m.n_cols(), m.col_stride());
    }
    static tcol col(tmat m, size_t i) {
        if (i >= m.n_cols()) throw mat_out_of_bounds(i, true);
        return tcol(m.col_ptr(i), m.n_rows());
    }
    static vec<T, false> majvec(tmat m, size_t i) { return col(m, i); }
    static vec<T, true > minvec(tmat m, size_t i) { return row(m, i); }
};
/// --- end of dispatch of matrix indexing to strideful and strideless vector types ---

template<typename T, mat_maj maj>
struct mat {
protected:
    /// --- fields ---
    T*     elems {};
    size_t rows  {};
    size_t cols  {};
    /// --- end fields ---

    template<typename OT, mat_maj om>
    void assert_size_eq(mat<OT, om> o) const {
        if (rows != o.rows || cols != o.cols) throw mat_size_mismatch(rows, cols, o.rows, o.cols);
    }

public:
    static constexpr bool is_mutable = !std::is_const_v<T>;
    using row_vec = row_vec<T, maj>;
    using col_vec = col_vec<T, maj>;
    using maj_vec = maj_vec<T, maj>;
    using min_vec = min_vec<T, maj>;

    /// --- constructors ---
    mat() noexcept = default;

    explicit mat(T* elems, size_t rows, size_t cols) noexcept // NOLINT (easily swapped)
        : elems(elems), rows(rows), cols(cols) {}
    /// --- end constructors ---

    /// --- mutability conversions ---
    [[nodiscard]] mat<T      , maj> as_ref  () const noexcept { return *this; }
    [[nodiscard]] mat<T const, maj> as_const() const noexcept
        { return mat<T const, maj>(elems, rows, cols); }
    [[nodiscard]] operator mat<T const, maj>() const noexcept { return as_const(); }
    /// --- end mutability conversions ---

    /// --- reinterpretation ---
    [[nodiscard]] mat<T, maj_transpose<maj>> transposed() const noexcept
        { return mat(elems, rows, cols); }
    /// --- end reinterpretation ---

    /// --- accessors ---
    [[nodiscard]] T*     base_ptr  ()           const noexcept {return elems                   ;}
    [[nodiscard]] size_t row_stride()           const noexcept {return maj==mat_maj::row?cols:1;}
    [[nodiscard]] size_t col_stride()           const noexcept {return maj==mat_maj::col?rows:1;}
    [[nodiscard]] size_t row_idx   (size_t row) const noexcept {return row * row_stride()      ;}
    [[nodiscard]] size_t col_idx   (size_t col) const noexcept {return col * col_stride()      ;}
    [[nodiscard]] T*     row_ptr   (size_t row) const noexcept {return elems + row_idx(row)    ;}
    [[nodiscard]] T*     col_ptr   (size_t col) const noexcept {return elems + col_idx(col)    ;}

    [[nodiscard]] size_t n_rows() const noexcept { return rows; }
    [[nodiscard]] size_t n_cols() const noexcept { return cols; }
    [[nodiscard]] size_t n_maj () const noexcept { return maj==mat_maj::col?n_cols():n_rows(); }
    [[nodiscard]] size_t n_min () const noexcept { return maj==mat_maj::col?n_rows():n_cols(); }
    /// --- end accessors ---

    /// --- vector acquisition ---
    [[nodiscard]] row_vec row(size_t row) const
        { return matvecdisp<T, maj>::row(*this, row); }
    [[nodiscard]] col_vec col(size_t col) const
        { return matvecdisp<T, maj>::col(*this, col); }
    [[nodiscard]] row_vec operator [](size_t i) const { return row(i); }

    // The m parameter can be used to match rows with rows or columns with columns
    // even when matrices differ in storage order.
    template<mat_maj m = maj>
    [[nodiscard]] ::maj_vec<T, m> majvec(size_t i) const
        { return matvecdisp<T, m>::majvec(*this, i); }
    template<mat_maj m = maj>
    [[nodiscard]] ::min_vec<T, m> minvec(size_t i) const
        { return matvecdisp<T, m>::minvec(*this, i); }
    /// --- end vector acquisition ---

    /// --- comparison ---
    template<typename OT, mat_maj om>
    bool operator ==(mat<OT, om> const& o) const noexcept {
        if (rows != o.rows || cols != o.cols) return false;
        for (size_t i = 0; i < n_maj(); i++) {
            if (!majvec(i).eq_nocklen(o.template majvec<maj>(i))) return false;
        }
        return true;
    }
    /// --- end comparison ---

    /// --- in-place modification ---
    template<typename OT, mat_maj om>
    mat& add_assign_nocklen(mat<OT, om> const& o) requires is_mutable
        { for (size_t i = 0; i < n_maj(); i++)majvec(i)+=o.template majvec<maj>(i);return *this; }
    template<typename OT, mat_maj om>
    mat& sub_assign_nocklen(mat<OT, om> const& o) requires is_mutable
        { for (size_t i = 0; i < n_maj(); i++)majvec(i)-=o.template majvec<maj>(i);return *this; }

    template<typename OT, mat_maj om>
    mat& operator +=(mat<OT, om> const& o) requires is_mutable {
        assert_size_eq(o);
        return add_assign_nocklen(o);
    }
    template<typename OT, mat_maj om>
    mat& operator -=(mat<OT, om> const& o) requires is_mutable {
        assert_size_eq(o);
        return sub_assign_nocklen(o);
    }

    template<typename U = T, mat_maj om = maj>
    void copy_from_nocklen(mat<U, om> const& o) const noexcept requires is_mutable {
        for (size_t i = 0; i < n_maj(); i++) {
            majvec(i).copy_from_nocklen(o.template majvec<maj>(i));
        }
    }
    template<typename U = T, mat_maj om = maj>
    void copy_from(mat<U, om> const& o) const requires is_mutable {
        if (o.rows < rows || o.cols < cols) throw mat_size_mismatch(rows, cols, o.rows, o.cols);
        copy_from_nocklen(o);
    }

    void zero_fill() const noexcept requires is_mutable
        { memset(elems, 0, rows * cols * sizeof(T)); }
    /// --- end in-place modification ---
};
template<typename T> using col_mat = mat<T, mat_maj::col>;
template<typename T> using row_mat = mat<T, mat_maj::row>;

template<typename T, mat_maj maj>
std::ostream& operator <<(std::ostream& o, mat<T, maj> const& m) {
    for (size_t r = 0; r < m.n_rows(); r++) {
        auto const& row = m[r];
        for (size_t c = 0; c < m.n_cols(); c++) {
            o << std::setw(6) << row[c];
        }
        o << '\n';
    }
    return o;
}

static inline void inst(mat<double, mat_maj::col> a, mat<double, mat_maj::col> b) {
    a.copy_from(b);
}

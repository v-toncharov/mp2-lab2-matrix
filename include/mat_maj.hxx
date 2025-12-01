#pragma once
enum class mat_maj : bool { col, row };

template<mat_maj maj> class transpose_tpl {};
template<>class transpose_tpl<mat_maj::col> {public:static constexpr mat_maj val = mat_maj::row;};
template<>class transpose_tpl<mat_maj::row> {public:static constexpr mat_maj val = mat_maj::col;};
template<mat_maj maj> constexpr mat_maj maj_transpose = transpose_tpl<maj>::val;

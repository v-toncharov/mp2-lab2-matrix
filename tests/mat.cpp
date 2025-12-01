#include "owned_mat.hxx"
#include <gtest.h>

// NOLINTBEGIN
TEST(mat, create) { ASSERT_NO_THROW(owned_col_mat<int> m(5, 6)); }
TEST(mat, create_too_large_fails) { ASSERT_ANY_THROW(owned_col_mat<int> m(SIZE_MAX, SIZE_MAX)); }
TEST(mat, create_with_negative_length_fails) { ASSERT_ANY_THROW(owned_col_mat<int> m(-1, -1)); }
TEST(mat, create_copied) {
    owned_col_mat<int> a(12, 20);
    ASSERT_NO_THROW(owned_col_mat<int> b(a));
}

TEST(mat, copy_equal_to_original) {
    owned_col_mat<int> m1(4, 4);
    m1[0][0] = m1[0][1] = m1[2][2] = m1[3][3] = 1;
    owned_col_mat<int> m2 {m1};
    ASSERT_EQ(m1, m2);
}

TEST(mat, copy_has_its_own_memory) {
    owned_col_mat<int> m(4, 4);
    owned_col_mat<int> m2 {m};
    ASSERT_NE(&m[0][0], &m2[0][0]);
}

TEST(mat, get_size) {
    owned_col_mat<int> m(10, 4);
    ASSERT_EQ(m.n_rows(), 10);
    ASSERT_EQ(m.n_cols(), 4);
}

TEST(mat, get_and_set_element) {
    owned_col_mat<int> m(4, 3);
    m[1][2] = 3;
    ASSERT_EQ(m[1][2], 3);
}

TEST(mat, setting_element_with_negative_index_fails) {
    owned_col_mat<int> m(3, 4);
    ASSERT_ANY_THROW(m[-1][-2] = 4);
}

TEST(mat, setting_element_out_of_bounds_fails) {
    owned_col_mat<int> m(3, 4);
    ASSERT_ANY_THROW(m[5][6] = 7);
}

TEST(mat, assign_to_self) {
    owned_col_mat<int> m(3, 4);
    m[1][2] = 3;
    m = m;
    ASSERT_EQ(m[1][2], 3);
}
TEST(mat, assign_with_equal_size) {
    owned_col_mat<int> m1(3, 4), m2(3, 4);
    m1[2][3] = 4;
    m2 = m1;
    ASSERT_EQ(m2[2][3], 4);
}
TEST(mat, assignment_changes_size) {
    owned_col_mat<int> m1(3, 4), m2(5, 6);
    m2 = m1;
    ASSERT_EQ(m2.n_rows(), 3);
    ASSERT_EQ(m2.n_cols(), 4);
}
TEST(mat, assign_with_different_size) {
    owned_col_mat<int> m1(3, 4), m2(1, 2);
    m1[2][3] = 4;
    m2 = m1;
    ASSERT_EQ(m2.n_rows(), 3);
    ASSERT_EQ(m2.n_cols(), 4);
    ASSERT_EQ(m2[2][3], 4);
}

TEST(mat, compare_equal) {
    owned_col_mat<int> m1(3, 4), m2(3, 4);
    m1[0][0] = m2[0][0] = 2;
    m1[2][3] = m2[2][3] = 4;
    ASSERT_EQ(m1, m2);
}

TEST(mat, compare_with_self) {
    owned_col_mat<int> m(3, 4);
    m[0][0] = 2;
    ASSERT_EQ(m, m);
}
TEST(mat, different_sizes_are_not_equal) {
    owned_col_mat<int> m1(5, 6), m2(7, 8);
    m1[0][0] = m2[0][0] = 2;
    m1[2][3] = m2[2][3] = 4;
    ASSERT_NE(m1, m2);
}

TEST(mat, add_with_equal_size) {
    owned_col_mat<int> m1(3, 4), m2(3, 4);
    m1[0][0] = 1;
    m2[1][1] = 2;
    m2 += m1;
    ASSERT_EQ(m2[0][0], 1);
    ASSERT_EQ(m2[1][1], 2);
}
TEST(mat, add_with_different_size_fails) {
    owned_col_mat<int> m1(3, 4), m2(5, 6);
    ASSERT_ANY_THROW(m2 += m1);
}

TEST(mat, subtract_with_equal_size) {
    owned_col_mat<int> m1(3, 4), m2(3, 4);
    m1[0][0] = 1;
    m2[1][1] = 2;
    m2 -= m1;
    ASSERT_EQ(m2[0][0], -1);
    ASSERT_EQ(m2[1][1], 2);
}
TEST(mat, subtract_with_different_size_fails) {
    owned_col_mat<int> m1(3, 4), m2(5, 6);
    ASSERT_ANY_THROW(m2 -= m1);
}
// NOLINTEND

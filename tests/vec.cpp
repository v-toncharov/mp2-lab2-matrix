#include "owned_vec.hxx"
#include <gtest.h>

// NOLINTBEGIN
TEST(vec, create) { ASSERT_NO_THROW(owned_vec<int> v(5)); }
TEST(vec, create_too_large_fails) { ASSERT_ANY_THROW(owned_vec<int> v(SIZE_MAX)); }
TEST(vec, create_with_negative_length_fails) { ASSERT_ANY_THROW(owned_vec<int> v(-5)); }
TEST(vec, create_copied) {
    owned_vec<int> a(10);
    ASSERT_NO_THROW(owned_vec<int> b(a));
}

TEST(vec, copy_equal_to_original) {
    owned_vec<int> a {5};
    for (size_t i = 0; i < a.len(); i++) a[i] = i;
    owned_vec<int> b {a};
    ASSERT_EQ(b, a);
}
TEST(vec, copy_has_its_own_memory) {
    owned_vec<int> a {3};
    owned_vec<int> b {a};
    ASSERT_NE(&a[0], &b[0]);
}

TEST(vec, get_len) {
    owned_vec<int> a {4};
    ASSERT_EQ(a.len(), 4);
}

TEST(vec, set_and_get_element) {
  owned_vec<int> a {4};
  a[0] = 4;
  ASSERT_EQ(a[0], 4);
}

TEST(vec, setting_element_with_negative_index_fails) {
  owned_vec<int> a {3};
  ASSERT_ANY_THROW(a[-1] = 20);
}

TEST(vec, setting_element_out_of_bounds_fails) {
  owned_vec<int> a {3};
  ASSERT_ANY_THROW(a[3] = 20);
}

TEST(vec, assign_to_self) {
    owned_vec<int> a {3};
    a[0] = 1;
    a = a;
    ASSERT_EQ(a[0], 1);
}

TEST(vec, assign_with_equal_size) {
    owned_vec<int> v1(3), v2(3);
    v1[2] = 4;
    v2 = v1;
    ASSERT_EQ(v2[2], 4);
}
TEST(vec, assignment_changes_size) {
    owned_vec<int> v1(3), v2(5);
    v2 = v1;
    ASSERT_EQ(v2.len(), 3);
}
TEST(vec, assign_with_different_size) {
    owned_vec<int> v1(3), v2(5);
    v1[2] = 4;
    v2 = v1;
    ASSERT_EQ(v2.len(), 3);
    ASSERT_EQ(v2[2], 4);
}

TEST(vec, compare_equal) {
    owned_vec<int> v1(3), v2(3);
    v1[0] = v2[0] = 2;
    v1[2] = v2[2] = 4;
    ASSERT_EQ(v1, v2);
}

TEST(vec, compare_with_self) {
    owned_vec<int> v(3);
    v[0] = 2;
    ASSERT_EQ(v, v);
}

TEST(vec, with_different_sizes_are_not_equal) {
    owned_vec<int> v1(5), v2(7);
    v1[0] = v2[0] = 2;
    v1[2] = v2[2] = 4;
    ASSERT_NE(v1, v2);
}

TEST(vec, add_scalar_to_vector) {
    owned_vec<int> v(5);
    v += 5;
    for (size_t i = 0; i < v.len(); i++) ASSERT_EQ(v[i], 5);
}

TEST(vec, subtract_scalar_from_vector) {
    owned_vec<int> v(5);
    v -= 5;
    for (size_t i = 0; i < v.len(); i++) ASSERT_EQ(v[i], -5);
}

TEST(vec, multiply_vector_by_scalar) {
    owned_vec<int> v(5);
    v += 1;
    v *= 5;
    for (size_t i = 0; i < v.len(); i++) ASSERT_EQ(v[i], 5);
}

TEST(vec, add_with_equal_size) {
    owned_vec<int> v1(3), v2(3);
    v1[0] = 1;
    v2[1] = 2;
    v1[2] = v2[2] = 3;
    owned_vec<int> v3 = v1 + v2;
    ASSERT_EQ(v3[0], 1);
    ASSERT_EQ(v3[1], 2);
    ASSERT_EQ(v3[2], 6);
}
TEST(vec, add_with_different_size_fails) {
    owned_vec<int> v1(3), v2(4);
    ASSERT_ANY_THROW(v1 + v2);
}

TEST(vec, subtract_with_equal_size) {
    owned_vec<int> v1(3), v2(3);
    v1[0] = 1;
    v2[1] = 2;
    v1[2] = v2[2] = 3;
    owned_vec<int> v3 = v1 - v2;
    ASSERT_EQ(v3[0],  1);
    ASSERT_EQ(v3[1], -2);
    ASSERT_EQ(v3[2],  0);
}

TEST(vec, subtract_with_different_size_fails) {
    owned_vec<int> v1(3), v2(4);
    ASSERT_ANY_THROW(v1 - v2);
}

TEST(vec, dot_with_equal_size) {
    owned_vec<int> v1(3), v2(3);
    v1[0] = 1;
    v2[1] = 2;
    v1[2] = v2[2] = 3;
    int dot = v1.as_const() * v2.as_const();
    ASSERT_EQ(dot, 9);
}

TEST(vec, dot_with_different_size_fails) {
    owned_vec<int> v1(3), v2(4);
    ASSERT_ANY_THROW(v1 * v2);
}
// NOLINTEND

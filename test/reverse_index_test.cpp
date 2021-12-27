#include "gtest/gtest.h"

#include "reverse_index.hpp"

TEST(ReverseDAGIndex, AddsAndRemoves) {
    int a, b;
    ReverseDAGIndex<int> parents;

    EXPECT_EQ(parents.get_parent_count(a), 0);
    parents.add(a, b);
    EXPECT_EQ(parents.get_parent_count(a), 1);
    EXPECT_EQ(parents.get_parent_count(b), 0);

    // Add same link again.
    EXPECT_THROW(parents.add(a, b), std::logic_error);

    parents.remove(a, b);
    EXPECT_EQ(parents.get_parent_count(a), 0);
    EXPECT_EQ(parents.get_parent_count(b), 0);

    // Remove non-existent link.
    EXPECT_THROW(parents.remove(a, b), std::logic_error);
}

TEST(ReverseDAGIndex, ReplacesInput) {
    int a, b, c;
    ReverseDAGIndex<int> parents;

    parents.add(a, c);

    // B doesn't exist, but that's OK, it simply has no parents.
    EXPECT_NO_THROW(parents.replace_input(b, c));

    // Replace an input with one that already has parents.
    EXPECT_THROW(parents.replace_input(b, a), std::logic_error);

    parents.replace_input(a, b);
    EXPECT_EQ(parents.get_parent_count(a), 0);
    EXPECT_EQ(parents.get_parent_count(b), 1);
}

#include "gtest/gtest.h"

#include "utils.hpp"

using utils::ReferenceCounter;

TEST(ReferenceCounter, CountsUpAndDown) {
    int n = 0;
    {
        ReferenceCounter c1(n);
        ASSERT_EQ(n, 1);
        {
            ReferenceCounter c2(n);
            ASSERT_EQ(n, 2);
        }
        ASSERT_EQ(n, 1);
    }
    ASSERT_EQ(n, 0);
}

TEST(ReferenceCounter, HandlesCopyAndMove) {
    int n = 0;
    ReferenceCounter c1(n);
    ASSERT_EQ(n, 1);
    ReferenceCounter c2 = c1;
    ASSERT_EQ(n, 2);
    ReferenceCounter c3 = std::move(c1);
    ASSERT_EQ(n, 2);
}

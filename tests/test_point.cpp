#include <gtest/gtest.h>
#include "../src/point.h"

TEST(PointTest, DefaultConstructor) {
    Point p;
    EXPECT_FLOAT_EQ(p.x(), 0.0f);
    EXPECT_FLOAT_EQ(p.y(), 0.0f);
}

TEST(PointTest, ParameterizedConstructor) {
    Point p(3.5f, -2.1f);
    EXPECT_FLOAT_EQ(p.x(), 3.5f);
    EXPECT_FLOAT_EQ(p.y(), -2.1f);
}

TEST(PointTest, ZeroValues) {
    Point p(0.0f, 0.0f);
    EXPECT_FLOAT_EQ(p.x(), 0.0f);
    EXPECT_FLOAT_EQ(p.y(), 0.0f);
}

TEST(PointTest, NegativeCoordinates) {
    Point p(-100.0f, -200.0f);
    EXPECT_FLOAT_EQ(p.x(), -100.0f);
    EXPECT_FLOAT_EQ(p.y(), -200.0f);
}

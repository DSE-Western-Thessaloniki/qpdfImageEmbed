#include <gtest/gtest.h>
#include "../src/rect.h"

TEST(RectTest, DefaultConstructor) {
    Rect r;
    EXPECT_DOUBLE_EQ(r.width(), 0.0);
    EXPECT_DOUBLE_EQ(r.height(), 0.0);
    EXPECT_DOUBLE_EQ(r.x(), 0.0);
    EXPECT_DOUBLE_EQ(r.y(), 0.0);
}

TEST(RectTest, SetCoordsPositive) {
    Rect r;
    r.setCoords(10, 20, 100, 200);
    EXPECT_DOUBLE_EQ(r.x(), 10);
    EXPECT_DOUBLE_EQ(r.y(), 20);
    EXPECT_DOUBLE_EQ(r.width(), 90);
    EXPECT_DOUBLE_EQ(r.height(), 180);
}

TEST(RectTest, SetCoordsReversed) {
    Rect r;
    r.setCoords(100, 200, 10, 20);
    EXPECT_DOUBLE_EQ(r.x(), 10);
    EXPECT_DOUBLE_EQ(r.y(), 20);
    EXPECT_DOUBLE_EQ(r.width(), 90);
    EXPECT_DOUBLE_EQ(r.height(), 180);
}

TEST(RectTest, SetCoordsNegative) {
    Rect r;
    r.setCoords(-100, -200, -10, -20);
    EXPECT_DOUBLE_EQ(r.x(), -100);
    EXPECT_DOUBLE_EQ(r.y(), -200);
    EXPECT_DOUBLE_EQ(r.width(), 90);
    EXPECT_DOUBLE_EQ(r.height(), 180);
}

TEST(RectTest, ZeroDimensions) {
    Rect r;
    r.setCoords(50, 50, 50, 50);
    EXPECT_DOUBLE_EQ(r.width(), 0);
    EXPECT_DOUBLE_EQ(r.height(), 0);
}

TEST(RectTest, MixedSignCoords) {
    Rect r;
    r.setCoords(-50, -50, 50, 50);
    EXPECT_DOUBLE_EQ(r.x(), -50);
    EXPECT_DOUBLE_EQ(r.y(), -50);
    EXPECT_DOUBLE_EQ(r.width(), 100);
    EXPECT_DOUBLE_EQ(r.height(), 100);
}

TEST(RectTest, FractionalCoords) {
    Rect r;
    r.setCoords(1.5, 2.5, 10.5, 20.5);
    EXPECT_DOUBLE_EQ(r.x(), 1.5);
    EXPECT_DOUBLE_EQ(r.y(), 2.5);
    EXPECT_DOUBLE_EQ(r.width(), 9.0);
    EXPECT_DOUBLE_EQ(r.height(), 18.0);
}

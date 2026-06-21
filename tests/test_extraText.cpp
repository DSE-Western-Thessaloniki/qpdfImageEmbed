#include <gtest/gtest.h>
#include "../src/extraText.h"
#include "../src/logger.h"

Logger logger;

TEST(ExtraTextTest, PlainTextOnly) {
    ExtraText et("Hello");
    EXPECT_EQ(et.text(), "Hello");
    EXPECT_FLOAT_EQ(et.x(), 0);
    EXPECT_FLOAT_EQ(et.y(), 0);
    EXPECT_FLOAT_EQ(et.font_size(), 8);
    EXPECT_EQ(et.style(), "");
}

TEST(ExtraTextTest, WithPosition) {
    ExtraText et("100,200:Hello");
    EXPECT_EQ(et.text(), "Hello");
    EXPECT_FLOAT_EQ(et.x(), 100);
    EXPECT_FLOAT_EQ(et.y(), 200);
    EXPECT_FLOAT_EQ(et.font_size(), 8);
    EXPECT_EQ(et.style(), "");
}

TEST(ExtraTextTest, WithSize) {
    ExtraText et("24:World");
    EXPECT_EQ(et.text(), "World");
    EXPECT_FLOAT_EQ(et.x(), 0);
    EXPECT_FLOAT_EQ(et.y(), 0);
    EXPECT_FLOAT_EQ(et.font_size(), 24);
    EXPECT_EQ(et.style(), "");
}

TEST(ExtraTextTest, WithStyleBold) {
    ExtraText et("b:Greetings");
    EXPECT_EQ(et.text(), "Greetings");
    EXPECT_FLOAT_EQ(et.x(), 0);
    EXPECT_FLOAT_EQ(et.y(), 0);
    EXPECT_FLOAT_EQ(et.font_size(), 8);
    EXPECT_EQ(et.style(), "Bold");
}

TEST(ExtraTextTest, WithStyleItalic) {
    ExtraText et("i:ItalicText");
    EXPECT_EQ(et.text(), "ItalicText");
    EXPECT_EQ(et.style(), "Oblique");
}

TEST(ExtraTextTest, WithStyleBoldItalic) {
    ExtraText et("bi:BoldItalic");
    EXPECT_EQ(et.text(), "BoldItalic");
    EXPECT_EQ(et.style(), "BoldOblique");
}

TEST(ExtraTextTest, WithStyleItalicBold) {
    ExtraText et("ib:ItalicBold");
    EXPECT_EQ(et.text(), "ItalicBold");
    EXPECT_EQ(et.style(), "BoldOblique");
}

TEST(ExtraTextTest, FullFormat) {
    ExtraText et("100,200:24:b:Hello World");
    EXPECT_EQ(et.text(), "Hello World");
    EXPECT_FLOAT_EQ(et.x(), 100);
    EXPECT_FLOAT_EQ(et.y(), 200);
    EXPECT_FLOAT_EQ(et.font_size(), 24);
    EXPECT_EQ(et.style(), "Bold");
}

TEST(ExtraTextTest, PositionAndSize) {
    ExtraText et("50,75:12:SomeText");
    EXPECT_EQ(et.text(), "SomeText");
    EXPECT_FLOAT_EQ(et.x(), 50);
    EXPECT_FLOAT_EQ(et.y(), 75);
    EXPECT_FLOAT_EQ(et.font_size(), 12);
    EXPECT_EQ(et.style(), "");
}

TEST(ExtraTextTest, PositionAndStyle) {
    ExtraText et("10,20:i:StyledText");
    EXPECT_EQ(et.text(), "StyledText");
    EXPECT_FLOAT_EQ(et.x(), 10);
    EXPECT_FLOAT_EQ(et.y(), 20);
    EXPECT_FLOAT_EQ(et.font_size(), 8);
    EXPECT_EQ(et.style(), "Oblique");
}

TEST(ExtraTextTest, SizeAndStyle) {
    ExtraText et("36:bi:BigBoldItalic");
    EXPECT_EQ(et.text(), "BigBoldItalic");
    EXPECT_FLOAT_EQ(et.x(), 0);
    EXPECT_FLOAT_EQ(et.y(), 0);
    EXPECT_FLOAT_EQ(et.font_size(), 36);
    EXPECT_EQ(et.style(), "BoldOblique");
}

TEST(ExtraTextTest, PositionWithDecimal) {
    ExtraText et("100.5,200.75:Hello");
    EXPECT_EQ(et.text(), "Hello");
    EXPECT_FLOAT_EQ(et.x(), 100.5f);
    EXPECT_FLOAT_EQ(et.y(), 200.75f);
}

TEST(ExtraTextTest, TextWithColon) {
    ExtraText et("Hello:World");
    EXPECT_EQ(et.text(), "World");
}

TEST(ExtraTextTest, EmptyText) {
    ExtraText et("");
    EXPECT_EQ(et.text(), "");
    EXPECT_FLOAT_EQ(et.x(), 0);
    EXPECT_FLOAT_EQ(et.y(), 0);
    EXPECT_FLOAT_EQ(et.font_size(), 8);
    EXPECT_EQ(et.style(), "");
}

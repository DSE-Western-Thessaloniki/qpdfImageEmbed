#include <gtest/gtest.h>
#include <boost/program_options.hpp>
#include <cstdlib>
#include <string>

#include "../src/logger.h"
#include "../src/options.h"

Logger logger;

// Death tests: invalid invocations should exit with error codes.
// These run in forked processes, so they don't affect the test runner.

TEST(OptionsDeathTest, NoArguments) {
    int argc = 1;
    const char *argv[] = {"qpdfImageEmbed", nullptr};
    EXPECT_EXIT(readCLIOptions(argc, const_cast<char **>(argv)),
                testing::ExitedWithCode(1), "");
}

TEST(OptionsDeathTest, HelpFlag) {
    int argc = 2;
    const char *argv[] = {"qpdfImageEmbed", "--help", nullptr};
    EXPECT_EXIT(readCLIOptions(argc, const_cast<char **>(argv)),
                testing::ExitedWithCode(1), "");
}

TEST(OptionsDeathTest, MissingContentOptions) {
    int argc = 5;
    const char *argv[] = {"qpdfImageEmbed", "-i", "in.pdf", "-o", "out.pdf",
                          nullptr};
    EXPECT_EXIT(readCLIOptions(argc, const_cast<char **>(argv)),
                testing::ExitedWithCode(1), "");
}

TEST(OptionsDeathTest, InvalidRotate) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",     "in.pdf", "-o",
                          "out.pdf",        "--qr",   "test",   "--rotate",
                          "45",             nullptr};
    EXPECT_EXIT(readCLIOptions(argc, const_cast<char **>(argv)),
                testing::ExitedWithCode(2), "");
}

TEST(OptionsDeathTest, ImgSideOutOfRange) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",   "in.pdf", "-o",
                          "out.pdf",        "--qr", "test",   "--img-side",
                          "3",              nullptr};
    EXPECT_EXIT(readCLIOptions(argc, const_cast<char **>(argv)),
                testing::ExitedWithCode(2), "");
}

TEST(OptionsDeathTest, ImgXWithoutY) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed",  "-i",      "in.pdf",
                          "-o",              "out.pdf",
                          "--stamp",         "img.png",
                          "--img-x",         "100",     nullptr};
    EXPECT_EXIT(readCLIOptions(argc, const_cast<char **>(argv)),
                testing::ExitedWithCode(2), "");
}

TEST(OptionsDeathTest, AbsoluteAndRelativePositionConflict) {
    int argc = 13;
    const char *argv[] = {"qpdfImageEmbed",    "-i",         "in.pdf",
                          "-o",                "out.pdf",    "--stamp",
                          "img.png",           "--img-x",    "100",
                          "--img-y",           "200",
                          "--img-top-margin",  "10",         nullptr};
    EXPECT_EXIT(readCLIOptions(argc, const_cast<char **>(argv)),
                testing::ExitedWithCode(2), "");
}

TEST(OptionsDeathTest, NegativeScale) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",     "in.pdf",
                          "-o",             "out.pdf", "--qr",
                          "test",           "--img-scale", "-1", nullptr};
    EXPECT_EXIT(readCLIOptions(argc, const_cast<char **>(argv)),
                testing::ExitedWithCode(2), "");
}

TEST(OptionsTest, ValidQRInvocation) {
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "-i", "in.pdf", "-o",
                          "out.pdf",        "--qr", "test", nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv));
    EXPECT_EQ(std::get<std::string>(opts["inputPDF"]), "in.pdf");
    EXPECT_EQ(std::get<std::string>(opts["outputPDF"]), "out.pdf");
    EXPECT_EQ(std::get<std::string>(opts["qrText"]), "test");
    EXPECT_FLOAT_EQ(std::get<float>(opts["qr-scale"]), 1.0f);
    EXPECT_FLOAT_EQ(std::get<float>(opts["img-scale"]), 1.0f);
}

TEST(OptionsDeathTest, QrSideOutOfRange) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",    "in.pdf", "-o",
                          "out.pdf",        "--qr",  "test",   "--qr-side",
                          "3",              nullptr};
    EXPECT_EXIT(readCLIOptions(argc, const_cast<char **>(argv)),
                testing::ExitedWithCode(2), "");
}

// Validation should rely on content options, not argc.
// These tests verify that the minimum-argc heuristic (argc < 4) is not needed
// and that content-based validation catches all invalid cases.

TEST(OptionsTest, ValidStampOnly) {
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "-i", "in.pdf", "-o",
                          "out.pdf",        "--stamp", "img.png", nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv));
    EXPECT_EQ(std::get<std::string>(opts["inputPDF"]), "in.pdf");
    EXPECT_EQ(std::get<std::string>(opts["outputPDF"]), "out.pdf");
    EXPECT_EQ(std::get<std::string>(opts["imageFile"]), "img.png");
}

TEST(OptionsTest, ValidAddTextOnly) {
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "-i", "in.pdf", "-o",
                          "out.pdf",        "--add-text", "hello", nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv));
    EXPECT_EQ(std::get<std::string>(opts["inputPDF"]), "in.pdf");
    EXPECT_EQ(std::get<std::string>(opts["outputPDF"]), "out.pdf");
    auto text = std::get<std::vector<std::string>>(opts["text"]);
    ASSERT_EQ(text.size(), 1);
    EXPECT_EQ(text[0], "hello");
}

TEST(OptionsTest, MinimumArgcWithContent) {
    // argc=7 is the minimum for a valid invocation: prog -i file -o file --qr text
    // The argc < 4 heuristic should never trigger for valid calls.
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "-i", "a.pdf", "-o",
                          "b.pdf",          "--qr", "data", nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv));
    EXPECT_EQ(std::get<std::string>(opts["qrText"]), "data");
}

TEST(OptionsDeathTest, MissingContentCaughtWithoutArgcCheck) {
    // Even with argc >= 4, missing content must be rejected.
    // This validates that the content check (not argc) is the real guard.
    int argc = 5;
    const char *argv[] = {"qpdfImageEmbed", "-i", "in.pdf", "-o", "out.pdf",
                          nullptr};
    EXPECT_EXIT(readCLIOptions(argc, const_cast<char **>(argv)),
                testing::ExitedWithCode(1), "");
}

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

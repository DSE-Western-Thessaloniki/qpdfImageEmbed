#include <boost/program_options.hpp>
#include <cstdlib>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

#include "../src/logger.h"
#include "../src/options.h"

Logger logger;

TEST(OptionsInvalidTest, NoArguments) {
    int argc = 1;
    const char *argv[] = {"qpdfImageEmbed", nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsTest, HelpFlag) {
    int argc = 2;
    const char *argv[] = {"qpdfImageEmbed", "--help", nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_TRUE(opts.empty());
}

TEST(OptionsTest, VersionFlag) {
    int argc = 2;
    const char *argv[] = {"qpdfImageEmbed", "--version", nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_TRUE(opts.contains("version"));
    EXPECT_EQ(std::get<int>(opts["version"]), 1);
    EXPECT_FALSE(opts.contains("inputPDF"));
}

TEST(OptionsInvalidTest, MissingContentOptions) {
    int argc = 5;
    const char *argv[] = {"qpdfImageEmbed", "-i",   "in.pdf", "-o",
                          "out.pdf",        nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsInvalidTest, InvalidRotate) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",   "in.pdf",   "-o", "out.pdf",
                          "--qr",           "test", "--rotate", "45", nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsInvalidTest, ImgSideOutOfRange) {
    int argc = 9;
    const char *argv[] = {
        "qpdfImageEmbed", "-i",   "in.pdf",     "-o", "out.pdf",
        "--qr",           "test", "--img-side", "3",  nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsInvalidTest, ImgXWithoutY) {
    int argc = 9;
    const char *argv[] = {
        "qpdfImageEmbed", "-i",      "in.pdf",  "-o",  "out.pdf",
        "--stamp",        "img.png", "--img-x", "100", nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsInvalidTest, AbsoluteAndRelativePositionConflict) {
    int argc = 13;
    const char *argv[] = {"qpdfImageEmbed",
                          "-i",
                          "in.pdf",
                          "-o",
                          "out.pdf",
                          "--stamp",
                          "img.png",
                          "--img-x",
                          "100",
                          "--img-y",
                          "200",
                          "--img-top-margin",
                          "10",
                          nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsInvalidTest, NegativeScale) {
    int argc = 9;
    const char *argv[] = {
        "qpdfImageEmbed", "-i",   "in.pdf",      "-o", "out.pdf",
        "--qr",           "test", "--img-scale", "-1", nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsTest, ValidQRInvocation) {
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "-i",   "in.pdf", "-o",
                           "out.pdf",        "--qr", "test",   nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<std::string>(opts["inputPDF"]), "in.pdf");
    EXPECT_EQ(std::get<std::string>(opts["outputPDF"]), "out.pdf");
    EXPECT_EQ(std::get<std::string>(opts["qrText"]), "test");
    EXPECT_FLOAT_EQ(std::get<float>(opts["qr-scale"]), 1.0f);
    EXPECT_FLOAT_EQ(std::get<float>(opts["img-scale"]), 1.0f);
    // Default ECC level should be M (1)
    EXPECT_EQ(std::get<int>(opts["qr-ecc"]), 1);
    // Default colors
    EXPECT_EQ(std::get<std::string>(opts["qr-fg-color"]), "black");
    EXPECT_EQ(std::get<std::string>(opts["qr-bg-color"]), "white");
    // Default opacity
    EXPECT_FLOAT_EQ(std::get<float>(opts["img-opacity"]), 1.0f);
}

TEST(OptionsTest, QrEccLevelL) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",       "in.pdf", "-o",
                           "out.pdf",        "--qr",     "test",   "--qr-ecc",
                           "L",              nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<int>(opts["qr-ecc"]), 0);
}

TEST(OptionsTest, QrEccLevelM) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",       "in.pdf", "-o",
                           "out.pdf",        "--qr",     "test",   "--qr-ecc",
                           "M",              nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<int>(opts["qr-ecc"]), 1);
}

TEST(OptionsTest, QrEccLevelQ) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",       "in.pdf", "-o",
                           "out.pdf",        "--qr",     "test",   "--qr-ecc",
                           "Q",              nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<int>(opts["qr-ecc"]), 2);
}

TEST(OptionsTest, QrEccLevelH) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",       "in.pdf", "-o",
                           "out.pdf",        "--qr",     "test",   "--qr-ecc",
                           "H",              nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<int>(opts["qr-ecc"]), 3);
}

TEST(OptionsInvalidTest, InvalidQrEccLevel) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",       "in.pdf", "-o",
                           "out.pdf",        "--qr",     "test",   "--qr-ecc",
                           "X",              nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsTest, QrDefaultColors) {
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "-i",   "in.pdf", "-o",
                           "out.pdf",        "--qr", "test",   nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<std::string>(opts["qr-fg-color"]), "black");
    EXPECT_EQ(std::get<std::string>(opts["qr-bg-color"]), "white");
}

TEST(OptionsTest, QrCustomColors) {
    int argc = 11;
    const char *argv[] = {"qpdfImageEmbed", "-i",           "in.pdf", "-o",
                           "out.pdf",        "--qr",         "test",
                           "--qr-fg-color",  "#FF0000",      "--qr-bg-color",
                           "#00FF00",        nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<std::string>(opts["qr-fg-color"]), "#FF0000");
    EXPECT_EQ(std::get<std::string>(opts["qr-bg-color"]), "#00FF00");
}

TEST(OptionsTest, DefaultOpacity) {
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "-i",   "in.pdf", "-o",
                           "out.pdf",        "--qr", "test",   nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_FLOAT_EQ(std::get<float>(opts["img-opacity"]), 1.0f);
}

TEST(OptionsTest, CustomOpacity) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",          "in.pdf", "-o",
                           "out.pdf",        "--qr",        "test",
                           "--img-opacity",  "0.5",         nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_FLOAT_EQ(std::get<float>(opts["img-opacity"]), 0.5f);
}

TEST(OptionsInvalidTest, OpacityOutOfRange) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",          "in.pdf", "-o",
                           "out.pdf",        "--qr",        "test",
                           "--img-opacity",  "1.5",         nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsInvalidTest, OpacityNegative) {
    int argc = 9;
    const char *argv[] = {"qpdfImageEmbed", "-i",          "in.pdf", "-o",
                           "out.pdf",        "--qr",        "test",
                           "--img-opacity",  "-0.1",        nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsInvalidTest, QrSideOutOfRange) {
    int argc = 9;
    const char *argv[] = {
        "qpdfImageEmbed", "-i",   "in.pdf",    "-o", "out.pdf",
        "--qr",           "test", "--qr-side", "3",  nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

// Validation should rely on content options, not argc.
// These tests verify that the minimum-argc heuristic (argc < 4) is not needed
// and that content-based validation catches all invalid cases.

TEST(OptionsTest, ValidStampOnly) {
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "-i",      "in.pdf",  "-o",
                          "out.pdf",        "--stamp", "img.png", nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<std::string>(opts["inputPDF"]), "in.pdf");
    EXPECT_EQ(std::get<std::string>(opts["outputPDF"]), "out.pdf");
    EXPECT_EQ(std::get<std::string>(opts["imageFile"]), "img.png");
}

TEST(OptionsTest, ValidAddTextOnly) {
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "-i",         "in.pdf", "-o",
                          "out.pdf",        "--add-text", "hello",  nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<std::string>(opts["inputPDF"]), "in.pdf");
    EXPECT_EQ(std::get<std::string>(opts["outputPDF"]), "out.pdf");
    auto text = std::get<std::vector<std::string>>(opts["text"]);
    ASSERT_EQ(text.size(), 1);
    EXPECT_EQ(text[0], "hello");
}

TEST(OptionsTest, MinimumArgcWithContent) {
    // argc=7 is the minimum for a valid invocation: prog -i file -o file --qr
    // text The argc < 4 heuristic should never trigger for valid calls.
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "-i",   "a.pdf", "-o",
                          "b.pdf",          "--qr", "data",  nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<std::string>(opts["qrText"]), "data");
}

TEST(OptionsInvalidTest, MissingContentCaughtWithoutArgcCheck) {
    // Even with argc >= 4, missing content must be rejected.
    // This validates that the content check (not argc) is the real guard.
    int argc = 5;
    const char *argv[] = {"qpdfImageEmbed", "-i",   "in.pdf", "-o",
                           "out.pdf",        nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

// ============================================================
// Batch mode option tests
// ============================================================

TEST(OptionsBatchTest, ValidInputDirWithQR) {
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "--input-dir", "/tmp/in",
                          "--output-dir",   "/tmp/out",    "--qr",
                          "test",           nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<std::string>(opts["inputDir"]), "/tmp/in");
    EXPECT_EQ(std::get<std::string>(opts["outputDir"]), "/tmp/out");
    EXPECT_EQ(std::get<std::string>(opts["qrText"]), "test");
}

TEST(OptionsBatchTest, ValidInputDirWithStamp) {
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "--input-dir", "/tmp/in",
                          "--output-dir",   "/tmp/out",    "--stamp",
                          "img.png",        nullptr};
    auto opts = readCLIOptions(argc, const_cast<char **>(argv), logger);
    EXPECT_EQ(std::get<std::string>(opts["inputDir"]), "/tmp/in");
    EXPECT_EQ(std::get<std::string>(opts["outputDir"]), "/tmp/out");
    EXPECT_EQ(std::get<std::string>(opts["imageFile"]), "img.png");
}

TEST(OptionsInvalidTest, BatchModeMissingOutputDir) {
    int argc = 5;
    const char *argv[] = {"qpdfImageEmbed", "--input-dir", "/tmp/in",
                          "--qr",           "test",        nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsInvalidTest, BatchModeMissingInputDir) {
    int argc = 5;
    const char *argv[] = {"qpdfImageEmbed", "--output-dir", "/tmp/out",
                          "--qr",           "test",         nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsInvalidTest, BothSingleAndBatchMode) {
    int argc = 11;
    const char *argv[] = {"qpdfImageEmbed", "-i",           "in.pdf",
                          "-o",             "out.pdf",      "--input-dir",
                          "/tmp/in",        "--output-dir", "/tmp/out",
                          "--qr",           "test",         nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsInvalidTest, BatchModeNoContent) {
    int argc = 5;
    const char *argv[] = {"qpdfImageEmbed", "--input-dir", "/tmp/in",
                          "--output-dir",   "/tmp/out",    nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

TEST(OptionsInvalidTest, NeitherSingleNorBatchMode) {
    int argc = 7;
    const char *argv[] = {"qpdfImageEmbed", "--stamp", "img.png", "--qr",
                          "test",           "--add-text", "hello", nullptr};
    EXPECT_THROW(readCLIOptions(argc, const_cast<char **>(argv), logger),
                 std::runtime_error);
}

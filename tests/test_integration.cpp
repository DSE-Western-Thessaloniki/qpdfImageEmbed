#include <gtest/gtest.h>
#include <qpdf/Buffer.hh>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFWriter.hh>
#include <qrencode.h>
#include <Magick++.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>

#include "../src/imageProvider.h"
#include "../src/pdfProcessor.h"
#include "../src/logger.h"

Logger logger;

class MagickEnvironment : public ::testing::Environment {
public:
    void SetUp() override { Magick::InitializeMagick(nullptr); }
    void TearDown() override { Magick::TerminateMagick(); }
};

// Defined by CMake: path to test data directory and the main binary
#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "tests/data"
#endif
#ifndef QPDF_IMAGE_EMBED_BINARY
#define QPDF_IMAGE_EMBED_BINARY "src/qpdfImageEmbed"
#endif

// ============================================================
// ImageProvider tests
// ============================================================

TEST(ImageProviderTest, LoadFromPPM) {
    std::string path = std::string(TEST_DATA_DIR) + "/test_image.ppm";
    ImageProvider *img = new ImageProvider(path);
    EXPECT_EQ(img->getWidth(), 100);
    EXPECT_EQ(img->getHeight(), 100);
    delete img;
}

TEST(ImageProviderTest, CreateFromQR) {
    QRcode *qr = QRcode_encodeString("qpdfImageEmbed test", 0, QR_ECLEVEL_M,
                                     QR_MODE_8, 1);
    ASSERT_NE(qr, nullptr);
    ImageProvider *img = new ImageProvider(qr);
    EXPECT_EQ(img->getWidth(), qr->width);
    EXPECT_EQ(img->getHeight(), qr->width);
    QRcode_free(qr);
    delete img;
}

// ============================================================
// PDFProcessor tests
// ============================================================

TEST(PDFProcessorTest, OpenBlankPDF) {
    std::string path = std::string(TEST_DATA_DIR) + "/blank.pdf";
    PDFProcessor proc;
    EXPECT_TRUE(proc.open(path));
}

TEST(PDFProcessorTest, OpenAndSave) {
    std::string inPath = std::string(TEST_DATA_DIR) + "/blank.pdf";
    std::string outPath = std::string(TEST_DATA_DIR) + "/_test_output.pdf";

    PDFProcessor proc;
    ASSERT_TRUE(proc.open(inPath));
    proc.save(outPath);

    // Verify output is a non-empty valid PDF
    std::ifstream f(outPath);
    ASSERT_TRUE(f.good());
    f.seekg(0, std::ios::end);
    EXPECT_GT(f.tellg(), 0);
    f.close();

    // Re-open with QPDF to verify it's parseable
    QPDF verify;
    EXPECT_NO_THROW(verify.processFile(outPath.c_str()));

    std::remove(outPath.c_str());
}

TEST(PDFProcessorTest, EmbedImageAndVerify) {
    std::string inPath = std::string(TEST_DATA_DIR) + "/blank.pdf";
    std::string imgPath = std::string(TEST_DATA_DIR) + "/test_image.ppm";
    std::string outPath = std::string(TEST_DATA_DIR) + "/_test_embed.pdf";

    PDFProcessor proc;
    ASSERT_TRUE(proc.open(inPath));

    ImageProvider *img = new ImageProvider(imgPath);
    proc.addImage(img, 0.5f, 10.0f, 10.0f);

    proc.save(outPath);

    // Verify output is valid PDF
    QPDF verify;
    EXPECT_NO_THROW(verify.processFile(outPath.c_str()));

    auto pages = verify.getAllPages();
    ASSERT_EQ(pages.size(), 1);

    std::remove(outPath.c_str());
}

TEST(PDFProcessorTest, EmbedQRAndVerify) {
    std::string inPath = std::string(TEST_DATA_DIR) + "/blank.pdf";
    std::string outPath = std::string(TEST_DATA_DIR) + "/_test_qr.pdf";

    PDFProcessor proc;
    ASSERT_TRUE(proc.open(inPath));

    QRcode *qr = QRcode_encodeString("https://example.com", 0, QR_ECLEVEL_M,
                                     QR_MODE_8, 1);
    ASSERT_NE(qr, nullptr);
    ImageProvider *qrImg = new ImageProvider(qr);
    QRcode_free(qr);

    proc.addImage(qrImg, 1.0f, 10.0f, 10.0f);
    proc.save(outPath);

    QPDF verify;
    EXPECT_NO_THROW(verify.processFile(outPath.c_str()));

    std::remove(outPath.c_str());
}

TEST(PDFProcessorTest, EmbedImageWithLink) {
    std::string inPath = std::string(TEST_DATA_DIR) + "/blank.pdf";
    std::string imgPath = std::string(TEST_DATA_DIR) + "/test_image.ppm";
    std::string outPath = std::string(TEST_DATA_DIR) + "/_test_link.pdf";

    PDFProcessor proc;
    ASSERT_TRUE(proc.open(inPath));

    ImageProvider *img = new ImageProvider(imgPath);
    proc.addImage(img, 0.5f, 10.0f, 10.0f, "https://example.com");

    proc.save(outPath);

    // Verify output is valid and contains annotation
    QPDF verify;
    ASSERT_NO_THROW(verify.processFile(outPath.c_str()));
    auto pages = verify.getAllPages();
    QPDFObjectHandle page = pages.at(0);
    EXPECT_TRUE(page.hasKey("/Annots"));

    std::remove(outPath.c_str());
}

// ============================================================
// CLI end-to-end tests
// ============================================================

class CLITest : public ::testing::Test {
protected:
    std::string inPath;
    std::string imgPath;
    std::string outPath;
    std::string binaryPath;

    void SetUp() override {
        inPath = std::string(TEST_DATA_DIR) + "/blank.pdf";
        imgPath = std::string(TEST_DATA_DIR) + "/test_image.ppm";
        outPath = std::string(TEST_DATA_DIR) + "/_test_cli_output.pdf";
        binaryPath = QPDF_IMAGE_EMBED_BINARY;
    }

    void TearDown() override { std::remove(outPath.c_str()); }

    int runBinary(const std::vector<std::string> &args) {
        std::string cmd = binaryPath;
        for (const auto &a : args) {
            cmd += " " + a;
        }
        cmd += " 2>/dev/null";
        return system(cmd.c_str());
    }
};

TEST_F(CLITest, EmbedImageSuccess) {
    std::vector<std::string> args = {
        "-i", inPath, "-o", outPath, "--stamp", imgPath,
    };
    int ret = runBinary(args);
    EXPECT_EQ(ret, 0);

    std::ifstream f(outPath);
    EXPECT_TRUE(f.good());
}

TEST_F(CLITest, EmbedQRSuccess) {
    std::vector<std::string> args = {
        "-i", inPath, "-o", outPath, "--qr", "test-data",
    };
    int ret = runBinary(args);
    EXPECT_EQ(ret, 0);

    std::ifstream f(outPath);
    EXPECT_TRUE(f.good());
}

TEST_F(CLITest, MissingInput) {
    std::vector<std::string> args = {
        "-i", "/nonexistent/input.pdf", "-o", outPath, "--qr", "test",
    };
    int ret = runBinary(args);
    EXPECT_NE(ret, 0);
}

TEST_F(CLITest, MissingOutput) {
    std::vector<std::string> args = {
        "-i", inPath, "-o", "/nonexistent/output.pdf", "--qr", "test",
    };
    int ret = runBinary(args);
    EXPECT_NE(ret, 0);
}

TEST_F(CLITest, BadRotateValue) {
    std::vector<std::string> args = {
        "-i", inPath, "-o", outPath, "--qr", "test", "--rotate", "45",
    };
    int ret = runBinary(args);
    EXPECT_NE(ret, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new MagickEnvironment);
    return RUN_ALL_TESTS();
}

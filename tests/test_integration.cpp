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
#include <sstream>
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
    ImageProvider *img = new ImageProvider(path, logger);
    EXPECT_EQ(img->getWidth(), 100);
    EXPECT_EQ(img->getHeight(), 100);
    delete img;
}

TEST(ImageProviderTest, LoadFromStdin) {
    std::string path = std::string(TEST_DATA_DIR) + "/test_image.ppm";
    std::ifstream file(path, std::ios::binary);
    ASSERT_TRUE(file.good());
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();

    ImageProvider img(ss, logger);
    EXPECT_EQ(img.getWidth(), 100);
    EXPECT_EQ(img.getHeight(), 100);
}

TEST(ImageProviderTest, CreateFromQR) {
    QRcode *qr = QRcode_encodeString("qpdfImageEmbed test", 0, QR_ECLEVEL_M,
                                     QR_MODE_8, 1);
    ASSERT_NE(qr, nullptr);
    ImageProvider *img = new ImageProvider(qr, logger);
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
    PDFProcessor proc(logger);
    EXPECT_TRUE(proc.open(path));
}

TEST(PDFProcessorTest, OpenAndSave) {
    std::string inPath = std::string(TEST_DATA_DIR) + "/blank.pdf";
    std::string outPath = std::string(TEST_DATA_DIR) + "/_test_output.pdf";

    PDFProcessor proc(logger);
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

    PDFProcessor proc(logger);
    ASSERT_TRUE(proc.open(inPath));

    ImageProvider *img = new ImageProvider(imgPath, logger);
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

    PDFProcessor proc(logger);
    ASSERT_TRUE(proc.open(inPath));

    QRcode *qr = QRcode_encodeString("https://example.com", 0, QR_ECLEVEL_M,
                                     QR_MODE_8, 1);
    ASSERT_NE(qr, nullptr);
    ImageProvider *qrImg = new ImageProvider(qr, logger);
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

    PDFProcessor proc(logger);
    ASSERT_TRUE(proc.open(inPath));

    ImageProvider *img = new ImageProvider(imgPath, logger);
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
// Font name collision tests
// ============================================================

static void createPDFWithFonts(
    const std::string &path,
    const std::vector<std::pair<std::string, std::string>> &fonts) {
    std::string blankPath = std::string(TEST_DATA_DIR) + "/blank.pdf";

    QPDF pdf;
    pdf.processFile(blankPath.c_str());
    auto pages = pdf.getAllPages();
    ASSERT_EQ(pages.size(), 1);
    QPDFObjectHandle page = pages.at(0);

    QPDFObjectHandle resources = page.getKey("/Resources");
    if (!resources.isDictionary()) {
        resources = QPDFObjectHandle::newDictionary();
        page.replaceKey("/Resources", resources);
    }

    QPDFObjectHandle fontsDict = resources.getKey("/Font");
    if (!fontsDict.isDictionary()) {
        fontsDict = QPDFObjectHandle::newDictionary();
        resources.replaceKey("/Font", fontsDict);
    }

    for (const auto &[name, basefont] : fonts) {
        QPDFObjectHandle f = QPDFObjectHandle::parse(
            "<< /Type /Font /Subtype /Type1 /BaseFont /" + basefont + " >>");
        fontsDict.replaceKey(name.c_str(), f);
    }

    QPDFWriter writer(pdf, path.c_str());
    writer.write();
}

TEST(PDFProcessorTest, AddExtraTextNoFontCollision) {
    std::string setupPath =
        std::string(TEST_DATA_DIR) + "/_test_font_setup.pdf";
    std::string outPath =
        std::string(TEST_DATA_DIR) + "/_test_font_no_collision.pdf";

    // Create a PDF with /F2 and /F3 already defined (gap at /F1)
    createPDFWithFonts(setupPath, {{"/F2", "Times-Roman"}, {"/F3", "Courier"}});

    {
        PDFProcessor proc(logger);
        ASSERT_TRUE(proc.open(setupPath));
        proc.addExtraText("Hello", 100, 100, 12, "Helvetica", "");
        proc.save(outPath);
    }

    // Verify: /F1 should be the new font, /F2 and /F3 should be untouched
    {
        QPDF verify;
        verify.processFile(outPath.c_str());
        auto pages = verify.getAllPages();
        ASSERT_EQ(pages.size(), 1);

        QPDFObjectHandle fonts =
            pages.at(0).getKey("/Resources").getKey("/Font");
        ASSERT_TRUE(fonts.isDictionary());

        EXPECT_TRUE(fonts.hasKey("/F1"))
            << "New font must occupy the first free slot /F1";
        EXPECT_TRUE(fonts.hasKey("/F2")) << "/F2 must be preserved";
        EXPECT_TRUE(fonts.hasKey("/F3")) << "/F3 must be preserved";

        // New font must have the right BaseFont
        std::string bf =
            fonts.getKey("/F1").getKey("/BaseFont").getName();
        EXPECT_EQ(bf, "/Helvetica");
    }

    std::remove(setupPath.c_str());
    std::remove(outPath.c_str());
}

TEST(PDFProcessorTest, AddExtraTextExistingFontReused) {
    std::string setupPath =
        std::string(TEST_DATA_DIR) + "/_test_font_reuse_setup.pdf";
    std::string outPath =
        std::string(TEST_DATA_DIR) + "/_test_font_reuse_out.pdf";

    // Create a PDF with /F1 that already has Helvetica
    createPDFWithFonts(setupPath, {{"/F1", "Helvetica"}});

    {
        PDFProcessor proc(logger);
        ASSERT_TRUE(proc.open(setupPath));
        // Adding the same basefont should reuse /F1, not create /F2
        proc.addExtraText("World", 200, 200, 14, "Helvetica", "");
        proc.save(outPath);
    }

    {
        QPDF verify;
        verify.processFile(outPath.c_str());
        auto pages = verify.getAllPages();
        QPDFObjectHandle fonts =
            pages.at(0).getKey("/Resources").getKey("/Font");
        ASSERT_TRUE(fonts.isDictionary());

        // Should still only have /F1 (reused), no /F2 created
        auto keys = fonts.getKeys();
        int count = 0;
        for (const auto &k : keys)
            if (k.substr(0, 2) == "/F")
                count++;
        EXPECT_EQ(count, 1) << "Must reuse existing /F1, not create a new one";

        // The content stream must reference /F1 (not /F12 or similar).
        // Extract the content stream and verify the font reference.
        QPDFObjectHandle contents = pages.at(0).getKey("/Contents");
        ASSERT_FALSE(contents.isNull());
        // Contents may be a stream or an array of streams
        std::string streamData;
        auto extractStream = [&streamData](QPDFObjectHandle s) {
            auto buf = s.getStreamData(qpdf_dl_generalized);
            streamData.append(
                reinterpret_cast<const char*>(buf->getBuffer()),
                buf->getSize());
        };
        if (contents.isStream()) {
            extractStream(contents);
        } else if (contents.isArray()) {
            for (int i = 0; i < contents.getArrayNItems(); i++) {
                QPDFObjectHandle item = contents.getArrayItem(i);
                if (item.isStream()) {
                    extractStream(item);
                }
            }
        }
        // Stream must reference /F1 as a standalone font name
        EXPECT_NE(streamData.find("/F1 "), std::string::npos)
            << "Stream must reference /F1, not a mangled name";
        // Must NOT contain /F12 — the increment should not be appended
        EXPECT_EQ(streamData.find("/F12"), std::string::npos)
            << "Stream must NOT reference /F12 (increment appended to found name)";
    }

    std::remove(setupPath.c_str());
    std::remove(outPath.c_str());
}

TEST(PDFProcessorTest, AddExtraTextMultipleNewFonts) {
    std::string setupPath =
        std::string(TEST_DATA_DIR) + "/_test_font_multi_setup.pdf";
    std::string outPath =
        std::string(TEST_DATA_DIR) + "/_test_font_multi_out.pdf";

    // Start with empty font dictionary
    createPDFWithFonts(setupPath, {});

    {
        PDFProcessor proc(logger);
        ASSERT_TRUE(proc.open(setupPath));
        proc.addExtraText("First", 10, 10, 10, "Helvetica", "");
        proc.addExtraText("Second", 20, 20, 10, "Times-Roman", "");
        proc.addExtraText("Third", 30, 30, 10, "Courier", "");
        proc.save(outPath);
    }

    {
        QPDF verify;
        verify.processFile(outPath.c_str());
        auto pages = verify.getAllPages();
        QPDFObjectHandle fonts =
            pages.at(0).getKey("/Resources").getKey("/Font");
        ASSERT_TRUE(fonts.isDictionary());

        EXPECT_TRUE(fonts.hasKey("/F1"));
        EXPECT_TRUE(fonts.hasKey("/F2"));
        EXPECT_TRUE(fonts.hasKey("/F3"));

        // No extras beyond F1-F3
        auto keys = fonts.getKeys();
        int count = 0;
        for (const auto &k : keys)
            if (k.substr(0, 2) == "/F")
                count++;
        EXPECT_EQ(count, 3);

        // Each font must have the right BaseFont
        EXPECT_EQ(fonts.getKey("/F1").getKey("/BaseFont").getName(),
                  "/Helvetica");
        EXPECT_EQ(fonts.getKey("/F2").getKey("/BaseFont").getName(),
                  "/Times-Roman");
        EXPECT_EQ(fonts.getKey("/F3").getKey("/BaseFont").getName(),
                  "/Courier");
    }

    std::remove(setupPath.c_str());
    std::remove(outPath.c_str());
}

TEST(PDFProcessorTest, AddExtraTextNoFontCollisionWithExisting) {
    std::string setupPath =
        std::string(TEST_DATA_DIR) + "/_test_font_collision_existing.pdf";
    std::string outPath =
        std::string(TEST_DATA_DIR) + "/_test_font_collision_existing_out.pdf";

    // Create a PDF with /F1 (Helvetica) and /F2 (Courier).
    // Adding a NEW font (Times-Roman) must produce /F3, not overwrite.
    createPDFWithFonts(setupPath,
                       {{"/F1", "Helvetica"}, {"/F2", "Courier"}});

    {
        PDFProcessor proc(logger);
        ASSERT_TRUE(proc.open(setupPath));
        proc.addExtraText("New", 50, 50, 10, "Times-Roman", "");
        proc.save(outPath);
    }

    {
        QPDF verify;
        verify.processFile(outPath.c_str());
        auto pages = verify.getAllPages();
        QPDFObjectHandle fonts =
            pages.at(0).getKey("/Resources").getKey("/Font");
        ASSERT_TRUE(fonts.isDictionary());

        EXPECT_TRUE(fonts.hasKey("/F1")) << "/F1 must be preserved";
        EXPECT_TRUE(fonts.hasKey("/F2")) << "/F2 must be preserved";
        EXPECT_TRUE(fonts.hasKey("/F3"))
            << "New font must be added at /F3 without collision";

        EXPECT_EQ(fonts.getKey("/F1").getKey("/BaseFont").getName(),
                  "/Helvetica");
        EXPECT_EQ(fonts.getKey("/F2").getKey("/BaseFont").getName(),
                  "/Courier");
        EXPECT_EQ(fonts.getKey("/F3").getKey("/BaseFont").getName(),
                  "/Times-Roman");
    }

    std::remove(setupPath.c_str());
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

TEST_F(CLITest, EmbedImageFromStdin) {
    std::string cmd = binaryPath + " -i " + inPath + " -o " + outPath +
                      " --stamp - < " + imgPath + " 2>/dev/null";
    int ret = system(cmd.c_str());
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

#include <Magick++.h>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <qrencode.h>
#include <stdexcept>

#include "config.h"
#include "extraText.h"
#include "imageProvider.h"
#include "logger.h"
#include "options.h"
#include "pdfProcessor.h"
#include "rect.h"

namespace fs = std::filesystem;

static int processPDF(
    const std::string &inputPath, const std::string &outputPath,
    const std::unordered_map<std::string,
                             std::variant<std::string, int, float,
                                          std::vector<std::string>>> &cliOption,
    Logger &logger) {

    PDFProcessor pdf_processor(logger);
    try {
        pdf_processor.open(inputPath);
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 4;
    }

    if (cliOption.contains("rotate")) {
        pdf_processor.rotate(std::get<int>(cliOption.at("rotate")));
    }

    int qr_side = 0;
    if (cliOption.contains("qr-side")) {
        qr_side = std::get<int>(cliOption.at("qr-side"));
    }
    pdf_processor.setPosition(qr_side);

    if (cliOption.contains("qrText")) {
        logger << "QR Text: " << std::get<std::string>(cliOption.at("qrText"))
               << "\n";
        QRecLevel eccLevel = QR_ECLEVEL_M;
        if (cliOption.contains("qr-ecc")) {
            eccLevel = static_cast<QRecLevel>(std::get<int>(cliOption.at("qr-ecc")));
        }
        QRcode *qr = QRcode_encodeString(
            std::get<std::string>(cliOption.at("qrText")).c_str(), 0,
            eccLevel, QR_MODE_8, 1);

        if (qr == nullptr) {
            logger << "QRcode_encodeString failed!\n";
            std::cerr << "QRcode_encodeString failed!" << std::endl;
            return 3;
        }

        logger << "QR version: " << qr->version << "\n";
        logger << "QR width: " << qr->width << "\n";

        Magick::Color fgColor(
            std::get<std::string>(cliOption.at("qr-fg-color")));
        Magick::Color bgColor(
            std::get<std::string>(cliOption.at("qr-bg-color")));

        pdf_processor.addImage(new ImageProvider(qr, logger, fgColor, bgColor),
                               std::get<float>(cliOption.at("qr-scale")),
                               std::get<float>(cliOption.at("qr-top-margin")),
                               std::get<float>(cliOption.at("qr-side-margin")),
                               std::get<std::string>(cliOption.at("link")),
                               nullptr,
                               std::get<float>(cliOption.at("img-opacity")));
    }

    int img_side = 0;
    if (cliOption.contains("img-side")) {
        img_side = std::get<int>(cliOption.at("img-side"));
    }
    pdf_processor.setPosition(img_side);

    if (cliOption.contains("imageFile")) {
        const std::string &imageFile =
            std::get<std::string>(cliOption.at("imageFile"));
        ImageProvider *imgProvider;
        if (imageFile == "-") {
            imgProvider = new ImageProvider(std::cin, logger);
        } else {
            imgProvider = new ImageProvider(imageFile, logger);
        }

        float opacity = std::get<float>(cliOption.at("img-opacity"));
        if (cliOption.contains("img-x")) {
            Point p(std::get<float>(cliOption.at("img-x")),
                    std::get<float>(cliOption.at("img-y")));
            pdf_processor.addImage(
                imgProvider, std::get<float>(cliOption.at("img-scale")), 0, 0,
                std::get<std::string>(cliOption.at("img-link-to")), &p,
                opacity);
        } else {
            pdf_processor.addImage(
                imgProvider, std::get<float>(cliOption.at("img-scale")),
                std::get<float>(cliOption.at("img-top-margin")),
                std::get<float>(cliOption.at("img-side-margin")),
                std::get<std::string>(cliOption.at("img-link-to")),
                nullptr, opacity);
        }
    }

    const std::vector<std::string> text_vector =
        std::get<std::vector<std::string>>(cliOption.at("text"));
    for (const auto &text : text_vector) {
        ExtraText parsed_text(text, logger);
        if (text != "") {
            pdf_processor.addExtraText(parsed_text.text(), parsed_text.x(),
                                       parsed_text.y(), parsed_text.font_size(),
                                       "Helvetica", parsed_text.style());
        }
    }

    try {
        pdf_processor.save(outputPath);
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 5;
    }

    return 0;
}

int main(int argc, char *argv[]) {

    std::unordered_map<std::string, std::variant<std::string, int, float,
                                                 std::vector<std::string>>>
        cliOption;

    Logger logger;

    try {
        cliOption = readCLIOptions(argc, argv, logger);
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (cliOption.contains("version")) {
        std::cout << "qpdfImageEmbed v" << qpdfImageEmbed_VERSION_MAJOR << "."
                  << qpdfImageEmbed_VERSION_MINOR << "."
                  << qpdfImageEmbed_VERSION_PATCH << std::endl;
        return 0;
    }

    if (!cliOption.contains("inputPDF") && !cliOption.contains("inputDir")) {
        return 0;
    }

    Magick::InitializeMagick(nullptr);

    int exit_code = 0;

    if (cliOption.contains("inputDir")) {
        const std::string &inputDir =
            std::get<std::string>(cliOption.at("inputDir"));
        const std::string &outputDir =
            std::get<std::string>(cliOption.at("outputDir"));

        fs::create_directories(outputDir);

        for (const auto &entry : fs::directory_iterator(inputDir)) {
            if (!entry.is_regular_file())
                continue;
            const auto &path = entry.path();
            if (path.extension() != ".pdf")
                continue;

            std::string outPath = (fs::path(outputDir) / path.filename()).string();
            logger << "Processing " << path.string() << " -> " << outPath
                   << "\n";

            int ret = processPDF(path.string(), outPath, cliOption, logger);
            if (ret != 0) {
                std::cerr << "Error processing " << path.string() << std::endl;
                exit_code = ret;
            }
        }
    } else {
        exit_code = processPDF(
            std::get<std::string>(cliOption.at("inputPDF")),
            std::get<std::string>(cliOption.at("outputPDF")), cliOption,
            logger);
    }

    Magick::TerminateMagick();
    return exit_code;
}

#include <Magick++.h>
#include <cmath>
#include <iostream>
#include <qrencode.h>
#include <stdexcept>

#include "config.h"
#include "extraText.h"
#include "logger.h"
#include "options.h"
#include "pdfProcessor.h"
#include "rect.h"

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

    if (!cliOption.contains("inputPDF")) {
        return 0;
    }

    Magick::InitializeMagick(nullptr);

    PDFProcessor pdf_processor(logger);
    try {
        pdf_processor.open(std::get<std::string>(cliOption["inputPDF"]));
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 4;
    }

    if (cliOption.contains("rotate")) {
        pdf_processor.rotate(std::get<int>(cliOption["rotate"]));
    }

    int qr_side = 0;
    if (cliOption.contains("qr-side")) {
        qr_side = std::get<int>(cliOption["qr-side"]);
    }
    pdf_processor.setPosition(qr_side);

    if (cliOption.contains("qrText")) {
        logger << "QR Text: " << std::get<std::string>(cliOption["qrText"])
               << "\n";
        QRcode *qr = QRcode_encodeString(
            std::get<std::string>(cliOption["qrText"]).c_str(), 0, QR_ECLEVEL_M,
            QR_MODE_8, 1);

        if (qr == nullptr) {
            logger << "QRcode_encodeString failed!\n";
            std::cerr << "QRcode_encodeString failed!" << std::endl;
            return 3;
        }

        logger << "QR version: " << qr->version << "\n";
        logger << "QR width: " << qr->width << "\n";

        pdf_processor.addImage(new ImageProvider(qr, logger),
                               std::get<float>(cliOption["qr-scale"]),
                               std::get<float>(cliOption["qr-top-margin"]),
                               std::get<float>(cliOption["qr-side-margin"]),
                               std::get<std::string>(cliOption["link"]));
    }

    int img_side = 0;
    if (cliOption.contains("img-side")) {
        img_side = std::get<int>(cliOption["img-side"]);
    }
    pdf_processor.setPosition(img_side);

    if (cliOption.contains("imageFile")) {
        const std::string &imageFile =
            std::get<std::string>(cliOption["imageFile"]);
        ImageProvider *imgProvider;
        if (imageFile == "-") {
            imgProvider = new ImageProvider(std::cin, logger);
        } else {
            imgProvider = new ImageProvider(imageFile, logger);
        }

        if (cliOption.contains("img-x")) {
            Point p(std::get<float>(cliOption["img-x"]),
                    std::get<float>(cliOption["img-y"]));
            pdf_processor.addImage(
                imgProvider, std::get<float>(cliOption["img-scale"]), 0, 0,
                std::get<std::string>(cliOption["img-link-to"]), &p);
        } else {
            pdf_processor.addImage(
                imgProvider, std::get<float>(cliOption["img-scale"]),
                std::get<float>(cliOption["img-top-margin"]),
                std::get<float>(cliOption["img-side-margin"]),
                std::get<std::string>(cliOption["img-link-to"]));
        }
    }

    // Add extra text if requested
    const std::vector<std::string> text_vector =
        std::get<std::vector<std::string>>(cliOption["text"]);
    for (const auto& text : text_vector) {
        ExtraText parsed_text(text, logger);

        if (text != "") {
            pdf_processor.addExtraText(parsed_text.text(), parsed_text.x(),
                                       parsed_text.y(), parsed_text.font_size(),
                                       "Helvetica", parsed_text.style());
        }
    }

    try {
        pdf_processor.save(std::get<std::string>(cliOption["outputPDF"]));
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        return 5;
    }

    Magick::TerminateMagick();

    return 0;
}

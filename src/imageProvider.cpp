#include "imageProvider.h"
#include "logger.h"
#include <stdexcept>

using namespace MagickCore;

ImageProvider::ImageProvider(int width, int height, Logger &logger)
    : width(width), height(height), m_logger(logger) {}

ImageProvider::ImageProvider(const std::string &filename, Logger &logger)
    : filename(filename), m_logger(logger) {
    try {
        img.read(filename);
    } catch (Magick::Exception &e) {
        throw std::runtime_error(std::string("Cannot read image '") + filename +
                                 "': " + e.what());
    }
    processImage();
}

ImageProvider::ImageProvider(const QRcode *qr, Logger &logger)
    : m_logger(logger) {
    this->qr = qr;

    Magick::Image image(Magick::Geometry(qr->width, qr->width),
                        Magick::Color(Quantum(0), Quantum(0), Quantum(0),
                                      Quantum(QuantumRange)));

    Magick::Color black("black");
    image.fillColor(black);
    image.strokeColor(black);

    int cellSize = 1;

    for (int row = 0; row < qr->width; row++) {
        for (int col = 0; col < qr->width; col++) {
            if ((qr->data[row * qr->width + col] & 1) == 1) {
                int x = col * cellSize;
                int y = row * cellSize;

                image.draw(Magick::DrawablePoint(x, y));
            }
        }
    }

    img = image;
    processImage();
}

ImageProvider::~ImageProvider() {
    delete[] rgbData;
    delete[] alphaData;
}

void ImageProvider::provideStreamData(int objid, int generation,
                                      Pipeline *pipeline) {
    // If we have an empty image
    if (filename.empty() && qr == nullptr) {
        // Paint an orange rectangle
        for (int i = 0; i < width * height; ++i) {
            pipeline->write(QUtil::unsigned_char_pointer("\xff\x7f\x00"), 3);
        }
    } else {
        pipeline->write(rgbData, width * height * 3);
    }
    pipeline->finish();
}

int ImageProvider::getHeight() const { return height; }

int ImageProvider::getWidth() const { return width; }

std::shared_ptr<Buffer> ImageProvider::getAlpha() const { return alphaBuf; }

void ImageProvider::processImage() {
    Magick::Geometry geometry = img.size();
    width = geometry.width();
    height = geometry.height();

    m_logger << "Image width: " << width << "\n";
    m_logger << "Image height: " << height << "\n";

    alphaData = new unsigned char[width * height];
    img.write(0, 0, width, height, "A", StorageType::CharPixel, alphaData);
    alphaBuf = std::make_shared<Buffer>(alphaData, width * height);

    rgbData = new unsigned char[width * height * 3];
    img.write(0, 0, width, height, "RGB", StorageType::CharPixel, rgbData);
}

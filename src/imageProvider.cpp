#include "imageProvider.h"
#include "logger.h"

using namespace MagickCore;

ImageProvider::ImageProvider(int width, int height)
    : width(width), height(height) {}

ImageProvider::ImageProvider(const std::string filename) : filename(filename) {
    try {
        img.read(filename);
    } catch (Magick::Exception &e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        std::cerr << "Does " << filename << " exist?" << std::endl;
        exit(4);
    }
    processImage();
}

ImageProvider::ImageProvider(const QRcode *qr) {
    this->qr = qr;

    Magick::Image image(Magick::Geometry(qr->width, qr->width),
                        Magick::Color(Quantum(0), Quantum(0), Quantum(0),
                                      Quantum(QuantumRange)));

    Magick::Color black("black");
    image.fillColor(black);
    image.strokeColor(black);

    int cellSize = 1; // Size of each QR cell

    for (int row = 0; row < qr->width; row++) {
        for (int col = 0; col < qr->width; col++) {
            if ((qr->data[row * qr->width + col] & 1) == 1) {
                int x = col * cellSize; // Calculate the x-coordinate
                int y = row * cellSize; // Calculate the y-coordinate

                image.draw(Magick::DrawablePoint(x, y));
                // Magick::DrawableRectangle rect(x, y, x + cellSize,
                //                                y + cellSize);
                // image.draw(rect);
            }
        }
    }

    img = image;
    processImage();
}

ImageProvider::~ImageProvider() {}

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

int ImageProvider::getHeight() { return height; }

int ImageProvider::getWidth() { return width; }

Buffer *ImageProvider::getAlpha() { return alphaBuf; }

void ImageProvider::processImage() {
    Magick::Geometry geometry = img.size();
    width = geometry.width();
    height = geometry.height();

    logger << "Image width: " << width << "\n";
    logger << "Image height: " << height << "\n";

    alphaData = new unsigned char[width * height];
    img.write(0, 0, width, height, "A", StorageType::CharPixel, alphaData);
    alphaBuf = new Buffer(alphaData, width * height);

    rgbData = new unsigned char[width * height * 3];
    img.write(0, 0, width, height, "RGB", StorageType::CharPixel, rgbData);
}

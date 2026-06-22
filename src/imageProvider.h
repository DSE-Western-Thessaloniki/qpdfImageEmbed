#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include "logger.h"
#include <Magick++/Image.h>
#include <istream>
#include <memory>
#include <qpdf/Buffer.hh>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>
#include <qrencode.h>

class ImageProvider : public QPDFObjectHandle::StreamDataProvider {
    public:
        ImageProvider(int width, int height, Logger& logger);
        ImageProvider(const std::string& filename, Logger& logger);
        ImageProvider(std::istream& stream, Logger& logger);
        ImageProvider(const QRcode *qr, Logger& logger);
        virtual ~ImageProvider();
        virtual void provideStreamData(int objid, int generation,
                                       Pipeline *pipeline);
        int getWidth() const;
        int getHeight() const;
        std::shared_ptr<Buffer> getAlpha() const;

    private:
        int width;
        int height;
        std::string filename;
        Magick::Image img;
        std::shared_ptr<Buffer> alphaBuf;
        unsigned char *alphaData = nullptr;
        unsigned char *rgbData = nullptr;
        const QRcode *qr = nullptr;
        Logger& m_logger;

        void processImage();
};

#endif // IMAGEPROVIDER_H
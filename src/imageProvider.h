#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <Magick++/Image.h>
#include <memory>
#include <qpdf/Buffer.hh>
#include <qpdf/QPDF.hh>
#include <qpdf/QPDFObjectHandle.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/QUtil.hh>
#include <qrencode.h>

class ImageProvider : public QPDFObjectHandle::StreamDataProvider {
    public:
        ImageProvider(int width, int height);
        ImageProvider(const std::string filename);
        ImageProvider(const QRcode *qr);
        virtual ~ImageProvider();
        virtual void provideStreamData(int objid, int generation,
                                       Pipeline *pipeline);
        int getWidth();
        int getHeight();
        std::shared_ptr<Buffer> getAlpha();

    private:
        int width;
        int height;
        std::string filename;
        Magick::Image img;
        std::shared_ptr<Buffer> alphaBuf;
        unsigned char *alphaData = nullptr;
        unsigned char *rgbData = nullptr;
        const QRcode *qr;

        void processImage();
};

#endif // IMAGEPROVIDER_H
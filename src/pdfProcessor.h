#ifndef PDFPROCESSOR_H
#define PDFPROCESSOR_H

#include "imageProvider.h"
#include "logger.h"
#include "point.h"
#include "rect.h"
#include <qpdf/QPDF.hh>
#include <string>

class PDFProcessor {
    private:
        QPDF m_pdf;
        Rect m_pageRect;
        int m_rotate = 0;
        int m_side = 0;
        QPDFObjectHandle m_xobject;
        QPDFObjectHandle m_firstPage;
        QPDFObjectHandle m_mediabox;
        Logger& m_logger;

        std::string rand_str(int length);
        std::string createNewImageName(const std::string& prefix);
        void createImageStream(ImageProvider *p, const std::string& name);

    public:
        PDFProcessor(Logger& logger);
        ~PDFProcessor();

        bool open(const std::string& filename);
        void rotate(int degrees);
        void setPosition(int side);
        void addImage(ImageProvider *p, float scale, float topMargin,
                      float sideMargin, const std::string& link = "",
                      Point *exactPosition = nullptr);
        void addExtraText(const std::string& text, float x, float y, float font_size,
                          const std::string& basefont, const std::string& style);
        void save(const std::string& filename);
};

#endif // PDFPROCESSOR_H
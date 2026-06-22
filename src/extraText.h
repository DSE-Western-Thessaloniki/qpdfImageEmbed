#ifndef EXTRATEXT_H
#define EXTRATEXT_H

#include "logger.h"
#include <string>

class ExtraText {
    private:
        float m_x = 0, m_y = 0, m_font_size = 8;
        std::string m_text;
        std::string m_style;
        Logger& m_logger;

    public:
        ExtraText(const std::string& text, Logger& logger);
        ~ExtraText();

        float x(void) const;
        float y(void) const;
        float font_size(void) const;
        const std::string& text(void) const;
        const std::string& style(void) const;
};

#endif // EXTRATEXT_H
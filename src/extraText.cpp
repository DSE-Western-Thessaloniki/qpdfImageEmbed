#include "extraText.h"
#include "logger.h"
#include <regex>

ExtraText::ExtraText(const std::string& text, Logger& logger)
    : m_logger(logger) {
    const std::regex re(":");

    std::sregex_token_iterator it{text.begin(), text.end(), re, -1};
    std::vector<std::string> tokenized{it, {}};

    // Additional check to remove empty strings
    tokenized.erase(
        std::remove_if(tokenized.begin(), tokenized.end(),
                       [](std::string const &s) { return s.size() == 0; }),
        tokenized.end());

    if (tokenized.empty()) {
        return;
    }

    const std::regex pos_r("^(\\d+\\.*\\d*),(\\d+\\.*\\d*)$");
    const std::regex size_r("^(\\d+\\.*\\d*)$");
    const std::regex style_r("^([i,b]{1,2})$");

    for (auto token : tokenized) {
        std::smatch match;

        if (std::regex_match(token, match, pos_r)) {
            m_x = std::stof(match[1].str());
            m_y = std::stof(match[2].str());

            m_logger << "ExtraText.(x,y) = (" << m_x << "," << m_y << ")\n";
        }

        if (std::regex_match(token, match, size_r)) {
            m_font_size = std::stof(match[1]);

            m_logger << "ExtraText.font_size = " << m_font_size << "\n";
        }

        if (std::regex_match(token, match, style_r)) {
            if (match[1].str() == "i") {
                m_style = "Oblique";
            }

            if (match[1].str() == "b") {
                m_style = "Bold";
            }

            if (match[1].str() == "bi" || match[1].str() == "ib") {
                m_style = "BoldOblique";
            }

            m_logger << "ExtraText.style = " << m_style << "\n";
        }
    }

    m_text = tokenized.back();
    m_logger << "ExtraText.text = " << m_text << "\n";
}

ExtraText::~ExtraText() {}

float ExtraText::x(void) const { return m_x; }

float ExtraText::y(void) const { return m_y; }

float ExtraText::font_size(void) const { return m_font_size; }

const std::string& ExtraText::style(void) const { return m_style; }

const std::string& ExtraText::text(void) const { return m_text; }
#ifndef LOGGER_H
#define LOGGER_H

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

class Logger {
    public:
        Logger();
        explicit Logger(const std::string& fileName);
        ~Logger();

        void setShowDateTime(bool value);
        void setEnabled(bool value);
        bool enabled(void) const;
        void close(void);
        void open(const std::string& fileName);
        Logger &operator<<(const std::string& text);
        Logger &operator<<(const char *text);
        Logger &operator<<(const int value);
        Logger &operator<<(const bool value);
        Logger &operator<<(const double value);
        Logger &operator<<(const size_t value);

    private:
        std::ofstream *file = nullptr;
        std::ostream *out = nullptr;
        bool m_showDate = true;
        bool m_enabled = false;

        void write(const std::string &value);
};

#endif // LOGGER_H

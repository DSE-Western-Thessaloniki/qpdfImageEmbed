#ifndef OPTIONS_H
#define OPTIONS_H

#include "logger.h"
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

std::unordered_map<std::string, std::variant<std::string, int, float,
                                             std::vector<std::string>>>
readCLIOptions(int argc, char *argv[], Logger& logger);

#endif // OPTIONS_H
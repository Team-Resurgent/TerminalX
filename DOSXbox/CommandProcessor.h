#pragma once

#include "External.h"

#include <string>
#include <vector>

class CommandProcessor
{
public:
    static std::vector<std::string> ParseLine(const std::string& line);
    static std::string Execute(const std::vector<std::string>& args);
};

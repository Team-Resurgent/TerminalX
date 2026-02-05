#pragma once

#include "CommandContext.h"
#include <string>
#include <vector>

class TimeCommand
{
public:
    static bool Matches(const std::string& cmd);
    static std::string Execute(const std::vector<std::string>& args, CommandContext& ctx);
    /** Parse and set time from prompt input (e.g. HH:MM:SS or HH:MM). Empty = keep same. Returns "" or error message. */
    static std::string SetTimeFromString(const std::string& line);
};

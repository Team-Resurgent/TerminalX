#pragma once

#include "CommandContext.h"
#include <string>
#include <vector>

class DateCommand
{
public:
    static bool Matches(const std::string& cmd);
    static std::string Execute(const std::vector<std::string>& args, CommandContext& ctx);
    /** Parse and set date from prompt input (e.g. yy-mm-dd). Empty = keep same. Returns "" or error message. */
    static std::string SetDateFromString(const std::string& line);
};

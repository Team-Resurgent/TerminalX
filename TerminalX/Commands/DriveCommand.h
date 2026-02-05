#pragma once

#include "CommandContext.h"
#include <string>
#include <vector>

class DriveCommand
{
public:
    static bool Matches(const std::vector<std::string>& args);
    static std::string Execute(const std::vector<std::string>& args, CommandContext& ctx);
};

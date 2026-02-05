#pragma once

#include "CommandContext.h"
#include <string>
#include <vector>

class ShutdownCommand
{
public:
    static bool Matches(const std::string& cmd);
    static std::string Execute(const std::vector<std::string>& args, CommandContext& ctx);
};

#pragma once

#include <string>

struct CommandContext
{
    std::string& currentDir;
    explicit CommandContext(std::string& dir) : currentDir(dir) {}
};

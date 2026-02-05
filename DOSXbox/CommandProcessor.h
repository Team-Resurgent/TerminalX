#pragma once

#include "External.h"

#include <string>
#include <vector>

class CommandProcessor
{
public:
    static std::vector<std::string> ParseLine(const std::string& line);
    static std::string Execute(const std::vector<std::string>& args);
    static std::string GetCurrentDir();
    /** Current directory formatted for prompt display (e.g. HDD0-E:\ or HDD0-E:\path\). */
    static std::string GetCurrentDirForPrompt();

    /** Echo state for ECHO ON/OFF (default on). Used when echoing commands. */
    static bool GetEcho();
    static void SetEcho(bool on);
};

#include "CommandProcessor.h"
#include "Commands\CommandContext.h"
#include "Commands\ClsCommand.h"
#include "Commands\ColorCommand.h"
#include "Commands\VerCommand.h"
#include "Commands\HelpCommand.h"
#include "Commands\DirCommand.h"
#include "Commands\CdCommand.h"
#include "Commands\ExitCommand.h"
#include "Commands\DriveCommand.h"
#include "String.h"
#include <cctype>
#include <string>
#include <vector>

namespace
{
    std::string s_currentDir = "HDD0-E\\";
}

static void Trim(std::string& s)
{
    while (!s.empty() && (s[0] == ' ' || s[0] == '\t'))
    {
        s.erase(0, 1);
    }
    while (!s.empty() && (s[s.length() - 1] == ' ' || s[s.length() - 1] == '\t'))
    {
        s.erase(s.length() - 1, 1);
    }
}

std::vector<std::string> CommandProcessor::ParseLine(const std::string& line)
{
    std::vector<std::string> args;
    std::string cur;
    for (size_t i = 0; i <= line.length(); i++)
    {
        char c = (i < line.length()) ? line[i] : ' ';
        if (c == ' ' || c == '\t')
        {
            Trim(cur);
            if (!cur.empty())
            {
                args.push_back(cur);
                cur.clear();
            }
        }
        else
        {
            cur += c;
        }
    }
    Trim(cur);
    if (!cur.empty())
    {
        args.push_back(cur);
    }
    return args;
}

std::string CommandProcessor::Execute(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        return "";
    }

    std::string cmd = String::ToUpper(args[0]);
    CommandContext ctx(s_currentDir);

    if (DriveCommand::Matches(args))
    {
        return DriveCommand::Execute(args, ctx);
    }
    if (ClsCommand::Matches(cmd))
    {
        return ClsCommand::Execute(args, ctx);
    }
    if (ColorCommand::Matches(cmd))
    {
        return ColorCommand::Execute(args, ctx);
    }
    if (VerCommand::Matches(cmd))
    {
        return VerCommand::Execute(args, ctx);
    }
    if (HelpCommand::Matches(cmd))
    {
        return HelpCommand::Execute(args, ctx);
    }
    if (DirCommand::Matches(cmd))
    {
        return DirCommand::Execute(args, ctx);
    }
    if (CdCommand::Matches(cmd))
    {
        return CdCommand::Execute(args, ctx);
    }
    if (ExitCommand::Matches(cmd))
    {
        return ExitCommand::Execute(args, ctx);
    }

    return "Bad command or file name - " + args[0] + "\n";
}

std::string CommandProcessor::GetCurrentDir()
{
    return s_currentDir;
}

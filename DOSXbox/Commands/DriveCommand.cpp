#include "DriveCommand.h"
#include "..\DriveMount.h"
#include "..\String.h"
#include <string>
#include <vector>

bool DriveCommand::Matches(const std::vector<std::string>& args)
{
    if (args.size() != 1)
    {
        return false;
    }
    if (args[0].length() < 1)
    {
        return false;
    }
    return (args[0][args[0].length() - 1] == ':');
}

std::string DriveCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    std::string driveName = args[0].substr(0, args[0].length() - 1);
    if (driveName.empty())
    {
        return "The system cannot find the drive specified.\n";
    }
    if (!DriveMount::Mount(driveName.c_str()))
    {
        return "The system cannot find the drive specified.\n";
    }
    ctx.currentDir = String::ToUpper(driveName);
    if (ctx.currentDir[ctx.currentDir.length() - 1] != '\\')
    {
        ctx.currentDir += "\\";
    }
    return "";
}

#include "CdCommand.h"
#include "..\DriveMount.h"
#include <string>
#include <vector>

bool CdCommand::Matches(const std::string& cmd)
{
    return (cmd == "CD" || cmd == "CHDIR");
}

std::string CdCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    if (args.size() < 2)
    {
        return ctx.currentDir + "\n";
    }
    std::string path = args[1];
    if (path == "..")
    {
        if (ctx.currentDir.length() > 3)
        {
            size_t p = ctx.currentDir.find_last_of("\\/", ctx.currentDir.length() - 2);
            ctx.currentDir = (p != std::string::npos && p >= 2) ? ctx.currentDir.substr(0, p + 1) : "C:\\";
        }
    }
    else if (path.length() >= 2 && path[1] == ':')
    {
        std::string driveName = path.substr(0, path.find(':'));
        DriveMount::Mount(driveName.c_str());
        ctx.currentDir = path;
        if (ctx.currentDir[ctx.currentDir.length() - 1] != '\\')
        {
            ctx.currentDir += '\\';
        }
    }
    else if (!path.empty())
    {
        if (ctx.currentDir[ctx.currentDir.length() - 1] != '\\')
        {
            ctx.currentDir += '\\';
        }
        ctx.currentDir += path;
        if (ctx.currentDir[ctx.currentDir.length() - 1] != '\\')
        {
            ctx.currentDir += '\\';
        }
    }
    return "";
}

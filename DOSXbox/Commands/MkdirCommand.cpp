#include "MkdirCommand.h"
#include "..\DriveMount.h"
#include "..\FileSystem.h"
#include "..\String.h"
#include <string>
#include <vector>

bool MkdirCommand::Matches(const std::string& cmd)
{
    return (cmd == "MD" || cmd == "MKDIR");
}

std::string MkdirCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    if (args.size() < 2)
    {
        return "The syntax of the command is incorrect.\n";
    }
    std::string pathArg = args[1];
    if (pathArg == "/?" || pathArg == "-?" || pathArg.find('?') != std::string::npos)
    {
        return "Creates a directory.\n\n"
               "MKDIR [drive:]path\n"
               "MD    [drive:]path\n\n"
               "  [drive:]path  Specifies drive and/or path for the new directory.\n\n"
               "Creates any intermediate directories in the path, if needed.\n"
               "Example: mkdir HDD0-E:\\a\\b\\c\\d\n";
    }
    std::string path = ctx.currentDir;
    size_t colon = pathArg.find(':');
    if (colon != std::string::npos)
    {
        std::string drivePart = String::ToUpper(pathArg.substr(0, colon));
        std::string pathPart = pathArg.substr(colon + 1);
        while (!pathPart.empty() && (pathPart[0] == '\\' || pathPart[0] == '/'))
        {
            pathPart.erase(0, 1);
        }
        if (!drivePart.empty())
        {
            if (!DriveMount::Mount(drivePart.c_str()))
            {
                return "The system cannot find the drive specified.\n";
            }
            path = drivePart + "\\";
            if (!pathPart.empty())
            {
                path += pathPart;
                if (path[path.length() - 1] != '\\')
                {
                    path += "\\";
                }
            }
        }
    }
    else if (!pathArg.empty() && pathArg != "." && pathArg != "..")
    {
        if (path.length() > 0 && path[path.length() - 1] != '\\')
        {
            path += "\\";
        }
        path += pathArg;
        if (path.length() > 0 && path[path.length() - 1] != '\\')
        {
            path += "\\";
        }
    }
    else
    {
        return "The syntax of the command is incorrect.\n";
    }
    return FileSystem::CreateDirectory(path);
}

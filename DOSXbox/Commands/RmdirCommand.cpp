#include "RmdirCommand.h"
#include "..\DriveMount.h"
#include "..\FileSystem.h"
#include "..\String.h"
#include <string>
#include <vector>

bool RmdirCommand::Matches(const std::string& cmd)
{
    return (cmd == "RD" || cmd == "RMDIR");
}

std::string RmdirCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    if (args.size() < 2)
    {
        return "The syntax of the command is incorrect.\n";
    }
    bool removeTree = false;
    bool quiet = false;
    std::string pathArg;
    for (size_t i = 1; i < args.size(); i++)
    {
        std::string a = args[i];
        if (a.length() >= 1 && (a[0] == '/' || a[0] == '-'))
        {
            std::string sw = String::ToUpper(a.length() >= 2 ? a.substr(1, 1) : "");
            if (sw == "S")
            {
                removeTree = true;
            }
            else if (sw == "Q")
            {
                quiet = true;
            }
            else if (a == "/?" || a == "-?" || a.find('?') != std::string::npos)
            {
                return "Removes (deletes) a directory.\n\n"
                       "RMDIR [/S] [/Q] [drive:]path\n"
                       "RD    [/S] [/Q] [drive:]path\n\n"
                       "  /S      Removes all directories and files in the specified directory\n"
                       "          in addition to the directory itself.  Used to remove a directory tree.\n\n"
                       "  /Q      Quiet mode, do not ask if ok to remove a directory tree with /S.\n\n"
                       "  [drive:]path  Specifies drive and/or path of the directory to remove.\n";
            }
        }
        else if (pathArg.empty() && !a.empty())
        {
            pathArg = a;
        }
    }
    (void)quiet;
    if (pathArg.empty())
    {
        return "The syntax of the command is incorrect.\n";
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
    else if (pathArg != "." && pathArg != "..")
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
    return FileSystem::RemoveDir(path, removeTree);
}

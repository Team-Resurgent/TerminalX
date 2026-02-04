#include "DirCommand.h"
#include "..\DriveMount.h"
#include "..\FileSystem.h"
#include "..\String.h"
#include <string>
#include <vector>

bool DirCommand::Matches(const std::string& cmd)
{
    return (cmd == "DIR");
}

std::string DirCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    std::string pathArg;
    bool showHelp = false;
    DirOptions dirOpts;
    for (size_t i = 1; i < args.size(); i++)
    {
        std::string a = args[i];
        if (a.length() >= 1 && (a[0] == '/' || a[0] == '-'))
        {
            if (a == "/?" || a == "-?" || a.find('?') != std::string::npos)
            {
                showHelp = true;
                break;
            }
            std::string sw = String::ToUpper(a.length() >= 2 ? a.substr(1, 1) : "");
            if (sw == "W")
            {
                dirOpts.wide = true;
            }
            else if (sw == "P")
            {
                dirOpts.pageLines = 23;
            }
            else if (sw == "A")
            {
                if (a.length() >= 3 && (a[2] == ':' || a[2] == ' '))
                {
                    dirOpts.attrib = a.length() > 3 ? String::ToUpper(a.substr(3)) : "";
                }
                else if (a.length() > 2)
                {
                    dirOpts.attrib = String::ToUpper(a.substr(2));
                }
                else if (i + 1 < args.size())
                {
                    dirOpts.attrib = String::ToUpper(args[++i]);
                }
            }
            else if (sw == "O")
            {
                std::string val;
                if (a.length() >= 3 && (a[2] == ':' || a[2] == ' '))
                {
                    val = a.length() > 3 ? a.substr(3) : "";
                }
                else if (a.length() > 2)
                {
                    val = a.substr(2);
                }
                else if (i + 1 < args.size())
                {
                    val = args[++i];
                }
                if (!val.empty())
                {
                    val = String::ToUpper(val);
                    dirOpts.sortReverse = (val[0] == '-');
                    dirOpts.sortBy = (val.length() >= (size_t)(dirOpts.sortReverse ? 2 : 1))
                        ? (char)val[dirOpts.sortReverse ? 1 : 0] : 'N';
                    if (dirOpts.sortBy != 'N' && dirOpts.sortBy != 'D' && dirOpts.sortBy != 'S' && dirOpts.sortBy != 'E')
                    {
                        dirOpts.sortBy = 'N';
                    }
                }
            }
        }
        else if (pathArg.empty() && !a.empty())
        {
            pathArg = a;
        }
    }
    if (showHelp)
    {
        return "Displays a list of files and subdirectories in a directory.\n\n"
               "DIR [drive:][path] [/P] [/W] [/A[:]attributes] [/O[:]sortorder] [/?]\n\n"
               "  [drive:][path]  Specifies drive and/or directory to list.\n\n"
               "  /P              Pauses after each screenful (inserts --- More ---).\n"
               "  /W              Uses wide list format.\n"
               "  /A[:]attributes D=Dir R=Read-only H=Hidden A=Archive S=System; - prefix excludes.\n"
               "  /O[:]sortorder  N=name D=date S=size E=extension; - prefix reverses.\n"
               "  /?              Displays this help.\n";
    }
    std::string path = ctx.currentDir;
    if (!pathArg.empty())
    {
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
                if (!pathPart.empty() && pathPart != "." && pathPart != "..")
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
    }
    return FileSystem::ListDirectory(path, dirOpts);
}

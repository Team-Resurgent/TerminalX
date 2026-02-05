#include "DelCommand.h"
#include "..\DriveMount.h"
#include "..\FileSystem.h"
#include "..\String.h"
#include <string>
#include <vector>

static bool IsSwitch(const std::string& a)
{
    return (a.length() >= 1 && (a[0] == '/' || a[0] == '-'));
}

static void ResolvePath(const std::string& pathArg, const std::string& currentDir, std::string& outPath)
{
    outPath = currentDir;
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
            DriveMount::Mount(drivePart.c_str());
            outPath = drivePart + "\\";
            if (!pathPart.empty() && pathPart != "." && pathPart != "..")
            {
                outPath += pathPart;
            }
        }
    }
    else if (!pathArg.empty() && pathArg != "." && pathArg != "..")
    {
        if (outPath.length() > 0 && outPath[outPath.length() - 1] != '\\')
        {
            outPath += "\\";
        }
        outPath += pathArg;
    }
    while (outPath.length() > 0 && (outPath[outPath.length() - 1] == '\\' || outPath[outPath.length() - 1] == '/'))
    {
        outPath.erase(outPath.length() - 1, 1);
    }
    if (outPath.empty() && currentDir.length() > 0)
    {
        outPath = currentDir;
        while (outPath.length() > 0 && (outPath[outPath.length() - 1] == '\\' || outPath[outPath.length() - 1] == '/'))
        {
            outPath.erase(outPath.length() - 1, 1);
        }
    }
}

bool DelCommand::Matches(const std::string& cmd)
{
    return (cmd == "DEL" || cmd == "ERASE");
}

std::string DelCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    if (args.size() < 2)
    {
        return "The syntax of the command is incorrect.\n";
    }
    bool prompt = false;   /* /P not implemented */
    bool force = false;   /* /F */
    bool recursive = false; /* /S */
    bool quiet = false;   /* /Q */
    std::string attribFilter;
    std::vector<std::string> names;

    for (size_t i = 1; i < args.size(); i++)
    {
        std::string a = args[i];
        if (IsSwitch(a))
        {
            if (a == "/?" || a == "-?" || a.find('?') != std::string::npos)
            {
                return "Deletes one or more files.\n\n"
                       "DEL [/P] [/F] [/S] [/Q] [/A[[:]attributes]] names\n"
                       "ERASE [/P] [/F] [/S] [/Q] [/A[[:]attributes]] names\n\n"
                       "  names         Specifies a list of one or more files or directories.\n"
                       "                Wildcards may be used to delete multiple files. If a\n"
                       "                directory is specified, all files within the directory\n"
                       "                will be deleted.\n\n"
                       "  /P            Prompts for confirmation before deleting each file (not implemented).\n"
                       "  /F            Force deleting of read-only files.\n"
                       "  /S            Delete specified files from all subdirectories.\n"
                       "  /Q            Quiet mode (no prompt on global wildcard).\n"
                       "  /A            Selects files to delete based on attributes.\n"
                       "  attributes    R Read-only  S System  H Hidden  A Archive  D Directory; - prefix excludes.\n";
            }
            std::string sw = String::ToUpper(a);
            if (sw == "/P" || sw == "-P")
            {
                prompt = true;
            }
            else if (sw == "/F" || sw == "-F")
            {
                force = true;
            }
            else if (sw == "/S" || sw == "-S")
            {
                recursive = true;
            }
            else if (sw == "/Q" || sw == "-Q")
            {
                quiet = true;
            }
            else if ((sw.length() >= 2 && sw[1] == 'A') || (sw.length() >= 2 && sw.substr(0, 2) == "-A"))
            {
                if (a.length() >= 3 && (a[2] == ':' || a[2] == ' '))
                {
                    attribFilter = a.length() > 3 ? String::ToUpper(a.substr(3)) : "";
                }
                else if (a.length() > 2)
                {
                    attribFilter = String::ToUpper(a.substr(2));
                }
                else if (i + 1 < args.size())
                {
                    attribFilter = String::ToUpper(args[++i]);
                }
            }
        }
        else
        {
            names.push_back(a);
        }
    }

    if (names.empty())
    {
        return "The syntax of the command is incorrect.\n";
    }

    (void)prompt;
    (void)quiet;

    std::string result;
    bool showOnlyDeleted = recursive; /* /S: show only the files that are deleted */
    for (size_t i = 0; i < names.size(); i++)
    {
        std::string resolved;
        ResolvePath(names[i], ctx.currentDir, resolved);
        if (resolved.empty())
        {
            result = "The syntax of the command is incorrect.\n";
            break;
        }
        std::string err = FileSystem::DeletePath(resolved, recursive, force, attribFilter, showOnlyDeleted);
        if (!err.empty())
        {
            bool isError = (err.find("The syntax") != std::string::npos ||
                            err.find("Could Not Find") != std::string::npos ||
                            err.find("Access is denied") != std::string::npos ||
                            err.find("The system cannot find") != std::string::npos);
            if (isError)
            {
                result = err;
                break;
            }
            result += err; /* list of deleted paths when /S */
        }
    }
    return result;
}

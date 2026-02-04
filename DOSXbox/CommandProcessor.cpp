#include "CommandProcessor.h"
#include "DriveMount.h"
#include <algorithm>
#include <cctype>

namespace
{
    std::string s_currentDir = "HDD0-E\\";
}

static std::string ToUpper(const std::string& s)
{
    std::string r = s;
    for (size_t i = 0; i < r.length(); i++)
        r[i] = (char)toupper((unsigned char)r[i]);
    return r;
}

static void Trim(std::string& s)
{
    while (!s.empty() && (s[0] == ' ' || s[0] == '\t'))
        s.erase(0, 1);
    while (!s.empty() && (s[s.length() - 1] == ' ' || s[s.length() - 1] == '\t'))
        s.erase(s.length() - 1, 1);
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
            cur += c;
    }
    Trim(cur);
    if (!cur.empty())
        args.push_back(cur);
    return args;
}

std::string CommandProcessor::Execute(const std::vector<std::string>& args)
{
    if (args.empty())
        return "";

    std::string cmd = ToUpper(args[0]);

    if (cmd == "CLS")
    {
        return "\x01CLS";  /* special: caller clears screen */
    }
    if (cmd == "VER")
    {
        return "Microsoft Windows XP [Version 5.1.2600]\n(DOSXbox)\n";
    }
    if (cmd == "HELP")
    {
        return "For more information on a specific command, type HELP command-name\n"
               "DIR    Displays a list of files and subdirectories in a directory.\n"
               "CD     Displays the name of or changes the current directory.\n"
               "CLS    Clears the screen.\n"
               "VER    Displays the Windows version.\n"
               "HELP   Provides Help information for Windows commands.\n"
               "EXIT   Quits the command interpreter.\n";
    }
    if (cmd == "DIR")
    {
        std::string out = " Volume in drive C has no label.\n Volume Serial Number is 0000-0000\n\n Directory of " + s_currentDir + "\n\n";
        out += "01/01/2020  12:00 AM    <DIR>          .\n";
        out += "01/01/2020  12:00 AM    <DIR>          ..\n";
        out += "06/15/2020  02:30 PM            12,345 AUTOEXEC.BAT\n";
        out += "06/15/2020  02:30 PM             8,192 COMMAND.COM\n";
        out += "02/03/2025  10:15 AM    <DIR>          GAMES\n";
        out += "02/03/2025  10:16 AM    <DIR>          APPS\n";
        out += "               2 File(s)         20,537 bytes\n";
        out += "               4 Dir(s)   1,234,567,890 bytes free\n";
        return out;
    }
    if (cmd == "CD" || cmd == "CHDIR")
    {
        if (args.size() < 2)
            return s_currentDir + "\n";
        std::string path = args[1];
        if (path == "..")
        {
            if (s_currentDir.length() > 3)
            {
                size_t p = s_currentDir.find_last_of("\\/", s_currentDir.length() - 2);
                s_currentDir = (p != std::string::npos && p >= 2) ? s_currentDir.substr(0, p + 1) : "C:\\";
            }
        }
        else if (path.length() >= 2 && path[1] == ':')
        {
            std::string driveName = path.substr(0, path.find(':'));
            DriveMount::Mount(driveName.c_str());
            s_currentDir = path;
            if (s_currentDir[s_currentDir.length() - 1] != '\\')
                s_currentDir += '\\';
        }
        else if (!path.empty())
        {
            if (s_currentDir[s_currentDir.length() - 1] != '\\')
                s_currentDir += '\\';
            s_currentDir += path;
            if (s_currentDir[s_currentDir.length() - 1] != '\\')
                s_currentDir += '\\';
        }
        return "";
    }
    if (cmd == "EXIT")
    {
        return "\x02EXIT";  /* special: caller exits app */
    }
    /* Drive change: e.g. HDD0-E: or H: */
    if (args.size() == 1 && args[0].length() >= 1 && args[0][args[0].length() - 1] == ':')
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
        s_currentDir = ToUpper(driveName);
        if (s_currentDir[s_currentDir.length() - 1] != '\\')
        {
            s_currentDir += "\\";
        }
        return "";
    }
    return "Bad command or file name - " + args[0] + "\n";
}

std::string CommandProcessor::GetCurrentDir()
{
    return s_currentDir;
}

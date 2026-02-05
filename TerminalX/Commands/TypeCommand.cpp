#include "TypeCommand.h"
#include "..\DriveMount.h"
#include "..\FileSystem.h"
#include "..\String.h"
#include <string>
#include <vector>
#include <xtl.h>

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif
#ifndef ERROR_PATH_NOT_FOUND
#define ERROR_PATH_NOT_FOUND 3
#endif
#ifndef ERROR_FILE_NOT_FOUND
#define ERROR_FILE_NOT_FOUND 2
#endif

static const size_t TYPE_MAX_SIZE = 65536; /* 64 KB */

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

/** Read file contents and append to out. Returns empty on success, error message otherwise. */
static std::string TypeOneFile(const std::string& path)
{
    if (path.empty())
    {
        return "The syntax of the command is incorrect.\n";
    }
    std::string apiPath = FileSystem::ToApiPath(path);
    DWORD attrs = GetFileAttributesA(apiPath.c_str());
    if (attrs == 0xFFFFFFFF)
    {
        DWORD err = GetLastError();
        if (err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND)
        {
            return "File Not Found\n";
        }
        return "Access is denied.\n";
    }
    if (attrs & FILE_ATTRIBUTE_DIRECTORY)
    {
        return "Access is denied.\n";
    }
    HANDLE h = CreateFileA(apiPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE)
    {
        return "Access is denied.\n";
    }
    DWORD sizeHigh = 0;
    DWORD sizeLow = GetFileSize(h, &sizeHigh);
    if (sizeLow == 0xFFFFFFFF && GetLastError() != NO_ERROR)
    {
        CloseHandle(h);
        return "Access is denied.\n";
    }
    if (sizeHigh != 0 || sizeLow > (DWORD)TYPE_MAX_SIZE)
    {
        CloseHandle(h);
        return "File too large.\n";
    }
    std::string out;
    out.reserve(sizeLow);
    char buf[4096];
    DWORD read = 0;
    while (ReadFile(h, buf, sizeof(buf), &read, NULL) && read > 0)
    {
        for (DWORD i = 0; i < read; i++)
        {
            char c = buf[i];
            if (c == '\0')
            {
                out += ' ';
            }
            else
            {
                out += c;
            }
        }
    }
    CloseHandle(h);
    return out;
}

bool TypeCommand::Matches(const std::string& cmd)
{
    return (cmd == "TYPE");
}

std::string TypeCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    if (args.size() < 2)
    {
        return "The syntax of the command is incorrect.\n";
    }
    std::string result;
    bool hadFileArg = false;
    for (size_t i = 1; i < args.size(); i++)
    {
        std::string a = args[i];
        if (IsSwitch(a))
        {
            if (a == "/?" || a == "-?" || a.find('?') != std::string::npos)
            {
                return "Displays the contents of a text file or files.\n\n"
                       "TYPE [drive:][path]filename\n\n"
                       "  [drive:][path]filename  Specifies the file or files to display.\n";
            }
            continue;
        }
        hadFileArg = true;
        std::string path;
        ResolvePath(a, ctx.currentDir, path);
        result += TypeOneFile(path);
    }
    if (!hadFileArg)
    {
        return "The syntax of the command is incorrect.\n";
    }
    return result;
}

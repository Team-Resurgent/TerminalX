#include "CopyCommand.h"
#include "..\DriveMount.h"
#include "..\FileSystem.h"
#include "..\String.h"
#include <string>
#include <vector>
#include <xtl.h>

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
            if (!pathPart.empty())
            {
                outPath += pathPart;
                if (outPath[outPath.length() - 1] != '\\')
                {
                    outPath += "\\";
                }
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
        if (outPath.length() > 0 && outPath[outPath.length() - 1] != '\\')
        {
            outPath += "\\";
        }
    }
}

static std::string GetPathWithoutTrailingSlash(const std::string& path)
{
    std::string p = path;
    while (p.length() > 0 && (p[p.length() - 1] == '\\' || p[p.length() - 1] == '/'))
    {
        p.erase(p.length() - 1, 1);
    }
    return p;
}

bool CopyCommand::Matches(const std::string& cmd)
{
    return (cmd == "COPY");
}

std::string CopyCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    if (args.size() < 2)
    {
        return "The syntax of the command is incorrect.\n";
    }
    bool overwrite = true;
    std::vector<std::string> pathArgs;
    for (size_t i = 1; i < args.size(); i++)
    {
        std::string a = args[i];
        if (IsSwitch(a))
        {
            if (a == "/?" || a == "-?" || a.find('?') != std::string::npos)
            {
                return "Copies one or more files to another location.\n\n"
                       "COPY [/Y | /-Y] source [+ source [+ ...]] [destination]\n\n"
                       "  source       The file(s) to be copied.\n"
                       "  destination  The directory and/or filename for the new file(s).\n"
                       "  /Y           Suppresses prompting to confirm overwriting (default).\n"
                       "  /-Y          Prompts to confirm overwriting (not implemented).\n\n"
                       "To append files: COPY file1+file2+file3 destination\n";
            }
            std::string sw = String::ToUpper(a);
            if (sw == "/Y" || sw == "-Y")
            {
                overwrite = true;
            }
            else if (sw == "/-Y" || sw == "-/-Y")
            {
                overwrite = false;
            }
        }
        else
        {
            pathArgs.push_back(a);
        }
    }
    if (pathArgs.size() < 2)
    {
        return "The syntax of the command is incorrect.\n";
    }
    std::string destArg = pathArgs[pathArgs.size() - 1];
    std::vector<std::string> sourcePaths;
    for (size_t i = 0; i + 1 < pathArgs.size(); i++)
    {
        const std::string& spec = pathArgs[i];
        for (size_t j = 0; j < spec.length(); )
        {
            size_t plus = spec.find('+', j);
            if (plus == std::string::npos)
            {
                std::string part = spec.substr(j);
                if (!part.empty())
                {
                    sourcePaths.push_back(part);
                }
                break;
            }
            std::string part = spec.substr(j, plus - j);
            if (!part.empty())
            {
                sourcePaths.push_back(part);
            }
            j = plus + 1;
        }
    }
    if (sourcePaths.empty())
    {
        return "The syntax of the command is incorrect.\n";
    }
    std::string destResolved;
    ResolvePath(destArg, ctx.currentDir, destResolved);
    std::string destPath = GetPathWithoutTrailingSlash(destResolved);
    bool destIsDir = FileSystem::IsDirectory(destPath);
    if (sourcePaths.size() == 1 && !destIsDir)
    {
        std::string srcResolved;
        ResolvePath(sourcePaths[0], ctx.currentDir, srcResolved);
        std::string srcPath = GetPathWithoutTrailingSlash(srcResolved);
        return FileSystem::CopyPath(srcPath, destPath, overwrite);
    }
    if (sourcePaths.size() == 1 && destIsDir)
    {
        std::string srcResolved;
        ResolvePath(sourcePaths[0], ctx.currentDir, srcResolved);
        std::string srcPath = GetPathWithoutTrailingSlash(srcResolved);
        size_t slash = srcPath.find_last_of("\\/");
        std::string filename = (slash != std::string::npos) ? srcPath.substr(slash + 1) : srcPath;
        std::string dstPath = destPath + "\\" + filename;
        return FileSystem::CopyPath(srcPath, dstPath, overwrite);
    }
    if (sourcePaths.size() > 1 && destIsDir)
    {
        std::string destDir = destPath;
        if (destDir.length() > 0 && destDir[destDir.length() - 1] != '\\')
        {
            destDir += "\\";
        }
        for (size_t i = 0; i < sourcePaths.size(); i++)
        {
            std::string srcResolved;
            ResolvePath(sourcePaths[i], ctx.currentDir, srcResolved);
            std::string srcPath = GetPathWithoutTrailingSlash(srcResolved);
            size_t slash = srcPath.find_last_of("\\/");
            std::string filename = (slash != std::string::npos) ? srcPath.substr(slash + 1) : srcPath;
            std::string dstPath = GetPathWithoutTrailingSlash(destDir + filename);
            std::string err = FileSystem::CopyPath(srcPath, dstPath, overwrite);
            if (!err.empty())
            {
                return err;
            }
        }
        return "";
    }
    if (sourcePaths.size() > 1 && !destIsDir)
    {
        std::vector<std::string> srcFull;
        for (size_t i = 0; i < sourcePaths.size(); i++)
        {
            std::string srcResolved;
            ResolvePath(sourcePaths[i], ctx.currentDir, srcResolved);
            srcFull.push_back(GetPathWithoutTrailingSlash(srcResolved));
        }
        return FileSystem::AppendFiles(srcFull, destPath);
    }
    return "";
}

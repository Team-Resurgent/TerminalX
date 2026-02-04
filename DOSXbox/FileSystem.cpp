#include "FileSystem.h"
#include "String.h"
#include <xtl.h>
#include <string>
#include <stdio.h>

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#endif

std::string FileSystem::ToApiPath(const std::string& path)
{
    if (path.empty())
    {
        return path;
    }
    size_t p = path.find('\\');
    if (p == std::string::npos)
    {
        return path + ":\\";
    }
    return path.substr(0, p) + ":\\" + path.substr(p + 1);
}

static std::string FormatFileTime(const FILETIME& ft)
{
    SYSTEMTIME st;
    if (!FileTimeToSystemTime(&ft, &st))
    {
        return "01/01/1980  12:00 AM";
    }
    const char* ampm = (st.wHour < 12) ? "AM" : "PM";
    int hour12 = (st.wHour % 12);
    if (hour12 == 0)
    {
        hour12 = 12;
    }
    return String::Format("%02u/%02u/%04u  %2d:%02u %s",
        (unsigned)st.wMonth,
        (unsigned)st.wDay,
        (unsigned)st.wYear,
        hour12,
        (unsigned)st.wMinute,
        ampm);
}

std::string FileSystem::ListDirectory(const std::string& path)
{
    std::string apiPath = ToApiPath(path);
    std::string searchPath = apiPath;
    if (searchPath.length() > 0 && searchPath[searchPath.length() - 1] != '\\')
    {
        searchPath += "\\";
    }
    searchPath += "*";

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(searchPath.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if (err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND)
        {
            return "File Not Found\n";
        }
        return "Error reading directory\n";
    }

    std::string out;
    out += " Volume in drive " + path.substr(0, path.find('\\')) + " has no label.\n";
    out += " Volume Serial Number is 0000-0000\n\n";
    out += " Directory of " + apiPath + "\n\n";

    int dirCount = 0;
    int fileCount = 0;
    unsigned __int64 totalBytes = 0;

    do
    {
        std::string name = fd.cFileName;
        bool isDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        if (isDir)
        {
            dirCount++;
            out += FormatFileTime(fd.ftLastWriteTime) + "    <DIR>          " + name + "\n";
        }
        else
        {
            unsigned __int64 size = ((unsigned __int64)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
            fileCount++;
            totalBytes += size;
            std::string sizeStr;
            if (fd.nFileSizeHigh == 0)
            {
                sizeStr = String::Format("%16u ", (unsigned)fd.nFileSizeLow);
            }
            else
            {
                sizeStr = String::Format("%u%09u ", (unsigned)fd.nFileSizeHigh, (unsigned)fd.nFileSizeLow);
            }
            out += FormatFileTime(fd.ftLastWriteTime) + " " + sizeStr + name + "\n";
        }
    }
    while (FindNextFileA(h, &fd));

    FindClose(h);

    ULARGE_INTEGER freeBytes, totalDisk, freeToCaller;
    freeBytes.QuadPart = 0;
    totalDisk.QuadPart = 0;
    freeToCaller.QuadPart = 0;
    if (GetDiskFreeSpaceExA(apiPath.c_str(), &freeToCaller, &totalDisk, &freeBytes))
    {
        out += String::Format("               %d File(s) ", fileCount);
        if ((totalBytes >> 32) == 0)
        {
            out += String::Format("%u bytes\n", (unsigned)totalBytes);
        }
        else
        {
            out += String::Format("%u%09u bytes\n", (unsigned)(totalBytes >> 32), (unsigned)(totalBytes & 0xFFFFFFFF));
        }
        out += String::Format("               %d Dir(s)  ", dirCount);
        if (freeBytes.QuadPart >> 32)
        {
            out += String::Format("%u%09u bytes free\n", (unsigned)(freeBytes.QuadPart >> 32), (unsigned)(freeBytes.QuadPart & 0xFFFFFFFF));
        }
        else
        {
            out += String::Format("%u bytes free\n", (unsigned)freeBytes.QuadPart);
        }
    }
    else
    {
        out += String::Format("               %d File(s) ", fileCount);
        if ((totalBytes >> 32) == 0)
        {
            out += String::Format("%u bytes\n", (unsigned)totalBytes);
        }
        else
        {
            out += String::Format("%u%09u bytes\n", (unsigned)(totalBytes >> 32), (unsigned)(totalBytes & 0xFFFFFFFF));
        }
        out += String::Format("               %d Dir(s)\n", dirCount);
    }
    return out;
}

bool FileSystem::IsDirectory(const std::string& path)
{
    std::string apiPath = ToApiPath(path);
    DWORD attrs = GetFileAttributesA(apiPath.c_str());
    if (attrs == 0xFFFFFFFF)
    {
        return false;
    }
    return (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool FileSystem::Exists(const std::string& path)
{
    std::string apiPath = ToApiPath(path);
    DWORD attrs = GetFileAttributesA(apiPath.c_str());
    return (attrs != 0xFFFFFFFF);
}

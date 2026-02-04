#include "FileSystem.h"
#include "String.h"
#include <xtl.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <algorithm>

#ifndef FILE_ATTRIBUTE_DIRECTORY
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010
#endif
#ifndef FILE_ATTRIBUTE_READONLY
#define FILE_ATTRIBUTE_READONLY 0x01
#endif
#ifndef FILE_ATTRIBUTE_HIDDEN
#define FILE_ATTRIBUTE_HIDDEN 0x02
#endif
#ifndef FILE_ATTRIBUTE_SYSTEM
#define FILE_ATTRIBUTE_SYSTEM 0x04
#endif
#ifndef FILE_ATTRIBUTE_ARCHIVE
#define FILE_ATTRIBUTE_ARCHIVE 0x20
#endif
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#endif
#ifndef ERROR_ALREADY_EXISTS
#define ERROR_ALREADY_EXISTS 183
#endif
#ifndef ERROR_DIR_NOT_EMPTY
#define ERROR_DIR_NOT_EMPTY 145
#endif

struct DirEntry
{
    std::string name;
    bool isDir;
    unsigned __int64 size;
    FILETIME lastWriteTime;
    DWORD attributes;
};

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

static bool PassesAttributeFilter(const DirEntry& e, const std::string& attrib)
{
    if (attrib.empty())
        return true;
    int mustHave[5] = { 0, 0, 0, 0, 0 }; /* D=0, R=1, H=2, A=3, S=4: 0=don't care, 1=must have, -1=must not have */
    for (size_t i = 0; i < attrib.length(); i++)
    {
        char c = (char)toupper((unsigned char)attrib[i]);
        if (c == '-')
            continue;
        int idx = -1;
        if (c == 'D') idx = 0;
        else if (c == 'R') idx = 1;
        else if (c == 'H') idx = 2;
        else if (c == 'A') idx = 3;
        else if (c == 'S') idx = 4;
        if (idx >= 0)
        {
            bool exclude = (i > 0 && attrib[i - 1] == '-');
            mustHave[idx] = exclude ? -1 : 1;
        }
    }
    if (mustHave[0] == 1 && !e.isDir) return false;
    if (mustHave[0] == -1 && e.isDir) return false;
    if (mustHave[1] == 1 && !(e.attributes & FILE_ATTRIBUTE_READONLY)) return false;
    if (mustHave[1] == -1 && (e.attributes & FILE_ATTRIBUTE_READONLY)) return false;
    if (mustHave[2] == 1 && !(e.attributes & FILE_ATTRIBUTE_HIDDEN)) return false;
    if (mustHave[2] == -1 && (e.attributes & FILE_ATTRIBUTE_HIDDEN)) return false;
    if (mustHave[3] == 1 && !(e.attributes & FILE_ATTRIBUTE_ARCHIVE)) return false;
    if (mustHave[3] == -1 && (e.attributes & FILE_ATTRIBUTE_ARCHIVE)) return false;
    if (mustHave[4] == 1 && !(e.attributes & FILE_ATTRIBUTE_SYSTEM)) return false;
    if (mustHave[4] == -1 && (e.attributes & FILE_ATTRIBUTE_SYSTEM)) return false;
    return true;
}

static std::string GetExtension(const std::string& name)
{
    size_t dot = name.rfind('.');
    if (dot == std::string::npos)
        return "";
    return name.substr(dot);
}

static int CompareDirEntry(const DirEntry& a, const DirEntry& b, char sortBy, bool reverse)
{
    int cmp = 0;
    if (sortBy == 'N' || sortBy == 0)
    {
        std::string na = a.name, nb = b.name;
        for (size_t i = 0; i < na.length(); i++) na[i] = (char)toupper((unsigned char)na[i]);
        for (size_t i = 0; i < nb.length(); i++) nb[i] = (char)toupper((unsigned char)nb[i]);
        cmp = na.compare(nb);
    }
    else if (sortBy == 'E')
    {
        cmp = GetExtension(a.name).compare(GetExtension(b.name));
        if (cmp == 0)
            cmp = a.name.compare(b.name);
    }
    else if (sortBy == 'D')
    {
        __int64 ta = ((__int64)a.lastWriteTime.dwHighDateTime << 32) | a.lastWriteTime.dwLowDateTime;
        __int64 tb = ((__int64)b.lastWriteTime.dwHighDateTime << 32) | b.lastWriteTime.dwLowDateTime;
        cmp = (ta < tb) ? -1 : (ta > tb) ? 1 : 0;
    }
    else if (sortBy == 'S')
    {
        cmp = (a.size < b.size) ? -1 : (a.size > b.size) ? 1 : 0;
    }
    if (reverse)
        cmp = -cmp;
    return cmp;
}

std::string FileSystem::ListDirectory(const std::string& path)
{
    return ListDirectory(path, DirOptions());
}

std::string FileSystem::ListDirectory(const std::string& path, const DirOptions& options)
{
    std::string apiPath = ToApiPath(path);
    std::string searchPath = apiPath;
    if (searchPath.length() > 0 && searchPath[searchPath.length() - 1] != '\\')
        searchPath += "\\";
    searchPath += "*";

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(searchPath.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if (err == ERROR_PATH_NOT_FOUND || err == ERROR_FILE_NOT_FOUND)
            return "File Not Found\n";
        return "Error reading directory\n";
    }

    std::vector<DirEntry> entries;
    do
    {
        DirEntry e;
        e.name = fd.cFileName;
        e.isDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        e.size = ((unsigned __int64)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
        e.lastWriteTime = fd.ftLastWriteTime;
        e.attributes = fd.dwFileAttributes;
        if (!PassesAttributeFilter(e, options.attrib))
            continue;
        entries.push_back(e);
    }
    while (FindNextFileA(h, &fd));
    FindClose(h);

    if (options.sortBy == 'N' || options.sortBy == 'D' || options.sortBy == 'S' || options.sortBy == 'E')
    {
        for (size_t i = 0; i < entries.size(); i++)
        {
            for (size_t j = i + 1; j < entries.size(); j++)
            {
                if (CompareDirEntry(entries[i], entries[j], options.sortBy, options.sortReverse) > 0)
                {
                    DirEntry t = entries[i];
                    entries[i] = entries[j];
                    entries[j] = t;
                }
            }
        }
    }

    std::string out;
    std::string drivePart = path.find('\\') != std::string::npos ? path.substr(0, path.find('\\')) : path;
    out += " Volume in drive " + drivePart + " has no label.\n";
    out += " Volume Serial Number is 0000-0000\n\n";
    out += " Directory of " + apiPath + "\n\n";

    int dirCount = 0;
    int fileCount = 0;
    unsigned __int64 totalBytes = 0;
    int entryLineCount = 0;
    const int WIDE_COLUMNS = 5;
    const int WIDE_COL_WIDTH = 14;

    if (options.wide)
    {
        int col = 0;
        for (size_t i = 0; i < entries.size(); i++)
        {
            const DirEntry& e = entries[i];
            if (e.isDir)
                dirCount++;
            else
            {
                fileCount++;
                totalBytes += e.size;
            }
            std::string disp = e.name;
            if ((int)disp.length() > WIDE_COL_WIDTH)
                disp = disp.substr(0, WIDE_COL_WIDTH);
            while ((int)disp.length() < WIDE_COL_WIDTH)
                disp += " ";
            out += disp;
            col++;
            if (col >= WIDE_COLUMNS)
            {
                out += "\n";
                col = 0;
                entryLineCount++;
                if (options.pageLines > 0 && entryLineCount % options.pageLines == 0)
                    out += "--- More ---\n";
            }
        }
        if (col != 0)
            out += "\n";
    }
    else
    {
        for (size_t i = 0; i < entries.size(); i++)
        {
            const DirEntry& e = entries[i];
            if (e.isDir)
            {
                dirCount++;
                out += FormatFileTime(e.lastWriteTime) + "    <DIR>          " + e.name + "\n";
            }
            else
            {
                fileCount++;
                totalBytes += e.size;
                std::string sizeStr = String::FormatBytesWithCommas((uint64_t)e.size);
                while (sizeStr.length() < 16)
                {
                    sizeStr = " " + sizeStr;
                }
                sizeStr += " ";
                out += FormatFileTime(e.lastWriteTime) + " " + sizeStr + e.name + "\n";
            }
            entryLineCount++;
            if (options.pageLines > 0 && entryLineCount % options.pageLines == 0)
                out += "--- More ---\n";
        }
    }

    ULARGE_INTEGER freeBytes, totalDisk, freeToCaller;
    freeBytes.QuadPart = 0;
    totalDisk.QuadPart = 0;
    freeToCaller.QuadPart = 0;
    if (GetDiskFreeSpaceExA(apiPath.c_str(), &freeToCaller, &totalDisk, &freeBytes))
    {
        out += String::Format("               %d File(s) ", fileCount);
        out += String::FormatBytesWithCommas((uint64_t)totalBytes) + " bytes\n";
        out += String::Format("               %d Dir(s)  ", dirCount);
        out += String::FormatBytesWithCommas((uint64_t)freeBytes.QuadPart) + " bytes free\n";
    }
    else
    {
        out += String::Format("               %d File(s) ", fileCount);
        out += String::FormatBytesWithCommas((uint64_t)totalBytes) + " bytes\n";
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

std::string FileSystem::CreateDirectory(const std::string& path)
{
    if (path.empty())
    {
        return "The syntax of the command is incorrect.\n";
    }
    std::string apiPath = ToApiPath(path);
    while (apiPath.length() > 0 && (apiPath[apiPath.length() - 1] == '\\' || apiPath[apiPath.length() - 1] == '/'))
    {
        apiPath.erase(apiPath.length() - 1, 1);
    }
    if (apiPath.empty())
    {
        return "";
    }
    std::vector<std::string> segments;
    std::string seg;
    for (size_t i = 0; i < apiPath.length(); i++)
    {
        char c = apiPath[i];
        if (c == '\\' || c == '/')
        {
            if (!seg.empty())
            {
                segments.push_back(seg);
                seg.clear();
            }
        }
        else
        {
            seg += c;
        }
    }
    if (!seg.empty())
    {
        segments.push_back(seg);
    }
    if (segments.empty())
    {
        return "";
    }
    std::string built = segments[0];
    for (size_t i = 1; i < segments.size(); i++)
    {
        built += "\\";
        built += segments[i];
        if (!CreateDirectoryA(built.c_str(), NULL))
        {
            DWORD err = GetLastError();
            if (err != ERROR_ALREADY_EXISTS)
            {
                if (err == ERROR_PATH_NOT_FOUND)
                {
                    return "The system cannot find the path specified.\n";
                }
                return "Unable to create directory.\n";
            }
        }
    }
    return "";
}

static std::string RemoveDirectoryRecursive(const std::string& apiPath)
{
    std::string searchPath = apiPath;
    if (searchPath.length() > 0 && searchPath[searchPath.length() - 1] != '\\')
    {
        searchPath += "\\";
    }
    searchPath += "*";
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(searchPath.c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            std::string name = fd.cFileName;
            if (name == "." || name == "..")
            {
                continue;
            }
            std::string full = apiPath;
            if (full.length() > 0 && full[full.length() - 1] != '\\')
            {
                full += "\\";
            }
            full += name;
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                std::string err = RemoveDirectoryRecursive(full);
                if (!err.empty())
                {
                    FindClose(h);
                    return err;
                }
                if (!RemoveDirectoryA(full.c_str()))
                {
                    DWORD e = GetLastError();
                    FindClose(h);
                    if (e == ERROR_PATH_NOT_FOUND)
                    {
                        return "The system cannot find the path specified.\n";
                    }
                    return "Unable to remove directory.\n";
                }
            }
            else
            {
                if (!DeleteFileA(full.c_str()))
                {
                    DWORD e = GetLastError();
                    FindClose(h);
                    if (e == ERROR_PATH_NOT_FOUND)
                    {
                        return "The system cannot find the path specified.\n";
                    }
                    return "Unable to delete file.\n";
                }
            }
        }
        while (FindNextFileA(h, &fd));
        FindClose(h);
    }
    if (!RemoveDirectoryA(apiPath.c_str()))
    {
        DWORD err = GetLastError();
        if (err == ERROR_DIR_NOT_EMPTY)
        {
            return "The directory is not empty.\n";
        }
        if (err == ERROR_PATH_NOT_FOUND)
        {
            return "The system cannot find the path specified.\n";
        }
        return "Unable to remove directory.\n";
    }
    return "";
}

std::string FileSystem::RemoveDirectory(const std::string& path, bool removeTree)
{
    if (path.empty())
    {
        return "The syntax of the command is incorrect.\n";
    }
    std::string apiPath = ToApiPath(path);
    while (apiPath.length() > 0 && (apiPath[apiPath.length() - 1] == '\\' || apiPath[apiPath.length() - 1] == '/'))
    {
        apiPath.erase(apiPath.length() - 1, 1);
    }
    if (apiPath.empty())
    {
        return "The syntax of the command is incorrect.\n";
    }
    DWORD attrs = GetFileAttributesA(apiPath.c_str());
    if (attrs == 0xFFFFFFFF)
    {
        return "The system cannot find the file specified.\n";
    }
    if ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
        return "The directory name is invalid.\n";
    }
    if (removeTree)
    {
        return RemoveDirectoryRecursive(apiPath);
    }
    if (!RemoveDirectoryA(apiPath.c_str()))
    {
        DWORD err = GetLastError();
        if (err == ERROR_DIR_NOT_EMPTY)
        {
            return "The directory is not empty.\n";
        }
        if (err == ERROR_PATH_NOT_FOUND)
        {
            return "The system cannot find the path specified.\n";
        }
        return "Unable to remove directory.\n";
    }
    return "";
}

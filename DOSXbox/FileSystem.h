#pragma once

#include "External.h"

#include <string>

struct DirOptions
{
    bool wide;           /* /W: wide list format */
    std::string attrib; /* /A[:]attributes: D R H A S, prefix - to exclude */
    char sortBy;         /* /O[:]sort: N=name, D=date, S=size, E=extension */
    bool sortReverse;   /* - prefix on sort (e.g. -N) */
    int pageLines;      /* /P: insert "More" every N lines (0 = off) */
    DirOptions() : wide(false), sortBy('N'), sortReverse(false), pageLines(0) {}
};

class FileSystem
{
public:
    /** Convert internal path (e.g. HDD0-E\\) to Win32 path (e.g. HDD0-E:\\) */
    static std::string ToApiPath(const std::string& path);

    /** List directory in DIR-style format; returns formatted string or error message */
    static std::string ListDirectory(const std::string& path);

    /** List directory with DIR options (/W, /A, /O, /P) */
    static std::string ListDirectory(const std::string& path, const DirOptions& options);

    /** Return true if path exists and is a directory */
    static bool IsDirectory(const std::string& path);

    /** Return true if path exists (file or directory) */
    static bool Exists(const std::string& path);
};

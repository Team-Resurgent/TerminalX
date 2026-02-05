#pragma once

#include "External.h"

#include <string>
#include <vector>

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

    /** Create directory and any intermediate directories; returns empty on success, error message otherwise */
    static std::string CreateDir(const std::string& path);

    /** Remove directory; if removeTree true, delete contents recursively (/S). Returns empty on success, error message otherwise */
    static std::string RemoveDir(const std::string& path, bool removeTree);

    /** Copy single file; creates parent of destination if needed. overwrite: false = fail if dest exists. Returns empty or error message */
    static std::string CopyPath(const std::string& src, const std::string& dst, bool overwrite);

    /** Append sources to destination (first source overwrites dest, rest appended). Returns empty or error message */
    static std::string AppendFiles(const std::vector<std::string>& sources, const std::string& dest);

    /** Delete one or more files. recursive=/S, force=/F, attribFilter=/A, showOnlyDeleted=true when /S (show only deleted). Returns empty or error message; with /S appends "path\n" per deleted file. */
    static std::string DeletePath(const std::string& path, bool recursive, bool force, const std::string& attribFilter, bool showOnlyDeleted);

    /** Move or rename file or directory. overwrite: allow overwriting existing destination file. Returns empty or error message. */
    static std::string MovePath(const std::string& src, const std::string& dst, bool overwrite);

    /** Fill outNames/outIsDir with directory entries whose names start with prefix (case-insensitive). dirPath is internal path (e.g. HDD0-E\\cerbios). Returns true if directory was listed. */
    static bool GetPathCompletions(const std::string& dirPath, const std::string& prefix, std::vector<std::string>& outNames, std::vector<bool>& outIsDir);
};

#pragma once

#include "External.h"

#include <string>

class FileSystem
{
public:
    /** Convert internal path (e.g. HDD0-E\\) to Win32 path (e.g. HDD0-E:\\) */
    static std::string ToApiPath(const std::string& path);

    /** List directory in DIR-style format; returns formatted string or error message */
    static std::string ListDirectory(const std::string& path);

    /** Return true if path exists and is a directory */
    static bool IsDirectory(const std::string& path);

    /** Return true if path exists (file or directory) */
    static bool Exists(const std::string& path);
};

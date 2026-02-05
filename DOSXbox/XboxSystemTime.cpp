#include "XboxSystemTime.h"
#include "External.h"

bool SetXboxSystemTime(const SYSTEMTIME* st)
{
    SYSTEMTIME local = *st;
    local.wMilliseconds = 0;
    FILETIME localFileTime;
    if (!SystemTimeToFileTime(&local, &localFileTime))
    {
        return false;
    }
    FILETIME utcFileTime;
    if (!LocalFileTimeToFileTime(&localFileTime, &utcFileTime))
    {
        return false;
    }
    LARGE_INTEGER li;
    li.LowPart = utcFileTime.dwLowDateTime;
    li.HighPart = (LONG)utcFileTime.dwHighDateTime;
    return NtSetSystemTime(&li, NULL) == STATUS_SUCCESS;
}

bool GetXboxSystemTime(SYSTEMTIME* st)
{
    SYSTEMTIME utcSt;
    GetSystemTime(&utcSt);
    FILETIME utcFileTime;
    if (!SystemTimeToFileTime(&utcSt, &utcFileTime))
    {
        return false;
    }
    FILETIME localFileTime;
    if (!FileTimeToLocalFileTime(&utcFileTime, &localFileTime))
    {
        return false;
    }
    if (!FileTimeToSystemTime(&localFileTime, st))
    {
        return false;
    }
    return true;
}

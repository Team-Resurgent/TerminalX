#include "XboxSystemTime.h"
#include "External.h"

bool SetXboxSystemTime(const SYSTEMTIME* st)
{
    FILETIME localFileTime;
    if (!SystemTimeToFileTime(st, &localFileTime)) {
        return false;
    }
    FILETIME utcFileTime;
    if (!LocalFileTimeToFileTime(&localFileTime, &utcFileTime)) {
        return false;
    }
    LARGE_INTEGER li;
    li.LowPart = utcFileTime.dwLowDateTime;
    li.HighPart = (LONG)utcFileTime.dwHighDateTime;
    return NtSetSystemTime(&li, NULL) == STATUS_SUCCESS;
}

#include "TimeCommand.h"
#include "..\String.h"
#include "..\XboxSystemTime.h"
#include <string>
#include <vector>
#include <ctype.h>
#include <stdlib.h>
#include <xtl.h>

static bool IsSwitch(const std::string& a)
{
    return (a.length() >= 1 && (a[0] == '/' || a[0] == '-'));
}

static std::string GetCurrentTimeString()
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    const char* ampm = (st.wHour < 12) ? "AM" : "PM";
    int hour12 = (st.wHour % 12);
    if (hour12 == 0)
        hour12 = 12;
    return String::Format("%d:%02u:%02u %s",
        hour12,
        (unsigned)st.wMinute,
        (unsigned)st.wSecond,
        ampm);
}

/* Parse HH:MM, HH:MM:SS, H.MM, HH.MM.SS, or HH:MM AM/PM. Sets hour, min, sec (sec default 0). Returns true on success. */
static bool ParseTime(const std::string& s, int& hour, int& minute, int& second)
{
    if (s.empty())
        return false;
    const char* p = s.c_str();
    while (*p == ' ')
        p++;
    if (!*p || !isdigit((unsigned char)*p))
        return false;
    hour = (int)strtol(p, (char**)&p, 10);
    while (*p == ' ' || *p == ':' || *p == '.')
        p++;
    if (!*p || !isdigit((unsigned char)*p))
        return false;
    minute = (int)strtol(p, (char**)&p, 10);
    second = 0;
    while (*p == ' ' || *p == ':' || *p == '.')
        p++;
    if (*p && isdigit((unsigned char)*p))
        second = (int)strtol(p, (char**)&p, 10);
    while (*p == ' ')
        p++;
    if (*p)
    {
        if ((p[0] == 'A' || p[0] == 'a') && (p[1] == 'M' || p[1] == 'm'))
        {
            if (hour == 12)
                hour = 0;
        }
        else if ((p[0] == 'P' || p[0] == 'p') && (p[1] == 'M' || p[1] == 'm'))
        {
            if (hour != 12)
                hour += 12;
        }
    }
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59)
        return false;
    return true;
}

static std::string SetTime(const std::string& timeArg)
{
    int hour = 0, minute = 0, second = 0;
    if (!ParseTime(timeArg, hour, minute, second))
    {
        return "The system cannot accept the time entered.\n";
    }
    SYSTEMTIME st;
    GetSystemTime(&st);
    st.wHour = (WORD)hour;
    st.wMinute = (WORD)minute;
    st.wSecond = (WORD)second;
    st.wMilliseconds = 0;
    if (!SetXboxSystemTime(&st))
    {
        return "Access is denied.\n";
    }
    return "";
}

bool TimeCommand::Matches(const std::string& cmd)
{
    return (cmd == "TIME");
}

std::string TimeCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    (void)ctx;
    std::string timeArg;

    for (size_t i = 1; i < args.size(); i++)
    {
        std::string a = args[i];
        if (IsSwitch(a))
        {
            if (a == "/?" || a == "-?" || a.find('?') != std::string::npos)
            {
                return "Displays or sets the system time.\n\n"
                       "TIME [/T | time]\n\n"
                       "  With no parameters, or /T, displays the current time.\n"
                       "  time  Set the time (e.g. HH:MM or HH:MM:SS).\n";
            }
        }
        else
        {
            timeArg = a;
        }
    }

    if (!timeArg.empty())
    {
        return SetTime(timeArg);
    }

    std::string timeStr = GetCurrentTimeString();
    return timeStr + "\n";
}

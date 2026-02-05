#include "TimeCommand.h"
#include "..\CommandProcessor.h"
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
    if (!GetXboxSystemTime(&st))
    {
        return "??:??:?? ??";
    }
    const char* ampm = (st.wHour < 12) ? "AM" : "PM";
    int hour12 = (st.wHour % 12);
    if (hour12 == 0)
    {
        hour12 = 12;
    }
    return String::Format("%d:%02u:%02u %s",
        hour12,
        (unsigned)st.wMinute,
        (unsigned)st.wSecond,
        ampm);
}

static std::string GetCurrentTimeString24()
{
    SYSTEMTIME st;
    if (!GetXboxSystemTime(&st))
    {
        return "??:??:??.??";
    }
    return String::Format("%02u:%02u:%02u.%02u",
        (unsigned)st.wHour,
        (unsigned)st.wMinute,
        (unsigned)st.wSecond,
        (unsigned)(st.wMilliseconds / 10));
}

/* Parse HH:MM, HH:MM:SS, HH:MM:SS.cc, or HH:MM AM/PM. Sets hour, min, sec (sec default 0). Returns true on success. */
static bool ParseTime(const std::string& s, int& hour, int& minute, int& second)
{
    if (s.empty())
    {
        return false;
    }
    const char* p = s.c_str();
    while (*p == ' ' || *p == '\r' || *p == '\n')
    {
        p++;
    }
    if (!*p || !isdigit((unsigned char)*p))
    {
        return false;
    }
    char* end;
    hour = (int)strtol(p, &end, 10);
    p = end;
    while (*p == ' ' || *p == ':' || *p == '.')
    {
        p++;
    }
    if (!*p || !isdigit((unsigned char)*p))
    {
        return false;
    }
    minute = (int)strtol(p, &end, 10);
    p = end;
    second = 0;
    while (*p == ' ' || *p == ':' || *p == '.')
    {
        p++;
    }
    if (*p && isdigit((unsigned char)*p))
    {
        second = (int)strtol(p, &end, 10);
        p = end;
    }
    if (*p == '.' && isdigit((unsigned char)p[1]))
    {
        (void)strtol(p + 1, &end, 10); /* skip hundredths */
        p = end;
    }
    while (*p == ' ' || *p == '\r' || *p == '\n')
    {
        p++;
    }
    if (*p)
    {
        if ((p[0] == 'A' || p[0] == 'a') && (p[1] == 'M' || p[1] == 'm'))
        {
            if (hour == 12)
            {
                hour = 0;
            }
        }
        else if ((p[0] == 'P' || p[0] == 'p') && (p[1] == 'M' || p[1] == 'm'))
        {
            if (hour != 12)
            {
                hour += 12;
            }
        }
    }
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59)
    {
        return false;
    }
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
    if (!GetXboxSystemTime(&st))
    {
        return "Unable to read system time.\n";
    }
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
    bool displayOnly = false; /* /T */

    for (size_t i = 1; i < args.size(); i++)
    {
        std::string a = args[i];
        if (IsSwitch(a))
        {
            if (a == "/?" || a == "-?" || a.find('?') != std::string::npos)
            {
                return "Displays or sets the system time.\n\n"
                       "TIME [/T | time]\n\n"
                       "  With no parameters, prompts for a new time. Press ENTER to keep the same time.\n"
                       "  /T    Display current time only (no prompt).\n"
                       "  time  Set the time (e.g. HH:MM or HH:MM:SS).\n";
            }
            if (String::ToUpper(a) == "/T" || String::ToUpper(a) == "-T")
            {
                displayOnly = true;
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

    if (displayOnly)
    {
        return GetCurrentTimeString24() + "\n";
    }

    CommandProcessor::SetPendingInputType(CommandProcessor::PendingTime);
    std::string timeStr = GetCurrentTimeString24();
    return std::string("\x03", 1) + "TIME\nThe current time is: " + timeStr + "\nEnter the new time: ";
}

std::string TimeCommand::SetTimeFromString(const std::string& line)
{
    std::string s = line;
    while (!s.empty() && (s[0] == ' ' || s[0] == '\t' || s[0] == '\r' || s[0] == '\n'))
    {
        s.erase(0, 1);
    }
    while (!s.empty() && (s[s.length() - 1] == ' ' || s[s.length() - 1] == '\t' || s[s.length() - 1] == '\r' || s[s.length() - 1] == '\n'))
    {
        s.erase(s.length() - 1, 1);
    }
    if (s.empty())
    {
        return ""; /* keep same time */
    }
    int hour = 0, minute = 0, second = 0;
    if (!ParseTime(s, hour, minute, second))
    {
        return "The system cannot accept the time entered.\n";
    }
    SYSTEMTIME st;
    if (!GetXboxSystemTime(&st))
    {
        return "Unable to read system time.\n";
    }
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

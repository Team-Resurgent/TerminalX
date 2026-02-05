#include "DateCommand.h"
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

static std::string GetCurrentDateString()
{
    SYSTEMTIME st;
    GetSystemTime(&st);
    return String::Format("%02u/%02u/%04u",
        (unsigned)st.wMonth,
        (unsigned)st.wDay,
        (unsigned)st.wYear);
}

/* Parse mm-dd-yy, mm/dd/yy, mm-dd-yyyy, mm/dd/yyyy. Returns true and sets m,d,y on success. */
static bool ParseDate(const std::string& s, int& month, int& day, int& year)
{
    if (s.empty())
        return false;
    const char* p = s.c_str();
    while (*p == ' ')
        p++;
    if (!*p || !isdigit((unsigned char)*p))
        return false;
    month = (int)strtol(p, (char**)&p, 10);
    while (*p == ' ' || *p == '-' || *p == '/')
        p++;
    if (!*p || !isdigit((unsigned char)*p))
        return false;
    day = (int)strtol(p, (char**)&p, 10);
    while (*p == ' ' || *p == '-' || *p == '/')
        p++;
    if (!*p || !isdigit((unsigned char)*p))
        return false;
    year = (int)strtol(p, (char**)&p, 10);
    if (year >= 0 && year <= 99)
        year += (year >= 70) ? 1900 : 2000;
    if (month < 1 || month > 12 || day < 1 || day > 31 || year < 1980 || year > 2099)
        return false;
    return true;
}

static std::string SetDate(const std::string& dateArg)
{
    int month = 0, day = 0, year = 0;
    if (!ParseDate(dateArg, month, day, year))
    {
        return "The system cannot accept the date entered.\n";
    }
    SYSTEMTIME st;
    GetSystemTime(&st);
    st.wMonth = (WORD)month;
    st.wDay = (WORD)day;
    st.wYear = (WORD)year;
    st.wDayOfWeek = 0; 
    if (!SetXboxSystemTime(&st))
    {
        return "Access is denied.\n";
    }
    return "";
}

bool DateCommand::Matches(const std::string& cmd)
{
    return (cmd == "DATE");
}

std::string DateCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    (void)ctx;
    std::string dateArg;

    for (size_t i = 1; i < args.size(); i++)
    {
        std::string a = args[i];
        if (IsSwitch(a))
        {
            if (a == "/?" || a == "-?" || a.find('?') != std::string::npos)
            {
                return "Displays or sets the date.\n\n"
                       "DATE [/T | date]\n\n"
                       "  With no parameters, or /T, displays the current date.\n"
                       "  date  Set the date (e.g. mm-dd-yy or mm/dd/yyyy).\n";
            }
        }
        else
        {
            dateArg = a;
        }
    }

    if (!dateArg.empty())
    {
        return SetDate(dateArg);
    }

    std::string dateStr = GetCurrentDateString();
    return dateStr + "\n";
}

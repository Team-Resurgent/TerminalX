#include "DateCommand.h"
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

static std::string GetCurrentDateString()
{
    SYSTEMTIME st;
    if (!GetXboxSystemTime(&st))
    {
        return "??/??/????";
    }
    return String::Format("%02u/%02u/%04u",
        (unsigned)st.wMonth,
        (unsigned)st.wDay,
        (unsigned)st.wYear);
}

static std::string GetCurrentDateStringYmd()
{
    SYSTEMTIME st;
    if (!GetXboxSystemTime(&st))
    {
        return "????-??-??";
    }
    return String::Format("%04u-%02u-%02u",
        (unsigned)st.wYear,
        (unsigned)st.wMonth,
        (unsigned)st.wDay);
}

/* Parse yy-mm-dd or yyyy-mm-dd. Returns true and sets y,m,d on success. */
static bool ParseDateYmd(const std::string& s, int& year, int& month, int& day)
{
    if (s.empty())
    {
        return false;
    }
    const char* p = s.c_str();
    while (*p == ' ')
    {
        p++;
    }
    if (!*p || !isdigit((unsigned char)*p))
    {
        return false;
    }
    year = (int)strtol(p, (char**)&p, 10);
    while (*p == ' ' || *p == '-' || *p == '/')
    {
        p++;
    }
    if (!*p || !isdigit((unsigned char)*p))
    {
        return false;
    }
    month = (int)strtol(p, (char**)&p, 10);
    while (*p == ' ' || *p == '-' || *p == '/')
    {
        p++;
    }
    if (!*p || !isdigit((unsigned char)*p))
    {
        return false;
    }
    day = (int)strtol(p, (char**)&p, 10);
    if (year >= 0 && year <= 99)
    {
        year += (year >= 70) ? 1900 : 2000;
    }
    if (month < 1 || month > 12 || day < 1 || day > 31 || year < 1980 || year > 2099)
    {
        return false;
    }
    return true;
}

/* Parse mm-dd-yy, mm/dd/yy, mm-dd-yyyy, mm/dd/yyyy. Returns true and sets m,d,y on success. */
static bool ParseDate(const std::string& s, int& month, int& day, int& year)
{
    if (s.empty())
    {
        return false;
    }
    const char* p = s.c_str();
    while (*p == ' ')
    {
        p++;
    }
    if (!*p || !isdigit((unsigned char)*p))
    {
        return false;
    }
    month = (int)strtol(p, (char**)&p, 10);
    while (*p == ' ' || *p == '-' || *p == '/')
    {
        p++;
    }
    if (!*p || !isdigit((unsigned char)*p))
    {
        return false;
    }
    day = (int)strtol(p, (char**)&p, 10);
    while (*p == ' ' || *p == '-' || *p == '/')
    {
        p++;
    }
    if (!*p || !isdigit((unsigned char)*p))
    {
        return false;
    }
    year = (int)strtol(p, (char**)&p, 10);
    if (year >= 0 && year <= 99)
    {
        year += (year >= 70) ? 1900 : 2000;
    }
    if (month < 1 || month > 12 || day < 1 || day > 31 || year < 1980 || year > 2099)
    {
        return false;
    }
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
    if (!GetXboxSystemTime(&st))
    {
        return "Unable to read system date.\n";
    }
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
    bool displayOnly = false; /* /T */

    for (size_t i = 1; i < args.size(); i++)
    {
        std::string a = args[i];
        if (IsSwitch(a))
        {
            if (a == "/?" || a == "-?" || a.find('?') != std::string::npos)
            {
                return "Displays or sets the date.\n\n"
                       "DATE [/T | date]\n\n"
                       "  With no parameters, prompts for a new date. Press ENTER to keep the same date.\n"
                       "  /T    Display current date only (no prompt).\n"
                       "  date  Set the date (e.g. yy-mm-dd or mm-dd-yy).\n";
            }
            if (String::ToUpper(a) == "/T" || String::ToUpper(a) == "-T")
            {
                displayOnly = true;
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

    if (displayOnly)
    {
        return GetCurrentDateStringYmd() + "\n";
    }

    CommandProcessor::SetPendingInputType(CommandProcessor::PendingDate);
    std::string dateStr = GetCurrentDateStringYmd();
    return std::string("\x03", 1) + "DATE\nThe current date is: " + dateStr + "\nEnter the new date: (yy-mm-dd) ";
}

std::string DateCommand::SetDateFromString(const std::string& line)
{
    std::string s = line;
    while (!s.empty() && (s[0] == ' ' || s[0] == '\t'))
    {
        s.erase(0, 1);
    }
    while (!s.empty() && (s[s.length() - 1] == ' ' || s[s.length() - 1] == '\t'))
    {
        s.erase(s.length() - 1, 1);
    }
    if (s.empty())
    {
        return ""; /* keep same date */
    }
    int year = 0, month = 0, day = 0;
    if (ParseDateYmd(s, year, month, day))
    {
        SYSTEMTIME st;
        if (!GetXboxSystemTime(&st))
        {
            return "Unable to read system date.\n";
        }
        st.wYear = (WORD)year;
        st.wMonth = (WORD)month;
        st.wDay = (WORD)day;
        st.wDayOfWeek = 0;
        if (!SetXboxSystemTime(&st))
        {
            return "Access is denied.\n";
        }
        return "";
    }
    int m = 0, d = 0, y = 0;
    if (ParseDate(s, m, d, y))
    {
        return SetDate(s);
    }
    return "The system cannot accept the date entered.\n";
}

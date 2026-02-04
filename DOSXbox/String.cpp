#include "String.h"

#include <xtl.h>
#include <string>
#include <ctype.h>

std::string String::Format(std::string message, ...)
{
    char buffer[1024];

    va_list arglist;
    va_start(arglist, message);
    _vsnprintf(buffer, 1024, message.c_str(), arglist);
    va_end(arglist);

    buffer[1024 - 1] = '\0';
    return std::string(buffer);
}

std::string String::ToUpper(const std::string& s)
{
    std::string r = s;
    for (size_t i = 0; i < r.length(); i++)
    {
        r[i] = (char)toupper((unsigned char)r[i]);
    }
    return r;
}

std::string String::ToLower(const std::string& s)
{
    std::string r = s;
    for (size_t i = 0; i < r.length(); i++)
    {
        r[i] = (char)tolower((unsigned char)r[i]);
    }
    return r;
}

static int CompareLoop(const std::string& a, const std::string& b)
{
    size_t i = 0;
    size_t na = a.length();
    size_t nb = b.length();
    while (i < na && i < nb)
    {
        int ca = (unsigned char)a[i];
        int cb = (unsigned char)b[i];
        if (ca != cb)
        {
            return ca - cb;
        }
        i++;
    }
    if (i < na) { return (unsigned char)a[i]; }
    if (i < nb) { return -(unsigned char)b[i]; }
    return 0;
}

int String::Compare(const std::string& a, const std::string& b, bool ignoreCase)
{
    if (ignoreCase)
    {
        return CompareLoop(ToUpper(a), ToUpper(b));
    }
    return CompareLoop(a, b);
}

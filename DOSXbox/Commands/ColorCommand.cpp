#include "ColorCommand.h"
#include "..\TerminalBuffer.h"
#include "..\String.h"
#include <string>
#include <vector>
#include <cctype>

static int HexDigitToInt(char c)
{
    c = (char)toupper((unsigned char)c);
    if (c >= '0' && c <= '9')
    {
        return (c - '0');
    }
    if (c >= 'A' && c <= 'F')
    {
        return (c - 'A' + 10);
    }
    return -1;
}

bool ColorCommand::Matches(const std::string& cmd)
{
    return (cmd == "COLOR");
}

std::string ColorCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    (void)ctx;
    if (args.size() < 2)
    {
        TerminalBuffer::ResetColorAttribute();
        return "";
    }
    std::string arg = String::ToUpper(args[1]);
    if (arg == "/?" || arg == "-?" || arg.find('?') != std::string::npos)
    {
        return "Sets the default console foreground and background colors.\n\n"
               "COLOR [attr]\n\n"
               "  attr        Specifies color attribute (two hex digits: background, foreground).\n\n"
               "  0=Black  1=Blue   2=Green  3=Aqua   4=Red   5=Purple  6=Yellow  7=White\n"
               "  8=Gray   9=Light Blue  A=Light Green  B=Light Aqua  C=Light Red\n"
               "  D=Light Purple  E=Light Yellow  F=Bright White\n\n"
               "If no argument is given, restores the default color (0A = green on black).\n"
               "Foreground and background must be different.\n\n"
               "Example: COLOR fc produces light red on bright white\n";
    }
    if (arg.length() != 2)
    {
        return "Invalid color attribute. Specify two hex digits (e.g. 0A or fc).\n";
    }
    int bg = HexDigitToInt(arg[0]);
    int fg = HexDigitToInt(arg[1]);
    if (bg < 0 || fg < 0)
    {
        return "Invalid color attribute. Use digits 0-9 and letters A-F.\n";
    }
    if (bg == fg)
    {
        return "The foreground and background colors must be different.\n";
    }
    unsigned char attr = (unsigned char)((bg << 4) | fg);
    TerminalBuffer::SetColorAttribute(attr);
    return "";
}

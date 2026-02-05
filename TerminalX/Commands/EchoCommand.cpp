#include "EchoCommand.h"
#include "..\CommandProcessor.h"
#include "..\String.h"
#include <string>
#include <vector>

bool EchoCommand::Matches(const std::string& cmd)
{
    return (cmd == "ECHO");
}

static bool IsSwitch(const std::string& a)
{
    return (a.length() >= 1 && (a[0] == '/' || a[0] == '-'));
}

std::string EchoCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    (void)ctx;
    if (args.size() < 2)
    {
        return (CommandProcessor::GetEcho() ? "ECHO is on.\n" : "ECHO is off.\n");
    }
    std::string arg = args[1];
    if (IsSwitch(arg) && (arg == "/?" || arg == "-?" || arg.find('?') != std::string::npos))
    {
        return "Displays messages, or turns command-echoing on or off.\n\n"
               "  ECHO [ON | OFF]\n"
               "  ECHO [message]\n\n"
               "  Type ECHO without parameters to display the current echo setting.\n";
    }
    std::string upper = String::ToUpper(arg);
    if (upper == "ON")
    {
        CommandProcessor::SetEcho(true);
        return "";
    }
    if (upper == "OFF")
    {
        CommandProcessor::SetEcho(false);
        return "";
    }
    /* ECHO message: output the rest of the line (args[1] might have been parsed as one token; join args 1..n) */
    std::string message = args[1];
    for (size_t i = 2; i < args.size(); i++)
    {
        message += " ";
        message += args[i];
    }
    return message + "\n";
}

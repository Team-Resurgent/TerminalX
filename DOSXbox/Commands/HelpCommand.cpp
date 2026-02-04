#include "HelpCommand.h"
#include <string>
#include <vector>

bool HelpCommand::Matches(const std::string& cmd)
{
    return (cmd == "HELP");
}

std::string HelpCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    (void)args;
    (void)ctx;
    return "For more information on a specific command, type HELP command-name\n"
           "DIR    Displays a list of files and subdirectories in a directory.\n"
           "CD     Displays the name of or changes the current directory.\n"
           "CLS    Clears the screen.\n"
           "VER    Displays the Windows version.\n"
           "HELP   Provides Help information for Windows commands.\n"
           "EXIT   Quits the command interpreter.\n";
}

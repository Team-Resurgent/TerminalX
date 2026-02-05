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
           "MD     Creates a directory (MKDIR).\n"
           "RD     Removes a directory (RMDIR).\n"
           "CLS    Clears the screen.\n"
           "COPY   Copies one or more files to another location.\n"
           "DATE   Displays or sets the date.\n"
           "DEL    Deletes one or more files (ERASE).\n"
           "ECHO   Displays messages, or turns command-echoing on or off.\n"
           "TIME   Displays or sets the system time.\n"
           "MOVE   Moves files and renames files and directories.\n"
           "COLOR  Sets the default console foreground and background colors.\n"
           "VER    Displays the Windows version.\n"
           "HELP   Provides Help information for Windows commands.\n"
           "EXIT   Quits the command interpreter.\n";
}

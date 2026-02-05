#include "LoginCommand.h"
#include "..\String.h"
#include <string>
#include <vector>

bool LoginCommand::Matches(const std::string& cmd)
{
    return (cmd == "LOGIN");
}

std::string LoginCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    (void)ctx;
    if (args.size() < 2)
    {
        return "Bad command or file name\n";
    }
    std::string name = String::ToUpper(args[1]);
    if (name == "JOSHUA")
    {
        return "Greetings, Professor Falken.\n"
               "Shall we play a game?\n"
               "A strange game. The only winning move is not to play.\n";
    }
    return "Hello, " + args[1] + ".\n";
}

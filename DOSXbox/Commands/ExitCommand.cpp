#include "ExitCommand.h"
#include <string>
#include <vector>

bool ExitCommand::Matches(const std::string& cmd)
{
    return (cmd == "EXIT");
}

std::string ExitCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    (void)args;
    (void)ctx;
    return "\x02EXIT";
}

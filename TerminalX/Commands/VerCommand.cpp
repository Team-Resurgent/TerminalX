#include "VerCommand.h"
#include <string>
#include <vector>

bool VerCommand::Matches(const std::string& cmd)
{
    return (cmd == "VER");
}

std::string VerCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    (void)args;
    (void)ctx;
    return "Microsoft Xbox Original [TerminalX]\n";
}

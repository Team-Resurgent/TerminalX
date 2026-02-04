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
    return "Microsoft Windows XP [Version 5.1.2600]\n(DOSXbox)\n";
}

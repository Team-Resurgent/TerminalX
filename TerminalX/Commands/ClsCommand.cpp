#include "ClsCommand.h"
#include "..\TerminalBuffer.h"
#include <string>
#include <vector>

bool ClsCommand::Matches(const std::string& cmd)
{
    return (cmd == "CLS");
}

std::string ClsCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    (void)args;
    (void)ctx;
    TerminalBuffer::Clear();
    TerminalBuffer::SetCursor(0, TerminalBuffer::GetRows() - 1);
    return "";
}

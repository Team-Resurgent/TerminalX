#include "ShutdownCommand.h"
#include "..\External.h"
#include "..\String.h"
#include <string>
#include <vector>

static bool IsSwitch(const std::string& a)
{
    return (a.length() >= 1 && (a[0] == '/' || a[0] == '-'));
}

bool ShutdownCommand::Matches(const std::string& cmd)
{
    return (cmd == "SHUTDOWN");
}

std::string ShutdownCommand::Execute(const std::vector<std::string>& args, CommandContext& ctx)
{
    (void)ctx;
    /* Options: /S power off (default), /R reboot (cycle), /W warm reboot (reset).
     * Requires SMBDEV_PIC16L, PIC16L_CMD_POWER, POWER_SUBCMD_* defines. */
    int doPowerOff = 1;   /* default */
    int doReboot = 0;
    int doWarmReboot = 0;

    for (size_t i = 1; i < args.size(); i++)
    {
        std::string a = args[i];
        if (!IsSwitch(a))
        {
            continue;
        }
        if (a == "/?" || a == "-?" || a.find('?') != std::string::npos)
        {
            return "Shuts down or reboots the Xbox.\n\n"
                   "SHUTDOWN [/S | /R | /W]\n\n"
                   "  /S  Power off (default).\n"
                   "  /R  Reboot (full power cycle).\n"
                   "  /W  Warm reboot (reset).\n";
        }
        std::string upper = String::ToUpper(a);
        if (upper == "/S" || upper == "-S")
        {
            doPowerOff = 1;
            doReboot = 0;
            doWarmReboot = 0;
        }
        else if (upper == "/R" || upper == "-R")
        {
            doPowerOff = 0;
            doReboot = 1;
            doWarmReboot = 0;
        }
        else if (upper == "/W" || upper == "-W")
        {
            doPowerOff = 0;
            doReboot = 0;
            doWarmReboot = 1;
        }
    }

    if (doWarmReboot)
    {
        HalWriteSMBusValue(SMBDEV_PIC16L, PIC16L_CMD_POWER, 0, POWER_SUBCMD_RESET);
        return "";
    }
    if (doReboot)
    {
        HalWriteSMBusValue(SMBDEV_PIC16L, PIC16L_CMD_POWER, 0, POWER_SUBCMD_CYCLE);
        return "";
    }
    HalWriteSMBusValue(SMBDEV_PIC16L, PIC16L_CMD_POWER, 0, POWER_SUBCMD_POWER_OFF);
    return "";
}

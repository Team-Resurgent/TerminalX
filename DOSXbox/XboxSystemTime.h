#pragma once

#include <xtl.h>

/** Set system time on Xbox using SYSTEMTIME (treated as local) -> UTC FILETIME -> NtSetSystemTime. Returns true on success. */
bool SetXboxSystemTime(const SYSTEMTIME* st);

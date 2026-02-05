#pragma once

#include <xtl.h>

/** Set system time on Xbox. \a st is local time; converted to UTC and passed to NtSetSystemTime. Returns true on success. */
bool SetXboxSystemTime(const SYSTEMTIME* st);

/** Get current system time in local time (UTC from system converted via FileTimeToLocalFileTime). Fills \a st. Returns false if any conversion fails. */
bool GetXboxSystemTime(SYSTEMTIME* st);

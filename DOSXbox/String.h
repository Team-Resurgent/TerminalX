#pragma once

#include "Integers.h"

#include <string>

class String
{
public:
	static std::string Format(std::string message, ...);
	static std::string ToUpper(const std::string& s);
	static std::string ToLower(const std::string& s);
	static int Compare(const std::string& a, const std::string& b, bool ignoreCase = false);
	static std::string FormatBytesWithCommas(uint64_t n);
};

#pragma once

#include <string>

class String
{
public:
	static std::string Format(std::string message, ...);
	static std::string ToUpper(const std::string& s);
	static std::string ToLower(const std::string& s);
	static int Compare(const std::string& a, const std::string& b, bool ignoreCase = false);
};

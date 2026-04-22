#pragma once
#include <string>
#define TIME_STRING_LEN (25)


template <typename... Args>
std::string FormatString(const std::string &format, Args... args);
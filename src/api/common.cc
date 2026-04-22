#include <memory>
#include "common.h"





template <typename... Args>
std::string FormatString(const std::string& format, Args... args)
{
    auto size { snprintf(nullptr, 0, format.c_str(), args...) + 1 };
    std::unique_ptr<char[]> buf {new char [size] };

    snprintf(buf.get(), size, format.c_str(), args...);

    return std::string(buf.get(), buf.get() + size - 1);
}
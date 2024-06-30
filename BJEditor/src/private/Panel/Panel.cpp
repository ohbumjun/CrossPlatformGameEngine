#include "hzpch.h"
#include "Panel/Panel.h"

namespace HazelEditor
{
Panel::Panel()
{
}
Panel::~Panel()
{
}
void Panel::Open(Hazel::TypeId type)
{
}
void Panel::Initialize()
{
}
void Panel::setUniqueName(const char *format, ...)
{
    char buffer[1042] = {
        0,
    };
    va_list rawBufArgs;
    va_start(rawBufArgs, format);
    vsnprintf(buffer, 1042, format, rawBufArgs);
    va_end(rawBufArgs);

    // 1. Determine required buffer size (avoid hardcoding)
    std::size_t requiredSize =
        std::snprintf(nullptr, 0, format, buffer, GetID());

    // 2. Allocate sufficient buffer using std::unique_ptr for RAII
    std::unique_ptr<char[]> uniqueBuffer(
        new char[requiredSize + 1]); // +1 for null terminator

    // 3. Format the string safely using snprintf
    va_list args;
    va_start(args, format);
    int chars_written =
        vsnprintf(uniqueBuffer.get(), requiredSize + 1, format, args);
    va_end(args);

    if (chars_written < 0 ||
        static_cast<std::size_t>(chars_written) != requiredSize)
    {
        // Handle formatting errors (unlikely, but good practice)
        std::cerr << "Error formatting string." << std::endl;
    }

    // 4. Construct std::string using the formatted buffer
    _name = std::string(uniqueBuffer.get()); // No need for strncpy
}
} // namespace HazelEditor

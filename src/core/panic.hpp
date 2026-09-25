#pragma once

#include <source_location>
#include <string_view>

namespace core {

[[noreturn]] auto panic(
    std::string_view message,
    std::source_location location = std::source_location::current()
) -> void;

} // namespace core

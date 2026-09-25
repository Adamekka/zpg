#include "panic.hpp"
#include "defer.hpp"
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <print>

namespace core {

auto panic(const std::string_view message, const std::source_location location)
    -> void {
    std::println(
        stderr,
        "PANIC: {}\n  at {}:{} in {}\n\nStack trace:",
        message,
        location.file_name(),
        location.line(),
        location.function_name()
    );
    auto _{std::fflush(stderr)};

    // Stack trace stores at most 64 frames
    auto frames{std::array<void*, 64>{}};
    const auto frame_count{backtrace(frames.data(), frames.size())};
    if (frame_count <= 0) {
        std::abort();
    }

    for (auto index{size_t{0}}; index < static_cast<size_t>(frame_count);
         ++index) {
        auto symbol{Dl_info{}};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
        if (dladdr(frames[index], &symbol) == 0
            || symbol.dli_sname == nullptr) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            std::println(stderr, "{:>2}  {}", index, frames[index]);
            continue;
        }

        auto status{0};
        auto* const demangled_name{
            abi::__cxa_demangle(symbol.dli_sname, nullptr, nullptr, &status)
        };

        defer({
            // NOLINTBEGIN(cppcoreguidelines-no-malloc)
            // NOLINTBEGIN(cppcoreguidelines-owning-memory)
            std::free(demangled_name);
            // NOLINTEND(cppcoreguidelines-owning-memory)
            // NOLINTEND(cppcoreguidelines-no-malloc)
        });

        const auto* const name{
            status == 0 && demangled_name != nullptr ? demangled_name
                                                     : symbol.dli_sname
        };

        if (symbol.dli_saddr != nullptr) {
            const auto offset{
                // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
                reinterpret_cast<uintptr_t>(frames[index])
                - reinterpret_cast<uintptr_t>(symbol.dli_saddr)
                // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
            };
            std::println(stderr, "{:>2}  {} + {:#x}", index, name, offset);
        } else {
            std::println(stderr, "{:>2}  {}", index, name);
        }
    }

    std::abort();
}

} // namespace core

#pragma once

#include <utility>

namespace core {

template<typename Fn> class ScopeExit final {
  public:
    constexpr explicit ScopeExit(Fn function) noexcept
        : function{std::move(function)} {}

    ScopeExit(const ScopeExit&) = delete;
    ScopeExit(ScopeExit&&) = delete;

    constexpr ~ScopeExit() noexcept {
        this->function();
    }

    auto operator=(const ScopeExit&) -> ScopeExit& = delete;
    auto operator=(ScopeExit&&) -> ScopeExit& = delete;

  private:
    Fn function;
};

namespace detail {

struct Defer final {};

template<typename Fn>
[[nodiscard]]
// NOLINTNEXTLINE(fuchsia-overloaded-operator, readability-named-parameter)
constexpr auto operator+(const Defer, Fn function) noexcept -> ScopeExit<Fn> {
    return ScopeExit<Fn>{std::move(function)};
}

} // namespace detail

} // namespace core

// NOLINTBEGIN(cppcoreguidelines-macro-usage, readability-identifier-naming)

#define DETAIL_CONCAT_IMPL(left, right) left##right
#define DETAIL_CONCAT(left, right) DETAIL_CONCAT_IMPL(left, right)

//
// Usage:
//
// defer({
//     ...
// });
//
#define defer(...)                                                             \
    [[maybe_unused]] const auto DETAIL_CONCAT(deferred_action_, __COUNTER__) { \
        ::core::detail::Defer{} + [&]() noexcept -> void __VA_ARGS__           \
    }

// NOLINTEND(cppcoreguidelines-macro-usage, readability-identifier-naming)

#pragma once

#include "panic.hpp"
#include <format>
#include <sstream>

namespace core {

namespace detail {

template<typename Value>
auto format_assertion_value(const Value& value) -> std::string {
    if constexpr (std::formattable<Value, char>) {
        return std::format("{}", value);
    } else if constexpr (
        std::is_pointer_v<Value>
        && std::is_object_v<std::remove_pointer_t<Value>>
    ) {
        return std::format(
            "{}",
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
            const_cast<const void*>(static_cast<const volatile void*>(value))
        );
    } else if constexpr (requires(std::ostream& stream) { stream << value; }) {
        auto stream{std::ostringstream{}};
        stream << value;
        return stream.str();
    } else {
        return std::format("<unprintable {}>", typeid(Value).name());
    }
}

} // namespace detail

constexpr auto assert_that(
    const bool c /* condition */,
    const std::source_location location = std::source_location::current()
) -> void {
    if (!c) {
        panic("assertion failed", location);
    }
}

template<typename Left, typename Right>
    requires requires(const Left& l, const Right& r) {
        { l == r } -> std::convertible_to<bool>;
    }
constexpr auto assert_eq(
    const Left& l,
    const Right& r,
    const std::source_location location = std::source_location::current()
) -> void {
    if (l == r) {
        return;
    }

    panic(
        std::format(
            "assertion failed: `(left == right)`\n  left: {}\n right: {}",
            detail::format_assertion_value(l),
            detail::format_assertion_value(r)
        ),
        location
    );
}

template<typename Left, typename Right>
    requires requires(const Left& l, const Right& r) {
        { l != r } -> std::convertible_to<bool>;
    }
constexpr auto assert_ne(
    const Left& l,
    const Right& r,
    const std::source_location location = std::source_location::current()
) -> void {
    if (l != r) {
        return;
    }

    panic(
        std::format(
            "assertion failed: `(left != right)`\n  left: {}\n right: {}",
            detail::format_assertion_value(l),
            detail::format_assertion_value(r)
        ),
        location
    );
}

} // namespace core

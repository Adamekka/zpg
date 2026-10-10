#pragma once

#include "../math/matrix.hpp"
#include <concepts>

namespace transform {

template<typename T>
concept Component = requires(const T& value) {
    { value.get_matrix() } -> std::same_as<math::Matrix<float, 4, 4>>;
};

namespace detail {

constexpr auto assert_finite(const math::Vector<float> value) -> void {
    core::assert_that(
        std::isfinite(value.x) && std::isfinite(value.y)
        && std::isfinite(value.z)
    );
}

} // namespace detail

} // namespace transform

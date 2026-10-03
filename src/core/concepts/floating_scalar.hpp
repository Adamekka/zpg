#pragma once

#include <concepts>

namespace core::concepts {

template<typename T>
concept FloatingScalar = std::same_as<T, float> || std::same_as<T, double>;

} // namespace core::concepts

#pragma once

#include "../core/assert.hpp"
#include "../core/concepts/floating_scalar.hpp"

namespace math {

template<typename T>
    requires(core::concepts::FloatingScalar<T> || std::integral<T>)
class Vector final {
  public:
    T x;
    T y;
    T z;

    // MARK: Lifecycle

    constexpr Vector(const T x, const T y, const T z)
        : x{x}
        , y{y}
        , z{z} {}

    constexpr explicit Vector(const std::array<T, 3>& values)
        : x{values[0]}
        , y{values[1]}
        , z{values[2]} {}

    Vector(const Vector&) = default;
    Vector(Vector&&) = default;

    ~Vector() = default;

    auto operator=(const Vector&) -> Vector& = default;
    auto operator=(Vector&&) -> Vector& = default;

    // MARK: Operations

    constexpr auto normalize() -> void
        requires(core::concepts::FloatingScalar<T>)
    {
        const auto length{std::hypot(this->x, this->y, this->z)};

        core::assert_that(std::isfinite(length));
        core::assert_that(length > 0);

        this->x /= length;
        this->y /= length;
        this->z /= length;
    }

    [[nodiscard]] constexpr auto normalized() const -> Vector
        requires(core::concepts::FloatingScalar<T>)
    {
        auto result{*this};
        result.normalize();
        return result;
    }

    [[nodiscard]] constexpr auto dot(const Vector& other) const -> T {
        auto result{T{}};
        result += this->x * other.x;
        result += this->y * other.y;
        result += this->z * other.z;
        return result;
    }

    [[nodiscard]] constexpr auto cross(const Vector& other) const -> Vector {
        return Vector{
            (this->y * other.z) - (this->z * other.y),
            (this->z * other.x) - (this->x * other.z),
            (this->x * other.y) - (this->y * other.x)
        };
    }
};
} // namespace math

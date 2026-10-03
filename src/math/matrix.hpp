#pragma once

#include "vector.hpp"
#include <numbers>
#include <print>

namespace math {

template<core::concepts::FloatingScalar T, size_t Columns, size_t Rows>
    requires(Columns >= 2 && Columns <= 4 && Rows >= 2 && Rows <= 4)
class Matrix final {
  public:
    // OpenGL consumes this flat storage in column-major order with GL_FALSE.
    std::array<T, Columns * Rows> values;

    // MARK: Lifecycle

    constexpr explicit Matrix(const std::array<T, Columns * Rows> values)
        : values{values} {}

    constexpr explicit Matrix(
        const std::array<std::array<T, Rows>, Columns>& values
    )
        : values{[&values]() -> auto {
            auto flat{std::array<T, Columns * Rows>{}};
            for (size_t column{0}; column < Columns; ++column) {
                for (size_t row{0}; row < Rows; ++row) {
                    flat[(column * Rows) + row] = values[column][row];
                }
            }
            return flat;
        }()} {}

    Matrix(const Matrix&) = default;
    Matrix(Matrix&&) = default;

    ~Matrix() = default;

    auto operator=(const Matrix&) -> Matrix& = default;
    auto operator=(Matrix&&) -> Matrix& = default;

    // MARK: Factory methods

    [[nodiscard]] static consteval auto zero() -> Matrix {
        return Matrix{std::array<T, Columns * Rows>{}};
    }

    [[nodiscard]] static consteval auto identity() -> Matrix {
        auto values{std::array<T, Columns * Rows>{}};
        for (size_t column{0}; column < Columns; ++column) {
            for (size_t row{0}; row < Rows; ++row) {
                values[(column * Rows) + row] = (column == row) ? 1 : 0;
            }
        }
        return Matrix{values};
    }

    [[nodiscard]] static constexpr auto
    look_at(const Vector<T>& eye, const Vector<T>& center, const Vector<T>& up)
        -> Matrix
        requires(Columns == 4 && Rows == 4)
    {
        const auto forward{Vector<T>{
            {center.x - eye.x, center.y - eye.y, center.z - eye.z}
        }.normalized()};

        const auto side{forward.cross(up).normalized()};
        const auto corrected_up{side.cross(forward).normalized()};

        const auto translation_x{-side.dot(eye)};
        const auto translation_y{-corrected_up.dot(eye)};
        const auto translation_z{forward.dot(eye)};

        // clang-format off
        return Matrix{std::array<T, Columns * Rows>{
            side.x,        corrected_up.x,  -forward.x,    0,
            side.y,        corrected_up.y,  -forward.y,    0,
            side.z,        corrected_up.z,  -forward.z,    0,
            translation_x, translation_y,   translation_z, 1
        }};
        // clang-format on
    }

    [[nodiscard]] static constexpr auto
    orthographic(const T left, const T right, const T bottom, const T top)
        -> Matrix
        requires(Columns == 4 && Rows == 4)
    {
        return orthographic(
            left, right, bottom, top, static_cast<T>(-1), static_cast<T>(1)
        );
    }

    [[nodiscard]] static constexpr auto orthographic(
        const T left,
        const T right,
        const T bottom,
        const T top,
        const T z_near,
        const T z_far
    ) -> Matrix
        requires(Columns == 4 && Rows == 4)
    {
        const auto width{right - left};
        const auto height{top - bottom};
        const auto depth{z_far - z_near};

        core::assert_ne(width, 0);
        core::assert_ne(height, 0);
        core::assert_ne(depth, 0);

        // clang-format off
        return Matrix{std::array<T, Columns * Rows>{
            static_cast<T>(2) / width, 0,                          0,                          0,
            0,                         static_cast<T>(2) / height, 0,                          0,
            0,                         0,                          static_cast<T>(-2) / depth, 0,
            -(right + left) / width,   -(top + bottom) / height,   -(z_far + z_near) / depth,  1
        }};
        // clang-format on
    }

    [[nodiscard]] static constexpr auto perspective(
        const T fov_y_radians,
        const T aspect_ratio,
        const T z_near,
        const T z_far
    ) -> Matrix
        requires(Columns == 4 && Rows == 4)
    {
        // Use perspective_relaxed for intentional nonstandard projections
        core::assert_that(fov_y_radians > 0);
        core::assert_that(fov_y_radians < std::numbers::pi_v<T>);
        core::assert_that(aspect_ratio > 0);
        core::assert_that(z_near > 0);
        core::assert_that(z_far > z_near);

        return perspective_relaxed(fov_y_radians, aspect_ratio, z_near, z_far);
    }

    [[nodiscard]] static constexpr auto perspective_relaxed(
        const T fov_y_radians,
        const T aspect_ratio,
        const T z_near,
        const T z_far
    ) -> Matrix
        requires(Columns == 4 && Rows == 4)
    {
        core::assert_ne(aspect_ratio, 0);
        core::assert_ne(z_near, 0);
        core::assert_ne(z_far, 0);

        const auto tangent{std::tan(fov_y_radians / static_cast<T>(2))};
        const auto depth{z_far - z_near};
        const auto horizontal_denominator{aspect_ratio * tangent};

        core::assert_that(std::isfinite(tangent));
        core::assert_ne(tangent, 0);
        core::assert_that(std::isfinite(horizontal_denominator));
        core::assert_ne(horizontal_denominator, 0);
        core::assert_that(std::isfinite(depth));
        core::assert_ne(depth, 0);

        const auto horizontal_scale{static_cast<T>(1) / horizontal_denominator};
        const auto vertical_scale{static_cast<T>(1) / tangent};
        const auto depth_scale{-(z_far + z_near) / depth};
        const auto depth_translation{
            -(static_cast<T>(2) * z_far * z_near) / depth
        };

        core::assert_that(std::isfinite(horizontal_scale));
        core::assert_ne(horizontal_scale, 0);
        core::assert_that(std::isfinite(vertical_scale));
        core::assert_ne(vertical_scale, 0);
        core::assert_that(std::isfinite(depth_scale));
        core::assert_that(std::isfinite(depth_translation));
        core::assert_ne(depth_translation, 0);

        // clang-format off
        return Matrix{std::array<T, Columns * Rows>{
            horizontal_scale, 0,              0,                 0,
            0,                vertical_scale, 0,                 0,
            0,                0,              depth_scale,       -1,
            0,                0,              depth_translation, 0
        }};
        // clang-format on
    }

    // MARK: Getters

    template<size_t Column, size_t Row>
    [[nodiscard]] constexpr auto get() -> T& {
        static_assert(Column < Columns, "Matrix column index is out of bounds");
        static_assert(Row < Rows, "Matrix row index is out of bounds");
        return this->values[(Column * Rows) + Row];
    }

    template<size_t Column, size_t Row>
    [[nodiscard]] constexpr auto get() const -> const T& {
        static_assert(Column < Columns, "Matrix column index is out of bounds");
        static_assert(Row < Rows, "Matrix row index is out of bounds");
        return this->values[(Column * Rows) + Row];
    }

    // MARK: Multiplication

    // NOLINTBEGIN(fuchsia-overloaded-operator)

    template<size_t RightColumns>
    [[nodiscard]] constexpr auto
    operator*(const Matrix<T, RightColumns, Columns>& right) const
        -> Matrix<T, RightColumns, Rows> {
        auto result{Matrix<T, RightColumns, Rows>::zero()};
        for (size_t column{0}; column < RightColumns; ++column) {
            for (size_t row{0}; row < Rows; ++row) {
                for (size_t shared{0}; shared < Columns; ++shared) {
                    result.values[(column * Rows) + row]
                        += this->values[(shared * Rows) + row]
                         * right.values[(column * Columns) + shared];
                }
            }
        }
        return result;
    }

    [[nodiscard]] constexpr auto
    operator*(const std::array<T, Columns>& vector) const
        -> std::array<T, Rows> {
        auto result{std::array<T, Rows>{}};
        for (size_t row{0}; row < Rows; ++row) {
            for (size_t column{0}; column < Columns; ++column) {
                result[row]
                    += this->values[(column * Rows) + row] * vector[column];
            }
        }
        return result;
    }

    [[nodiscard]] constexpr auto operator*(const T scalar) const -> Matrix {
        auto result{*this};
        result *= scalar;
        return result;
    }

    constexpr auto operator*=(const Matrix<T, Columns, Columns>& right)
        -> Matrix& {
        *this = *this * right;
        return *this;
    }

    constexpr auto operator*=(const T scalar) -> Matrix& {
        for (auto& value : this->values) {
            value *= scalar;
        }
        return *this;
    }

    [[nodiscard]] friend constexpr auto
    operator*(const std::array<T, Rows>& vector, const Matrix& matrix)
        -> std::array<T, Columns> {
        auto result{std::array<T, Columns>{}};
        for (size_t column{0}; column < Columns; ++column) {
            for (size_t row{0}; row < Rows; ++row) {
                result[column]
                    += vector[row] * matrix.values[(column * Rows) + row];
            }
        }
        return result;
    }

    [[nodiscard]] friend constexpr auto
    operator*(const T scalar, const Matrix& matrix) -> Matrix {
        return matrix * scalar;
    }

    // NOLINTEND(fuchsia-overloaded-operator)

    // MARK: Transformations

    constexpr auto scale(const std::array<T, Rows - 1>& factors) -> void
        requires(Columns == Rows && (Rows == 3 || Rows == 4))
    {
        for (size_t column{0}; column < Columns - 1; ++column) {
            for (size_t row{0}; row < Rows; ++row) {
                this->values[(column * Rows) + row] *= factors[column];
            }
        }
    }

    constexpr auto rotate(const T radians) -> void
        requires(Columns == 3 && Rows == 3)
    {
        const auto cosine{std::cos(radians)};
        const auto sine{std::sin(radians)};
        for (size_t row{0}; row < Rows; ++row) {
            const auto x{this->values[row]};
            const auto y{this->values[Rows + row]};
            this->values[row] = (x * cosine) + (y * sine);
            this->values[Rows + row] = (y * cosine) - (x * sine);
        }
    }

    constexpr auto rotate(const T radians, const Vector<T>& axis) -> void
        requires(Columns == 4 && Rows == 4)
    {
        const auto axis_length{std::hypot(axis.x, axis.y, axis.z)};
        core::assert_that(std::isfinite(axis_length));
        core::assert_that(axis_length > 0);

        const auto x{axis.x / axis_length};
        const auto y{axis.y / axis_length};
        const auto z{axis.z / axis_length};
        const auto cosine{std::cos(radians)};
        const auto sine{std::sin(radians)};
        const auto one_minus_cosine{static_cast<T>(1) - cosine};

        for (size_t row{0}; row < Rows; ++row) {
            const auto old_x{this->values[row]};
            const auto old_y{this->values[Rows + row]};
            const auto old_z{this->values[(2 * Rows) + row]};
            this->values[row]
                = (old_x * (cosine + (one_minus_cosine * x * x)))
                + (old_y * ((one_minus_cosine * x * y) + (sine * z)))
                + (old_z * ((one_minus_cosine * x * z) - (sine * y)));
            this->values[Rows + row]
                = (old_x * ((one_minus_cosine * x * y) - (sine * z)))
                + (old_y * (cosine + (one_minus_cosine * y * y)))
                + (old_z * ((one_minus_cosine * y * z) + (sine * x)));
            this->values[(2 * Rows) + row]
                = (old_x * ((one_minus_cosine * x * z) + (sine * y)))
                + (old_y * ((one_minus_cosine * y * z) - (sine * x)))
                + (old_z * (cosine + (one_minus_cosine * z * z)));
        }
    }

    constexpr auto rotate_x(const T radians) -> void
        requires(Columns == 4 && Rows == 4)
    {
        const auto cosine{std::cos(radians)};
        const auto sine{std::sin(radians)};
        for (size_t row{0}; row < Rows; ++row) {
            const auto y{this->values[Rows + row]};
            const auto z{this->values[(2 * Rows) + row]};
            this->values[Rows + row] = (y * cosine) + (z * sine);
            this->values[(2 * Rows) + row] = (z * cosine) - (y * sine);
        }
    }

    constexpr auto rotate_y(const T radians) -> void
        requires(Columns == 4 && Rows == 4)
    {
        const auto cosine{std::cos(radians)};
        const auto sine{std::sin(radians)};
        for (size_t row{0}; row < Rows; ++row) {
            const auto x{this->values[row]};
            const auto z{this->values[(2 * Rows) + row]};
            this->values[row] = (x * cosine) - (z * sine);
            this->values[(2 * Rows) + row] = (x * sine) + (z * cosine);
        }
    }

    constexpr auto rotate_z(const T radians) -> void
        requires(Columns == 4 && Rows == 4)
    {
        const auto cosine{std::cos(radians)};
        const auto sine{std::sin(radians)};
        for (size_t row{0}; row < Rows; ++row) {
            const auto x{this->values[row]};
            const auto y{this->values[Rows + row]};
            this->values[row] = (x * cosine) + (y * sine);
            this->values[Rows + row] = (y * cosine) - (x * sine);
        }
    }

    constexpr auto translate(const std::array<T, Rows - 1>& offset) -> void
        requires(Columns == Rows && (Rows == 3 || Rows == 4))
    {
        for (size_t row{0}; row < Rows; ++row) {
            auto& translated_value{this->values[((Columns - 1) * Rows) + row]};
            for (size_t column{0}; column < Columns - 1; ++column) {
                translated_value
                    += this->values[(column * Rows) + row] * offset[column];
            }
        }
    }

    // MARK: String conversion

    [[nodiscard]] auto to_string() const -> std::string {
        auto result{std::format(
            "{}mat{}x{}(\n", std::same_as<T, double> ? "d" : "", Columns, Rows
        )};
        for (size_t column{0}; column < Columns; ++column) {
            result += "    (";
            for (size_t row{0}; row < Rows; ++row) {
                result += std::format(
                    "{}{:f}",
                    row == 0 ? "" : ", ",
                    this->values[(column * Rows) + row]
                );
            }
            result += column == Columns - 1 ? ")\n" : "),\n";
        }
        result += ')';
        return result;
    }

    auto print() const -> void {
        std::println("{}", this->to_string());
    }
};

} // namespace math

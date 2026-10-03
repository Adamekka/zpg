#pragma once

#include "../../math/matrix.hpp"
#include "shader_var_type.hpp"

namespace object::shader {

namespace detail {

template<typename T> struct ShaderDataTraits;

// NOLINTBEGIN(bugprone-macro-parentheses, cppcoreguidelines-macro-usage)

#define SHADER_SCALAR(TYPE, ENUM, FUNCTION)                                    \
    template<> struct ShaderDataTraits<TYPE> final {                           \
        static constexpr auto SHADER_TYPE{ShaderVarType::Value::ENUM};         \
                                                                               \
        static auto pass(const int32_t location, const TYPE data) -> void {    \
            FUNCTION(location, data);                                          \
        }                                                                      \
    };

#define SHADER_VECTOR(TYPE, SIZE, ENUM, FUNCTION)                              \
    template<> struct ShaderDataTraits<std::array<TYPE, SIZE>> final {         \
        static constexpr auto SHADER_TYPE{ShaderVarType::Value::ENUM};         \
                                                                               \
        static auto                                                            \
        pass(const int32_t location, const std::array<TYPE, SIZE>& data)       \
            -> void {                                                          \
            FUNCTION(location, 1, data.data());                                \
        }                                                                      \
    };

#define SHADER_MATRIX(TYPE, COLUMNS, ROWS, ENUM, FUNCTION)                     \
    template<>                                                                 \
    struct ShaderDataTraits<math::Matrix<TYPE, COLUMNS, ROWS>> final {         \
        using Data = math::Matrix<TYPE, COLUMNS, ROWS>;                        \
                                                                               \
        static constexpr auto SHADER_TYPE{ShaderVarType::Value::ENUM};         \
        static_assert(sizeof(Data) == sizeof(decltype(Data::values)));         \
                                                                               \
        static auto pass(const int32_t location, const Data& data) -> void {   \
            FUNCTION(location, 1, GL_FALSE, data.values.data());               \
        }                                                                      \
    };

// NOLINTEND(bugprone-macro-parentheses, cppcoreguidelines-macro-usage)

SHADER_SCALAR(float, Float, glUniform1f)
SHADER_VECTOR(float, 2, Vec2, glUniform2fv)
SHADER_VECTOR(float, 3, Vec3, glUniform3fv)
SHADER_VECTOR(float, 4, Vec4, glUniform4fv)

SHADER_MATRIX(float, 2, 2, Mat2, glUniformMatrix2fv)
SHADER_MATRIX(float, 3, 3, Mat3, glUniformMatrix3fv)
SHADER_MATRIX(float, 4, 4, Mat4, glUniformMatrix4fv)
SHADER_MATRIX(float, 2, 3, Mat2x3, glUniformMatrix2x3fv)
SHADER_MATRIX(float, 2, 4, Mat2x4, glUniformMatrix2x4fv)
SHADER_MATRIX(float, 3, 2, Mat3x2, glUniformMatrix3x2fv)
SHADER_MATRIX(float, 3, 4, Mat3x4, glUniformMatrix3x4fv)
SHADER_MATRIX(float, 4, 2, Mat4x2, glUniformMatrix4x2fv)
SHADER_MATRIX(float, 4, 3, Mat4x3, glUniformMatrix4x3fv)

SHADER_SCALAR(int32_t, Int, glUniform1i)
SHADER_VECTOR(int32_t, 2, IVec2, glUniform2iv)
SHADER_VECTOR(int32_t, 3, IVec3, glUniform3iv)
SHADER_VECTOR(int32_t, 4, IVec4, glUniform4iv)

SHADER_SCALAR(uint32_t, UInt, glUniform1ui)
SHADER_VECTOR(uint32_t, 2, UVec2, glUniform2uiv)
SHADER_VECTOR(uint32_t, 3, UVec3, glUniform3uiv)
SHADER_VECTOR(uint32_t, 4, UVec4, glUniform4uiv)

SHADER_SCALAR(double, Double, glUniform1d)
SHADER_VECTOR(double, 2, DVec2, glUniform2dv)
SHADER_VECTOR(double, 3, DVec3, glUniform3dv)
SHADER_VECTOR(double, 4, DVec4, glUniform4dv)

SHADER_MATRIX(double, 2, 2, DMat2, glUniformMatrix2dv)
SHADER_MATRIX(double, 3, 3, DMat3, glUniformMatrix3dv)
SHADER_MATRIX(double, 4, 4, DMat4, glUniformMatrix4dv)
SHADER_MATRIX(double, 2, 3, DMat2x3, glUniformMatrix2x3dv)
SHADER_MATRIX(double, 2, 4, DMat2x4, glUniformMatrix2x4dv)
SHADER_MATRIX(double, 3, 2, DMat3x2, glUniformMatrix3x2dv)
SHADER_MATRIX(double, 3, 4, DMat3x4, glUniformMatrix3x4dv)
SHADER_MATRIX(double, 4, 2, DMat4x2, glUniformMatrix4x2dv)
SHADER_MATRIX(double, 4, 3, DMat4x3, glUniformMatrix4x3dv)

template<typename T>
    requires requires(const int32_t location, const std::array<T, 3>& data) {
        ShaderDataTraits<std::array<T, 3>>::SHADER_TYPE;
        ShaderDataTraits<std::array<T, 3>>::pass(location, data);
    }
struct ShaderDataTraits<math::Vector<T>> final {
    using ArrayData = std::array<T, 3>;

    static constexpr auto SHADER_TYPE{ShaderDataTraits<ArrayData>::SHADER_TYPE};

    static auto pass(const int32_t location, const math::Vector<T>& data)
        -> void {
        ShaderDataTraits<ArrayData>::pass(
            location, ArrayData{data.x, data.y, data.z}
        );
    }
};

#undef SHADER_MATRIX
#undef SHADER_SCALAR
#undef SHADER_VECTOR

template<typename T>
concept ShaderData = requires(const int32_t location, const T& data) {
    ShaderDataTraits<T>::SHADER_TYPE;
    ShaderDataTraits<T>::pass(location, data);
};

} // namespace detail

class ShaderArg final {
  public:
    constexpr ShaderArg(
        const ShaderVarType type,
        const int32_t location,
        const int32_t array_size
    )
        : type{type}
        , location{location}
        , array_size{array_size} {}

    ShaderArg(const ShaderArg&) = default;
    ShaderArg(ShaderArg&&) = default;

    ~ShaderArg() = default;

    auto operator=(const ShaderArg&) -> ShaderArg& = default;
    auto operator=(ShaderArg&&) -> ShaderArg& = default;

    template<detail::ShaderData T>
    auto set_uniform(const T& data) const -> void {
        core::assert_eq(
            this->type.value, detail::ShaderDataTraits<T>::SHADER_TYPE
        );
        detail::ShaderDataTraits<T>::pass(this->location, data);
    }

    [[nodiscard]] constexpr auto get_type() const -> ShaderVarType {
        return this->type;
    }

    [[nodiscard]] constexpr auto get_location() const -> int32_t {
        return this->location;
    }

    [[nodiscard]] constexpr auto get_array_size() const -> int32_t {
        return this->array_size;
    }

  private:
    ShaderVarType type;
    int32_t location;
    int32_t array_size;
};

} // namespace object::shader

#pragma once

#include "../../gl.hpp"
#include <string_view>

namespace object::shader {

struct ShaderVarType final {
    enum class Value : uint16_t {
        // Floating-point scalars and vectors
        Float = GL_FLOAT,
        Vec2 = GL_FLOAT_VEC2,
        Vec3 = GL_FLOAT_VEC3,
        Vec4 = GL_FLOAT_VEC4,

        // Floating-point matrices
        Mat2 = GL_FLOAT_MAT2,
        Mat3 = GL_FLOAT_MAT3,
        Mat4 = GL_FLOAT_MAT4,
        Mat2x3 = GL_FLOAT_MAT2x3,
        Mat2x4 = GL_FLOAT_MAT2x4,
        Mat3x2 = GL_FLOAT_MAT3x2,
        Mat3x4 = GL_FLOAT_MAT3x4,
        Mat4x2 = GL_FLOAT_MAT4x2,
        Mat4x3 = GL_FLOAT_MAT4x3,

        // Signed integer scalars and vectors
        Int = GL_INT,
        IVec2 = GL_INT_VEC2,
        IVec3 = GL_INT_VEC3,
        IVec4 = GL_INT_VEC4,

        // Unsigned integer scalars and vectors
        UInt = GL_UNSIGNED_INT,
        UVec2 = GL_UNSIGNED_INT_VEC2,
        UVec3 = GL_UNSIGNED_INT_VEC3,
        UVec4 = GL_UNSIGNED_INT_VEC4,

        // Double-precision scalars and vectors
        Double = GL_DOUBLE,
        DVec2 = GL_DOUBLE_VEC2,
        DVec3 = GL_DOUBLE_VEC3,
        DVec4 = GL_DOUBLE_VEC4,

        // Double-precision matrices
        DMat2 = GL_DOUBLE_MAT2,
        DMat3 = GL_DOUBLE_MAT3,
        DMat4 = GL_DOUBLE_MAT4,
        DMat2x3 = GL_DOUBLE_MAT2x3,
        DMat2x4 = GL_DOUBLE_MAT2x4,
        DMat3x2 = GL_DOUBLE_MAT3x2,
        DMat3x4 = GL_DOUBLE_MAT3x4,
        DMat4x2 = GL_DOUBLE_MAT4x2,
        DMat4x3 = GL_DOUBLE_MAT4x3,
    } value;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr ShaderVarType(Value value)
        : value{value} {}

    template<typename T>
    constexpr explicit ShaderVarType(T value)
        : value{static_cast<Value>(value)} {}

    [[nodiscard]] constexpr auto description() const -> std::string_view {
        switch (this->value) {
            case Value::Float: return "float";
            case Value::Vec2: return "vec2";
            case Value::Vec3: return "vec3";
            case Value::Vec4: return "vec4";

            case Value::Mat2: return "mat2";
            case Value::Mat3: return "mat3";
            case Value::Mat4: return "mat4";
            case Value::Mat2x3: return "mat2x3";
            case Value::Mat2x4: return "mat2x4";
            case Value::Mat3x2: return "mat3x2";
            case Value::Mat3x4: return "mat3x4";
            case Value::Mat4x2: return "mat4x2";
            case Value::Mat4x3: return "mat4x3";

            case Value::Int: return "int";
            case Value::IVec2: return "ivec2";
            case Value::IVec3: return "ivec3";
            case Value::IVec4: return "ivec4";

            case Value::UInt: return "uint";
            case Value::UVec2: return "uvec2";
            case Value::UVec3: return "uvec3";
            case Value::UVec4: return "uvec4";

            case Value::Double: return "double";
            case Value::DVec2: return "dvec2";
            case Value::DVec3: return "dvec3";
            case Value::DVec4: return "dvec4";

            case Value::DMat2: return "dmat2";
            case Value::DMat3: return "dmat3";
            case Value::DMat4: return "dmat4";
            case Value::DMat2x3: return "dmat2x3";
            case Value::DMat2x4: return "dmat2x4";
            case Value::DMat3x2: return "dmat3x2";
            case Value::DMat3x4: return "dmat3x4";
            case Value::DMat4x2: return "dmat4x2";
            case Value::DMat4x3: return "dmat4x3";
        }

        std::unreachable();
    }
};

} // namespace object::shader

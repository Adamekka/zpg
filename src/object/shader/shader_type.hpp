#pragma once

#include "../../gl.hpp"

namespace object::shader {

struct ShaderType final {
    enum class Value : uint16_t {
        Vertex = GL_VERTEX_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
    } value;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr ShaderType(Value value)
        : value{value} {}

    [[nodiscard]] constexpr auto get() const -> uint16_t {
        return static_cast<uint16_t>(this->value);
    }
};

} // namespace object::shader

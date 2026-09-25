#pragma once

#include "../../gl.hpp"

namespace object::mesh {

struct MeshDrawMode final {
    enum class Value : uint8_t {
        Triangles = GL_TRIANGLES,
        TriangleStrip = GL_TRIANGLE_STRIP,
    } value;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr MeshDrawMode(Value value)
        : value{value} {}

    template<typename T>
    constexpr explicit MeshDrawMode(T value)
        : value{static_cast<Value>(value)} {}

    [[nodiscard]] constexpr auto get() const -> uint8_t {
        return static_cast<uint8_t>(this->value);
    }
};

} // namespace object::mesh

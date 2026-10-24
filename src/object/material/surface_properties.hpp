#pragma once

#include <array>
#include <cstdint>

namespace object::material {

enum class AlphaMode : uint8_t {
    // Values are shared with the material shader's alpha_mode uniform.
    Opaque = 0,
    Blend = 2,
};

struct NoTexture final {};

struct SurfaceProperties final {
    std::array<float, 4> color;
    std::array<float, 3> specular_color;
    std::array<float, 3> emissive_color;
    float shininess;
    AlphaMode alpha_mode;
};

} // namespace object::material

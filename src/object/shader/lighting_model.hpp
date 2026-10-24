#pragma once

#include <cstdint>

namespace object::shader {

enum class LightingModel : uint8_t {
    Ambient,
    Lambert,
    Phong,
    BlinnPhong,
};

} // namespace object::shader

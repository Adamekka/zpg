#pragma once

#include "../material/surface_properties.hpp"
#include <cstddef>
#include <variant>

namespace object::importer {

using TextureIndex = std::variant<material::NoTexture, size_t>;

struct ImportedMaterial final {
    material::SurfaceProperties properties;
    TextureIndex color_texture;
    TextureIndex emissive_texture;
};

} // namespace object::importer

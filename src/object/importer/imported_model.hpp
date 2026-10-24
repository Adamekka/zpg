#pragma once

#include "imported_material.hpp"
#include "imported_submesh.hpp"
#include "imported_texture.hpp"

namespace object::importer {

struct ImportedModel final {
    std::vector<ImportedTexture> textures;
    std::vector<ImportedMaterial> materials;
    std::vector<ImportedSubmesh> submeshes;
};

} // namespace object::importer

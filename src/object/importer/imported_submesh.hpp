#pragma once

#include "../../math/vector.hpp"
#include <vector>

namespace object::importer {

struct ImportedSubmesh final {
    // Packed position, RGBA color, normal, and UV match runtime attribute
    // order.
    std::vector<float> vertices;
    size_t material_index;
    math::Vector<float> center;
};

} // namespace object::importer

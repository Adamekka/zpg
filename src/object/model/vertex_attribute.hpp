#pragma once

#include <cstdint>

namespace object::model {

struct VertexAttribute final {
    int32_t component_count;

    constexpr explicit VertexAttribute(const int32_t component_count)
        : component_count{component_count} {}
};

} // namespace object::model

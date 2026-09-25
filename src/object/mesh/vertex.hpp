#pragma once

#include "color.hpp"
#include "position.hpp"
#include <boost/hana/define_struct.hpp> // IWYU pragma: export

namespace object::mesh {

struct Vertex final {
    BOOST_HANA_DEFINE_STRUCT(Vertex, (Position, position), (Color, color));

    Vertex() = default;

    constexpr Vertex(const Position position, const Color color)
        : position{position}
        , color{color} {}
};

} // namespace object::mesh

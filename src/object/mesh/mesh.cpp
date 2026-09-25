#include "mesh.hpp"
#include "../../core/assert.hpp"
#include "../../gl.hpp"

namespace object::mesh {

auto Mesh::from_raw_data(
    const std::span<const float> data, const MeshDrawMode draw_mode
) -> Mesh {
    constexpr auto NUMBER_OF_PROPERTIES{Position::DIMENSION + Color::DIMENSION};
    static_assert(NUMBER_OF_PROPERTIES == size_t{6});

    core::assert_eq(data.size() % NUMBER_OF_PROPERTIES, size_t{0});

    auto vertices{std::vector<Vertex>{}};
    vertices.reserve(data.size() / NUMBER_OF_PROPERTIES);

    for (auto index{size_t{0}}; index < data.size();
         index += NUMBER_OF_PROPERTIES) {
        vertices.emplace_back(
            Position{data[index], data[index + 1], data[index + 2]},
            Color{data[index + 3], data[index + 4], data[index + 5]}
        );
    }

    return Mesh{std::move(vertices), draw_mode};
}

auto Mesh::draw() const -> void {
    this->vao.bind();

    glDrawArrays(
        this->draw_mode.get(), 0, static_cast<int32_t>(this->vertices.size())
    );
}

} // namespace object::mesh

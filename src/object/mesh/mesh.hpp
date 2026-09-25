#pragma once

#include "mesh_draw_mode.hpp"
#include "vao.hpp"
#include <vector>

namespace object::mesh {

class Mesh final {
  public:
    constexpr explicit Mesh(
        const std::span<const Vertex> vertices, const MeshDrawMode draw_mode
    )
        : vertices{vertices.begin(), vertices.end()}
        , draw_mode{draw_mode} {}

    explicit Mesh(std::vector<Vertex>&& vertices, const MeshDrawMode draw_mode)
        : vertices{std::move(vertices)}
        , draw_mode{draw_mode} {}

    Mesh(const Mesh&) = delete;
    Mesh(Mesh&&) = default;

    ~Mesh() = default;

    auto operator=(const Mesh&) -> Mesh& = delete;
    auto operator=(Mesh&&) -> Mesh& = default;

    [[nodiscard]] static auto
    from_raw_data(std::span<const float> data, MeshDrawMode draw_mode) -> Mesh;

    auto draw() const -> void;

  private:
    std::vector<Vertex> vertices;
    VBO vbo{this->vertices};
    VAO vao{this->vbo};

    MeshDrawMode draw_mode;
};

} // namespace object::mesh

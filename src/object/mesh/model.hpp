#pragma once

#include "mesh_draw_mode.hpp"
#include "vao.hpp"
#include <vector>

namespace object::mesh {

class Model final {
  public:
    constexpr explicit Model(
        const std::span<const Vertex> vertices, const MeshDrawMode draw_mode
    )
        : vertices{vertices.begin(), vertices.end()}
        , draw_mode{draw_mode} {}

    explicit Model(std::vector<Vertex>&& vertices, const MeshDrawMode draw_mode)
        : vertices{std::move(vertices)}
        , draw_mode{draw_mode} {}

    Model(const Model&) = delete;
    Model(Model&&) = default;

    ~Model() = default;

    auto operator=(const Model&) -> Model& = delete;
    auto operator=(Model&&) -> Model& = default;

    [[nodiscard]] static auto
    from_raw_data(std::span<const float> data, MeshDrawMode draw_mode) -> Model;

    auto draw() const -> void;

  private:
    std::vector<Vertex> vertices;
    VBO vbo{this->vertices};
    VAO vao{this->vbo};

    MeshDrawMode draw_mode;
};

} // namespace object::mesh

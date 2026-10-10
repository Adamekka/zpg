#pragma once

#include "mesh_draw_mode.hpp"
#include "vao.hpp"

namespace object::model {

class Model final {
  public:
    Model(const Model&) = delete;
    Model(Model&&) = default;

    ~Model() = default;

    auto operator=(const Model&) -> Model& = delete;
    auto operator=(Model&&) -> Model& = default;

    // Attribute order matches packed data order and shader input locations.
    [[nodiscard]] static auto from_raw_data(
        std::span<const float> data,
        std::span<const VertexAttribute> attributes,
        MeshDrawMode draw_mode
    ) -> Model;

    auto draw() const -> void;

  private:
    int32_t vertex_count;
    VBO vbo;
    VAO vao;

    MeshDrawMode draw_mode;

    Model(
        std::span<const float> data,
        std::span<const VertexAttribute> attributes,
        size_t floats_per_vertex,
        MeshDrawMode draw_mode
    );
};

} // namespace object::model

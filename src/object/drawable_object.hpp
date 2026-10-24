#pragma once

#include "../transform/transform.hpp"
#include "material/material.hpp"
#include "model/model.hpp"

namespace object {

class DrawableObject final {
  public:
    transform::Transform transform;

    DrawableObject(
        model::Model mesh,
        shader::ShaderProgram& shader_program,
        transform::Transform transform
    );

    DrawableObject(const DrawableObject&) = delete;
    DrawableObject(DrawableObject&&) = default;

    ~DrawableObject() = default;

    auto operator=(const DrawableObject&) -> DrawableObject& = delete;
    auto operator=(DrawableObject&&) -> DrawableObject& = default;

    [[nodiscard]] static auto load(
        const std::filesystem::path& path,
        shader::ShaderProgram& shader_program,
        transform::Transform transform
    ) -> std::expected<DrawableObject, std::string>;

    auto draw(
        const math::Matrix<float, 4, 4>& view_matrix,
        const math::Matrix<float, 4, 4>& projection_matrix
    ) const -> void;

  private:
    struct Submesh final {
        model::Model mesh;
        size_t material_index;
        math::Vector<float> center;
    };

    // Finalize textures before materials capture pointers; moving their vector
    // preserves addresses, while subsequent growth would invalidate them.
    std::vector<material::texture::Texture> textures;
    std::vector<material::Material> materials;
    std::vector<Submesh> submeshes;

    DrawableObject(
        std::vector<material::texture::Texture> textures,
        std::vector<material::Material> materials,
        std::vector<Submesh> submeshes,
        transform::Transform transform
    );
};

} // namespace object

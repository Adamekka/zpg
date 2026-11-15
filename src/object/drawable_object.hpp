#pragma once

#include "../core/types/id.hpp"
#include "../transform/transform.hpp"
#include "draw_pass.hpp"
#include "material/material.hpp"
#include "model/model.hpp"
#include <boost/uuid/random_generator.hpp>

namespace object {

class DrawableObject final {
  public:
    core::types::ID id{boost::uuids::random_generator{}()};
    transform::Transform transform;

    DrawableObject(
        model::Model mesh,
        material::Material material,
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

    [[nodiscard]] static auto from_colored_data(
        std::span<const float> data,
        math::Vector<float> color,
        shader::ShaderProgram& shader,
        transform::Transform transform,
        float shininess,
        float specular_strength
    ) -> DrawableObject;

    auto draw(const math::Matrix<float, 4, 4>& view_matrix, DrawPass pass) const
        -> void;

    auto draw_selection(shader::ShaderProgram& shader) const -> void;

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

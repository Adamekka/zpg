#pragma once

#include "../shader/shader_program.hpp"
#include "surface_properties.hpp"
#include "texture/texture.hpp"
#include <variant>

namespace object::material {

class Material final {
  public:
    explicit Material(shader::ShaderProgram& shader_program);
    Material(
        shader::ShaderProgram& shader_program,
        const SurfaceProperties& properties,
        const texture::Texture& color_texture,
        const texture::Texture& emissive_texture
    );

    auto bind(
        const math::Matrix<float, 4, 4>& model_matrix,
        const math::Matrix<float, 4, 4>& view_matrix,
        const math::Matrix<float, 4, 4>& projection_matrix
    ) const -> void;

    [[nodiscard]] auto is_blended() const -> bool;

  private:
    struct VertexColors final {};

    struct Surface final {
        SurfaceProperties properties;
        const texture::Texture* color_texture;
        const texture::Texture* emissive_texture;
    };

    shader::ShaderProgram* shader_program;
    std::variant<VertexColors, Surface> surface;
};

} // namespace object::material

#pragma once

#include "../shader/shader_program.hpp"
#include "surface_properties.hpp"
#include "texture/texture.hpp"
#include <variant>

namespace object::material {

class Material final {
  public:
    struct ModelColors final {
        math::Vector<float> color;
    };

    struct TreeColors final {};

    using Coloring = std::variant<ModelColors, TreeColors>;

    Material(
        shader::ShaderProgram& shader_program,
        float shininess,
        float specular_strength
    );
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
    struct Untextured final {
        float shininess;
        float specular_strength;
    };

    struct Textured final {
        SurfaceProperties properties;
        const texture::Texture* color_texture;
        const texture::Texture* emissive_texture;
    };

    shader::ShaderProgram* shader_program;
    std::variant<Untextured, Textured> surface;
};

} // namespace object::material

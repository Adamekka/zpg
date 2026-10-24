#include "material.hpp"

namespace object::material {

Material::Material(shader::ShaderProgram& shader_program)
    : shader_program{&shader_program}
    , surface{VertexColors{}} {}

Material::Material(
    shader::ShaderProgram& shader_program,
    const SurfaceProperties& properties,
    const texture::Texture& color_texture,
    const texture::Texture& emissive_texture
)
    : shader_program{&shader_program}
    , surface{Surface{properties, &color_texture, &emissive_texture}} {
    core::assert_that(
        std::isfinite(properties.shininess) && properties.shininess >= 0.0f
    );
    for (const auto value : properties.color) {
        core::assert_that(
            std::isfinite(value) && value >= 0.0f && value <= 1.0f
        );
    }
    for (const auto& color :
         {properties.specular_color, properties.emissive_color}) {
        for (const auto value : color) {
            core::assert_that(std::isfinite(value) && value >= 0.0f);
        }
    }
    core::assert_that(
        properties.alpha_mode == AlphaMode::Opaque
        || properties.alpha_mode == AlphaMode::Blend
    );
}

auto Material::bind(
    const math::Matrix<float, 4, 4>& model_matrix,
    const math::Matrix<float, 4, 4>& view_matrix,
    const math::Matrix<float, 4, 4>& projection_matrix
) const -> void {
    this->shader_program->bind();
    const auto& args{this->shader_program->get_args()};
    args.at("model_matrix").set_uniform(model_matrix);
    args.at("view_matrix").set_uniform(view_matrix);
    args.at("projection_matrix").set_uniform(projection_matrix);

    if (const auto* const surface{std::get_if<Surface>(&this->surface)}) {
        args.at("base_color_factor").set_uniform(surface->properties.color);
        args.at("specular_color")
            .set_uniform(surface->properties.specular_color);
        args.at("emissive_color")
            .set_uniform(surface->properties.emissive_color);
        args.at("shininess").set_uniform(surface->properties.shininess);
        args.at("alpha_mode")
            .set_uniform(static_cast<int32_t>(surface->properties.alpha_mode));
        args.at("color_texture").set_texture_unit(0);
        args.at("emissive_texture").set_texture_unit(1);

        surface->color_texture->bind(0);
        surface->emissive_texture->bind(1);
    }
}

auto Material::is_blended() const -> bool {
    const auto* const surface{std::get_if<Surface>(&this->surface)};
    return surface != nullptr
        && surface->properties.alpha_mode == AlphaMode::Blend;
}

} // namespace object::material

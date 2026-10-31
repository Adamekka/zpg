#pragma once

#include "object/shader/shader_program.hpp"

class PointLight final {
  public:
    PointLight(
        const math::Vector<float> position,
        const math::Vector<float> color,
        const math::Vector<float> ambient,
        const math::Vector<float> attenuation
    )
        : position{position}
        , color{color}
        , ambient{ambient}
        , attenuation{attenuation} {
        this->set_position(position);
        for (const auto value :
             {color.x,
              color.y,
              color.z,
              ambient.x,
              ambient.y,
              ambient.z,
              attenuation.x,
              attenuation.y,
              attenuation.z}) {
            core::assert_that(std::isfinite(value) && value >= 0.0f);
        }
        core::assert_that(attenuation.x > 0.0f);
    }

    [[nodiscard]] auto get_position() const -> math::Vector<float> {
        return this->position;
    }

    auto set_position(const math::Vector<float> position) -> void {
        core::assert_that(
            std::isfinite(position.x) && std::isfinite(position.y)
            && std::isfinite(position.z)
        );
        this->position = position;
    }

    auto bind(object::shader::ShaderProgram& shader) const -> void {
        const auto& args{shader.get_args()};
        // Only set light uniforms that this shader uses.
        for (const auto& [name, value] : std::array{
                 std::pair{"light_position", this->position},
                 std::pair{"light_color", this->color},
                 std::pair{"ambient_color", this->ambient},
                 std::pair{"light_attenuation", this->attenuation}
             }) {
            if (const auto arg{args.find(name)}; arg != args.end()) {
                arg->second.set_uniform(value);
            }
        }
    }

  private:
    math::Vector<float> position;
    math::Vector<float> color;
    math::Vector<float> ambient;
    math::Vector<float> attenuation;
};

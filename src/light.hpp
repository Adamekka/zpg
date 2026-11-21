#pragma once

#include "object/shader/shader_program.hpp"
#include <format>
#include <span>

class Light final {
  public:
    // Values match the type constants in phong.frag.
    enum class Type : int32_t {
        Point = 0,
        Directional = 1,
        Spot = 2,
    };

    // How a spot light fades from its inner to its outer cone angle. Values
    // match the falloff constants in phong.frag.
    enum class Falloff : int32_t {
        Hard = 0,
        Smooth = 1,
    };

    // Must match MAX_LIGHTS in phong.frag.
    static constexpr auto MAX_COUNT{size_t{16}};

    Type type;
    math::Vector<float> position;
    // Direction the light shines in; unused by point lights.
    math::Vector<float> direction;
    math::Vector<float> color;
    math::Vector<float> ambient;
    // Constant, linear, and quadratic distance attenuation.
    math::Vector<float> attenuation;
    // Cosines of the cone half-angles; only spot lights use them.
    float inner_cutoff;
    float outer_cutoff;
    Falloff falloff;

    [[nodiscard]] static auto point(
        const math::Vector<float> position,
        const math::Vector<float> color,
        const math::Vector<float> ambient,
        const math::Vector<float> attenuation
    ) -> Light {
        return Light{
            Type::Point,
            position,
            // Point lights shine in all directions; any unit vector is valid.
            {0.0f, -1.0f, 0.0f},
            color,
            ambient,
            attenuation,
            -1.0f,
            -1.0f,
            Falloff::Hard
        };
    }

    [[nodiscard]] static auto directional(
        const math::Vector<float> direction,
        const math::Vector<float> color,
        const math::Vector<float> ambient
    ) -> Light {
        return Light{
            Type::Directional,
            {0.0f, 0.0f, 0.0f},
            direction,
            color,
            ambient,
            // Directional light is infinitely far away and does not fade.
            {1.0f, 0.0f, 0.0f},
            -1.0f,
            -1.0f,
            Falloff::Hard
        };
    }

    // Angles are cone half-angles in radians, measured from the direction.
    [[nodiscard]] static auto spot(
        const math::Vector<float> position,
        const math::Vector<float> direction,
        const math::Vector<float> color,
        const math::Vector<float> ambient,
        const math::Vector<float> attenuation,
        const float inner_angle,
        const float outer_angle,
        const Falloff falloff
    ) -> Light {
        core::assert_that(
            0.0f <= inner_angle && inner_angle < outer_angle
            && outer_angle < std::numbers::pi_v<float> / 2.0f
        );
        return Light{
            Type::Spot,
            position,
            direction,
            color,
            ambient,
            attenuation,
            std::cos(inner_angle),
            std::cos(outer_angle),
            falloff
        };
    }

    // Uploads all lights. Shaders written for one light use only the first.
    static auto
    bind(object::shader::ShaderProgram& shader, std::span<const Light> lights)
        -> void {
        core::assert_that(!lights.empty() && lights.size() <= MAX_COUNT);
        const auto& args{shader.get_args()};
        // Only set light uniforms that this shader uses.
        const auto set{[&args](const std::string& name, const auto value) {
            if (const auto arg{args.find(name)}; arg != args.end()) {
                arg->second.set_uniform(value);
            }
        }};
        set("light_position", lights.front().position);
        set("light_count", static_cast<int32_t>(lights.size()));
        for (auto index{size_t{0}}; index < lights.size(); ++index) {
            const auto& light{lights[index]};
            light.validate();
            const auto field{[index](const std::string_view name) {
                return std::format("lights[{}].{}", index, name);
            }};
            set(field("type"), static_cast<int32_t>(light.type));
            set(field("position"), light.position);
            set(field("direction"), light.direction.normalized());
            set(field("color"), light.color);
            set(field("ambient"), light.ambient);
            set(field("attenuation"), light.attenuation);
            set(field("inner_cutoff"), light.inner_cutoff);
            set(field("outer_cutoff"), light.outer_cutoff);
            set(field("falloff"), static_cast<int32_t>(light.falloff));
        }
    }

  private:
    // Fields are public so scenes can animate lights; check them on upload.
    auto validate() const -> void {
        for (const auto vector : {this->position, this->direction}) {
            core::assert_that(
                std::isfinite(vector.x) && std::isfinite(vector.y)
                && std::isfinite(vector.z)
            );
        }
        core::assert_that(
            this->direction.x != 0.0f || this->direction.y != 0.0f
            || this->direction.z != 0.0f
        );
        for (const auto vector :
             {this->color, this->ambient, this->attenuation}) {
            for (const auto value : {vector.x, vector.y, vector.z}) {
                core::assert_that(std::isfinite(value) && value >= 0.0f);
            }
        }
        core::assert_that(this->attenuation.x > 0.0f);
    }
};

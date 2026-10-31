#pragma once

#include "object/shader/lighting_model.hpp"
#include "scene.hpp"

class SceneFactory final {
  public:
    SceneFactory(const SceneFactory&) = delete;
    SceneFactory(SceneFactory&&) = default;

    ~SceneFactory() = default;

    auto operator=(const SceneFactory&) -> SceneFactory& = delete;
    auto operator=(SceneFactory&&) -> SceneFactory& = default;

    [[nodiscard]] static auto new_scene(const Camera& camera) -> SceneFactory;

    auto set_light(const PointLight& light) -> void;

    auto add_ground(transform::Transform transform) -> void;
    auto add_tree(transform::Transform transform) -> void;
    auto add_bush(transform::Transform transform) -> void;
    auto add_lit_ground(transform::Transform transform) -> void;
    auto add_lit_tree(transform::Transform transform) -> void;
    auto add_lit_bush(transform::Transform transform) -> void;
    auto add_sun(transform::Transform transform) -> void;
    auto add_earth(transform::Transform transform) -> void;
    auto add_moon(transform::Transform transform) -> void;
    auto add_sphere(
        transform::Transform transform,
        math::Vector<float> color,
        object::shader::LightingModel lighting,
        float shininess = 32.0f,
        float specular_strength = 1.0f
    ) -> void;
    auto add_triangle(transform::Transform transform) -> void;
    auto add_clock(transform::Transform transform) -> void;
    auto add_login(transform::Transform transform) -> void;
    [[nodiscard]] auto
    add_model(const std::filesystem::path& path, transform::Transform transform)
        -> std::expected<void, std::string>;

    [[nodiscard]] auto build() && -> Scene;
    [[nodiscard]] auto build(
        std::function<void(std::span<object::DrawableObject>, PointLight&)>
            update_handler
    ) && -> Scene;

  private:
    Camera camera;
    PointLight light{
        {0.0f, 0.0f, 0.0f},
        {1.0f, 1.0f, 1.0f},
        {0.01f, 0.01f, 0.01f},
        {1.0f, 0.0f, 0.0f}
    };
    Scene::ShaderPrograms shader_programs;
    std::vector<object::DrawableObject> objects;
    std::vector<std::function<void()>> object_updates;

    explicit SceneFactory(const Camera& camera);

    [[nodiscard]] auto get_shader(
        const std::filesystem::path& vertex_path,
        const std::filesystem::path& fragment_path
    ) -> object::shader::ShaderProgram&;

    auto add_colored_object(
        std::span<const float> data,
        object::material::Material::Coloring coloring,
        const std::filesystem::path& fragment_path,
        transform::Transform transform,
        float shininess,
        float specular_strength
    ) -> void;

    auto add_object(
        std::span<const float> data,
        std::span<const object::model::VertexAttribute> attributes,
        const std::filesystem::path& vertex_path,
        const std::filesystem::path& fragment_path,
        transform::Transform transform,
        float shininess = 32.0f,
        float specular_strength = 1.0f
    ) -> object::shader::ShaderProgram&;
};

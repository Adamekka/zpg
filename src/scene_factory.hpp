#pragma once

#include "scene.hpp"

class SceneFactory final {
  public:
    SceneFactory(const SceneFactory&) = delete;
    SceneFactory(SceneFactory&&) = default;

    ~SceneFactory() = default;

    auto operator=(const SceneFactory&) -> SceneFactory& = delete;
    auto operator=(SceneFactory&&) -> SceneFactory& = default;

    [[nodiscard]] static auto new_scene(const Camera& camera) -> SceneFactory;

    auto add_ground(transform::Transform transform) -> void;
    auto add_tree(transform::Transform transform) -> void;
    auto add_bush(transform::Transform transform) -> void;
    auto add_sun(transform::Transform transform) -> void;
    auto add_earth(transform::Transform transform) -> void;
    auto add_moon(transform::Transform transform) -> void;
    auto add_triangle(transform::Transform transform) -> void;
    auto add_clock(transform::Transform transform) -> void;

    [[nodiscard]] auto build() && -> Scene;
    [[nodiscard]] auto build(
        std::function<void(std::span<object::DrawableObject>)> update_handler
    ) && -> Scene;

  private:
    Camera camera;
    Scene::ShaderPrograms shader_programs;
    std::vector<object::DrawableObject> objects;
    std::vector<std::function<void()>> object_updates;

    explicit SceneFactory(const Camera& camera);

    auto add_object(
        std::span<const float> data,
        std::span<const object::model::VertexAttribute> attributes,
        const std::filesystem::path& vertex_path,
        const std::filesystem::path& fragment_path,
        transform::Transform transform
    ) -> object::shader::ShaderProgram&;
};

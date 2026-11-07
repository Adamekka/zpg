#pragma once

#include "camera.hpp"
#include "object/drawable_object.hpp"
#include "object/skybox.hpp"
#include "point_light.hpp"
#include <functional>
#include <map>
#include <memory>
#include <span>
#include <utility>
#include <variant>
#include <vector>

class Scene final {
  public:
    using ShaderPrograms = std::map<
        std::pair<std::filesystem::path, std::filesystem::path>,
        object::shader::ShaderProgram>;

    struct NoSkybox final {};
    using Background = std::variant<NoSkybox, object::Skybox>;

    Camera camera;
    PointLight light;

    Scene(
        Camera camera,
        const PointLight& light,
        ShaderPrograms&& shader_programs,
        std::vector<object::DrawableObject>&& objects,
        Background&& background,
        std::function<void(std::span<object::DrawableObject>, PointLight&)>
            update_handler
    );

    Scene(const Scene&) = delete;
    Scene(Scene&&) = default;

    ~Scene() = default;

    auto operator=(const Scene&) -> Scene& = delete;
    auto operator=(Scene&&) -> Scene& = default;

    auto update() -> void;
    auto draw() -> void;

  private:
    ShaderPrograms shader_programs;
    std::vector<object::DrawableObject> objects;
    Background background;
    std::unique_ptr<
        std::function<void(std::span<object::DrawableObject>, PointLight&)>>
        update_handler;
};

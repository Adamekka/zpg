#pragma once

#include "object/drawable_object.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Scene final {
  public:
    Scene(
        std::unordered_map<std::string, object::shader::ShaderProgram>&&
            shader_programs,
        std::vector<object::DrawableObject>&& objects,
        std::function<void()> update_handler
    );

    Scene(const Scene&) = delete;
    Scene(Scene&&) = default;

    ~Scene() = default;

    auto operator=(const Scene&) -> Scene& = delete;
    auto operator=(Scene&&) -> Scene& = default;

    auto update() -> void;
    auto draw() const -> void;

  private:
    std::unordered_map<std::string, object::shader::ShaderProgram>
        shader_programs;
    std::vector<object::DrawableObject> objects;
    // Keep the running callback at a stable address if it adds scenes.
    std::unique_ptr<std::function<void()>> update_handler;
};

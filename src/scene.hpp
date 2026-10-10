#pragma once

#include "object/drawable_object.hpp"
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

class Scene final {
  public:
    Scene(
        std::unordered_map<std::string, object::shader::ShaderProgram>&&
            shader_programs,
        std::vector<object::DrawableObject>&& objects,
        std::function<void(std::span<object::DrawableObject>)> update_handler
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
    std::unique_ptr<std::function<void(std::span<object::DrawableObject>)>>
        update_handler;
};

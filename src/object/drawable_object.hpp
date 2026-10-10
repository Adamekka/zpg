#pragma once

#include "../transform/transform.hpp"
#include "model/model.hpp"
#include "shader/shader_program.hpp"

namespace object {

class DrawableObject final {
  public:
    transform::Transform transform;

    DrawableObject(
        model::Model mesh,
        shader::ShaderProgram* shader_program,
        transform::Transform transform
    )
        : transform{std::move(transform)}
        , mesh{std::move(mesh)}
        , shader_program{shader_program} {}

    DrawableObject(const DrawableObject&) = delete;
    DrawableObject(DrawableObject&&) = default;

    ~DrawableObject() = default;

    auto operator=(const DrawableObject&) -> DrawableObject& = delete;
    auto operator=(DrawableObject&&) -> DrawableObject& = default;

    auto draw() const -> void;

  private:
    model::Model mesh;
    shader::ShaderProgram* shader_program;
};

} // namespace object

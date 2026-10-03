#pragma once

#include "mesh/model.hpp"
#include "shader/shader_program.hpp"

namespace object {

class DrawableObject final {
  public:
    DrawableObject(mesh::Model mesh, shader::ShaderProgram* shader_program)
        : mesh{std::move(mesh)}
        , shader_program{shader_program} {}

    DrawableObject(const DrawableObject&) = delete;
    DrawableObject(DrawableObject&&) = default;

    ~DrawableObject() = default;

    auto operator=(const DrawableObject&) -> DrawableObject& = delete;
    auto operator=(DrawableObject&&) -> DrawableObject& = default;

    auto draw() const -> void;

  private:
    mesh::Model mesh;
    shader::ShaderProgram* shader_program;
};

} // namespace object

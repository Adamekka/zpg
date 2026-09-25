#pragma once

#include "mesh/mesh.hpp"
#include "shader/shader_program.hpp"

namespace object {

class Object final {
  public:
    Object(mesh::Mesh mesh, shader::ShaderProgram* shader_program)
        : mesh{std::move(mesh)}
        , shader_program{shader_program} {}

    Object(const Object&) = delete;
    Object(Object&&) = default;

    ~Object() = default;

    auto operator=(const Object&) -> Object& = delete;
    auto operator=(Object&&) -> Object& = default;

    auto draw() const -> void;

  private:
    mesh::Mesh mesh;
    shader::ShaderProgram* shader_program;
};

} // namespace object

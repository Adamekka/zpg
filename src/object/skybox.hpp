#pragma once

#include "model/model.hpp"
#include "shader/shader_program.hpp"
#include <expected>

namespace object {

class Skybox final {
  public:
    Skybox(const Skybox&) = delete;
    Skybox(Skybox&& other) noexcept;

    ~Skybox();

    auto operator=(const Skybox&) -> Skybox& = delete;
    auto operator=(Skybox&& other) noexcept -> Skybox&;

    [[nodiscard]] static auto load(
        const std::filesystem::path& directory,
        shader::ShaderProgram& shader_program
    ) -> std::expected<Skybox, std::string>;

    auto draw() -> void;

  private:
    model::Model cube;
    shader::ShaderProgram* shader_program;
    uint32_t cubemap{0};

    explicit Skybox(shader::ShaderProgram& shader_program);
};

} // namespace object

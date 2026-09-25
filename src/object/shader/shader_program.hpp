#pragma once

#include "shader_type.hpp"
#include <filesystem>
#include <vector>

namespace object::shader {

class ShaderProgram final {
  public:
    ShaderProgram() = default;

    ShaderProgram(const ShaderProgram&);
    ShaderProgram(ShaderProgram&&) noexcept;

    ~ShaderProgram();

    auto operator=(const ShaderProgram&) -> ShaderProgram&;
    auto operator=(ShaderProgram&&) noexcept -> ShaderProgram&;

    auto compile(const std::filesystem::path& path, ShaderType type) -> void;

    auto link() -> void;

    auto bind() const -> void;

    static auto unbind() -> void;

  private:
    uint32_t id{glCreateProgram()};
    std::vector<uint32_t> shaders;
};

} // namespace object::shader

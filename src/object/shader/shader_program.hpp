#pragma once

#include "shader_arg.hpp"
#include "shader_type.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace object::shader {

class ShaderProgram final {
  public:
    ShaderProgram() = default;

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram(ShaderProgram&&) noexcept;

    ~ShaderProgram();

    auto operator=(const ShaderProgram&) -> ShaderProgram& = delete;
    auto operator=(ShaderProgram&&) noexcept -> ShaderProgram&;

    auto compile(const std::filesystem::path& path, ShaderType type) -> void;

    auto link() -> std::unordered_map<std::string, ShaderArg>&;

    auto bind() const -> void;

    static auto unbind() -> void;

    [[nodiscard]] auto get_args()
        -> std::unordered_map<std::string, ShaderArg>&;

  private:
    uint32_t id{glCreateProgram()};
    std::vector<uint32_t> shaders;

    // TODO: Check arg exists before access to prevent crashes
    std::unordered_map<std::string, ShaderArg> args;
};

} // namespace object::shader

#include "shader_program.hpp"
#include "../../core/panic.hpp"
#include <format>
#include <fstream>
#include <utility>

namespace object::shader {

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : id{std::exchange(other.id, 0)}
    , shaders{std::move(other.shaders)} {}

ShaderProgram::~ShaderProgram() {
    if (this->id != 0) {
        glDeleteProgram(this->id);
    }
}

auto ShaderProgram::operator=(ShaderProgram&& other) noexcept
    -> ShaderProgram& {
    if (this != &other) {
        glDeleteProgram(this->id);
        this->id = std::exchange(other.id, 0);
        this->shaders = std::move(other.shaders);
    }

    return *this;
}

auto ShaderProgram::compile(
    const std::filesystem::path& path, const ShaderType type
) -> void {
    auto shader_source_stream{std::ifstream{path}};
    const auto source{std::string{
        std::istreambuf_iterator<char>{shader_source_stream},
        std::istreambuf_iterator<char>{}
    }};

    const auto shader{glCreateShader(type.get())};
    const auto* const source_ptr{source.c_str()};
    glShaderSource(shader, 1, &source_ptr, nullptr);

    glCompileShader(shader);

    constexpr auto CHECK_SHADER_COMPILE_STATUS{
        [&](const uint32_t shader) -> void {
            auto success{int32_t{}};
            auto info_log{std::array<char, 512>{}};
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

            if (success == 0) {
                glGetShaderInfoLog(
                    shader, info_log.size(), nullptr, info_log.data()
                );
                core::panic(
                    std::format(
                        "Shader compilation failed: {}", info_log.data()
                    )
                );
            }
        }
    };

    CHECK_SHADER_COMPILE_STATUS(shader);

    this->shaders.emplace_back(shader);
    glAttachShader(this->id, shader);
}

auto ShaderProgram::link() -> void {
    glLinkProgram(this->id);

    constexpr auto CHECK_SHADER_LINK_STATUS{[&](const uint32_t shader) -> void {
        auto success{int32_t{}};
        auto info_log{std::array<char, 512>{}};
        glGetProgramiv(shader, GL_LINK_STATUS, &success);

        if (success == 0) {
            glGetProgramInfoLog(
                shader, info_log.size(), nullptr, info_log.data()
            );
            core::panic(
                std::format("Shader linking failed: {}", info_log.data())
            );
        }
    }};

    CHECK_SHADER_LINK_STATUS(this->id);

    glUseProgram(this->id);

    for (const auto shader : this->shaders) {
        glDeleteShader(shader);
    }
}

auto ShaderProgram::bind() const -> void {
    glUseProgram(this->id);
}

auto ShaderProgram::unbind() -> void {
    glUseProgram(0);
}

} // namespace object::shader

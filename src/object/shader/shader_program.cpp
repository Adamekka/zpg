#include "shader_program.hpp"
#include <fstream>

namespace object::shader {

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : id{std::exchange(other.id, 0)}
    , shaders{std::move(other.shaders)}
    , args{std::move(other.args)} {}

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
        this->args = std::move(other.args);
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

auto ShaderProgram::link() -> std::unordered_map<std::string, ShaderArg>& {
    glLinkProgram(this->id);

    constexpr auto CHECK_SHADER_LINK_STATUS{[&](const uint32_t shader) -> void {
        auto success{int32_t{}};
        auto info_log{std::array<char, 512>{}};
        glGetProgramiv(shader, GL_LINK_STATUS, &success);

        if (success == 0) {
            glGetProgramInfoLog(
                shader, info_log.size(), nullptr, info_log.data()
            );
            std::println(stderr, "{}", info_log.data());
        }
    }};

    CHECK_SHADER_LINK_STATUS(this->id);

    glUseProgram(this->id);

    for (const auto shader : this->shaders) {
        glDeleteShader(shader);
    }

    // Uniforms

    auto uniform_count{int32_t{}};
    auto max_name_length{int32_t{}};

    glGetProgramiv(this->id, GL_ACTIVE_UNIFORMS, &uniform_count);
    glGetProgramiv(this->id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_length);

    this->args.clear();
    this->args.reserve(uniform_count);

    auto name{std::vector<char>(max_name_length)};

    for (auto i{0}; i < uniform_count; ++i) {
        auto name_len{int32_t{}};
        auto array_size{int32_t{}};
        auto type{uint32_t{}};

        glGetActiveUniform(
            this->id,
            i,
            max_name_length,
            &name_len,
            &array_size,
            &type,
            name.data()
        );

        auto block_index{int32_t{}};

        glGetActiveUniformsiv(
            this->id,
            1,
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            reinterpret_cast<const uint32_t* const>(&i),
            GL_UNIFORM_BLOCK_INDEX,
            &block_index
        );

        // Members of named uniform blocks are supplied through UBOs,
        // not through glUniform*.
        if (block_index != -1) {
            continue;
        }

        const auto location{glGetUniformLocation(this->id, name.data())};

        if (location == -1) {
            // Unexpected for an active default-block uniform.
            core::panic("Uniform is active but has no location");
        }

        this->args.emplace(
            name.data(), ShaderArg{ShaderVarType{type}, location, array_size}
        );
    }

    return this->args;
}

auto ShaderProgram::bind() const -> void {
    glUseProgram(this->id);
}

auto ShaderProgram::unbind() -> void {
    glUseProgram(0);
}

auto ShaderProgram::get_args() -> std::unordered_map<std::string, ShaderArg>& {
    return this->args;
}

} // namespace object::shader

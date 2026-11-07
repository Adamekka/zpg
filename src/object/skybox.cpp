#include "skybox.hpp"
#include "../core/defer.hpp"
#include <stb/stb_image.h>
#include <utility>

namespace object {

namespace {

// Inward-facing triangles keep the cube visible from the camera inside it.
// clang-format off
constexpr auto CUBE{std::array{
    -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
}};
// clang-format on

constexpr auto POSITION{std::array{model::VertexAttribute{3}}};

} // namespace

Skybox::Skybox(shader::ShaderProgram& shader_program)
    : cube{model::Model::from_raw_data(
          CUBE, POSITION, model::MeshDrawMode::Value::Triangles
      )}
    , shader_program{&shader_program} {
    glGenTextures(1, &this->cubemap);
}

auto Skybox::load(
    const std::filesystem::path& directory,
    shader::ShaderProgram& shader_program
) -> std::expected<Skybox, std::string> {
    auto skybox{Skybox{shader_program}};
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.cubemap);

    // OpenGL orders cubemap faces as +X, -X, +Y, -Y, +Z, -Z.
    constexpr auto FACES{std::array{
        "posx.jpg", "negx.jpg", "posy.jpg", "negy.jpg", "posz.jpg", "negz.jpg"
    }};
    auto max_size{int32_t{}};
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &max_size);
    auto face_size{int32_t{}};
    for (auto index{size_t{0}}; index < FACES.size(); ++index) {
        const auto path{directory / FACES[index]};
        auto width{int32_t{}};
        auto height{int32_t{}};
        auto* const pixels{
            stbi_load(path.c_str(), &width, &height, nullptr, STBI_rgb_alpha)
        };
        defer({ stbi_image_free(pixels); });
        if (pixels == nullptr) {
            auto message{
                "Failed to decode skybox face '" + path.string() + "'"
            };
            if (const auto* const reason{stbi_failure_reason()};
                reason != nullptr) {
                message.append(": ").append(reason);
            }
            return std::unexpected{std::move(message)};
        }
        if (width <= 0 || width != height || width > max_size) {
            return std::unexpected{
                "Skybox face '" + path.string()
                + "' must be square and no larger than "
                + std::to_string(max_size) + " pixels per side"
            };
        }
        if (index > 0 && width != face_size) {
            return std::unexpected{
                "Skybox face '" + path.string()
                + "' must have the same dimensions as the other faces"
            };
        }
        face_size = width;
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<uint32_t>(index),
            0,
            GL_RGBA8,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixels
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    for (const auto axis :
         std::array{GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T, GL_TEXTURE_WRAP_R}) {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, axis, GL_CLAMP_TO_EDGE);
    }
    return skybox;
}

Skybox::Skybox(Skybox&& other) noexcept
    : cube{std::move(other.cube)}
    , shader_program{other.shader_program}
    , cubemap{std::exchange(other.cubemap, 0)} {}

Skybox::~Skybox() {
    if (this->cubemap != 0) {
        glDeleteTextures(1, &this->cubemap);
    }
}

auto Skybox::operator=(Skybox&& other) noexcept -> Skybox& {
    if (this != &other) {
        glDeleteTextures(1, &this->cubemap);
        this->cube = std::move(other.cube);
        this->shader_program = other.shader_program;
        this->cubemap = std::exchange(other.cubemap, 0);
    }
    return *this;
}

auto Skybox::draw() -> void {
    this->shader_program->bind();
    const auto& args{this->shader_program->get_args()};
    args.at("skybox_texture").set_texture_unit(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubemap);

    auto depth_function{int32_t{}};
    auto depth_write{GLboolean{}};
    glGetIntegerv(GL_DEPTH_FUNC, &depth_function);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_write);
    const auto seamless{glIsEnabled(GL_TEXTURE_CUBE_MAP_SEAMLESS)};
    defer({
        glDepthFunc(depth_function);
        glDepthMask(depth_write);
        if (seamless == GL_FALSE) {
            glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
    });
    // Far-plane fragments fill only the background and never replace depth.
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    this->cube.draw();
}

} // namespace object

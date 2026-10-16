#include "scene_factory.hpp"
#include "../models/bushes.hpp"
#include "../models/clock.hpp"
#include "../models/earth.hpp"
#include "../models/moon.hpp"
#include "../models/plain.hpp"
#include "../models/sun.hpp"
#include "../models/tree.hpp"
#include <cerrno>
#include <chrono>
#include <ctime>

namespace {

constexpr auto POSITION_COLOR{std::array{
    object::model::VertexAttribute{3}, // Position
    object::model::VertexAttribute{3}  // Color
}};
constexpr auto POSITION_NORMAL{std::array{
    object::model::VertexAttribute{3}, // Position
    object::model::VertexAttribute{3}  // Normal
}};
constexpr auto POSITION_COLOR_NORMAL{std::array{
    object::model::VertexAttribute{3}, // Position
    object::model::VertexAttribute{3}, // Color
    object::model::VertexAttribute{3}  // Normal
}};

} // namespace

SceneFactory::SceneFactory(const Camera& camera)
    : camera{camera} {}

auto SceneFactory::new_scene(const Camera& camera) -> SceneFactory {
    return SceneFactory{camera};
}

auto SceneFactory::add_ground(transform::Transform transform) -> void {
    this->add_object(
        PLAIN,
        POSITION_NORMAL,
        "shaders/basic.vert",
        "shaders/ground.frag",
        std::move(transform)
    );
}

auto SceneFactory::add_tree(transform::Transform transform) -> void {
    this->add_object(
        TREE,
        POSITION_NORMAL,
        "shaders/tree.vert",
        "shaders/basic.frag",
        std::move(transform)
    );
}

auto SceneFactory::add_bush(transform::Transform transform) -> void {
    // The green shader only reads position, so the model's normals are unused.
    this->add_object(
        BUSHES,
        POSITION_NORMAL,
        "shaders/basic.vert",
        "shaders/green.frag",
        std::move(transform)
    );
}

auto SceneFactory::add_sun(transform::Transform transform) -> void {
    this->add_object(
        SUN,
        POSITION_COLOR_NORMAL,
        "shaders/basic.vert",
        "shaders/basic.frag",
        std::move(transform)
    );
}

auto SceneFactory::add_earth(transform::Transform transform) -> void {
    this->add_object(
        EARTH,
        POSITION_COLOR_NORMAL,
        "shaders/basic.vert",
        "shaders/basic.frag",
        std::move(transform)
    );
}

auto SceneFactory::add_moon(transform::Transform transform) -> void {
    this->add_object(
        MOON,
        POSITION_COLOR_NORMAL,
        "shaders/basic.vert",
        "shaders/basic.frag",
        std::move(transform)
    );
}

auto SceneFactory::add_triangle(transform::Transform transform) -> void {
    // clang-format off
    constexpr auto TRIANGLE{std::array{
        -0.2f, -0.15f, 0.0f, 1.0f, 0.0f, 0.0f,
         0.2f, -0.15f, 0.0f, 0.0f, 1.0f, 0.0f,
         0.0f,  0.30f, 0.0f, 0.0f, 0.0f, 1.0f
    }};
    // clang-format on
    this->add_object(
        TRIANGLE,
        POSITION_COLOR,
        "shaders/basic.vert",
        "shaders/basic.frag",
        std::move(transform)
    );
}

auto SceneFactory::add_clock(transform::Transform transform) -> void {
    auto* const clock_shader{&this->add_object(
        CLOCK,
        POSITION_COLOR,
        "shaders/clock.vert",
        "shaders/basic.frag",
        std::move(transform)
    )};
    this->object_updates.emplace_back([clock_shader]() -> void {
        const auto now{std::chrono::system_clock::now()};
        const auto whole_seconds{std::chrono::floor<std::chrono::seconds>(now)};
        const auto timestamp{
            std::chrono::system_clock::to_time_t(whole_seconds)
        };
        auto local_time{std::tm{}};
        if (localtime_r(&timestamp, &local_time) == nullptr) {
            [[maybe_unused]] const auto error{errno};
            core::panic("Cannot read the local time.");
        }
        const auto seconds{
            static_cast<float>(local_time.tm_sec)
            + std::chrono::duration<float>(now - whole_seconds).count()
        };
        const auto minutes{
            static_cast<float>(local_time.tm_min) + (seconds / 60.0f)
        };
        const auto hours{
            static_cast<float>(local_time.tm_hour % 12) + (minutes / 60.0f)
        };
        const auto turns{
            std::array{hours / 12.0f, minutes / 60.0f, seconds / 60.0f}
        };
        constexpr auto HAND_UNIFORMS{
            std::array{"hour_matrix", "minute_matrix", "second_matrix"}
        };
        clock_shader->bind();
        for (auto hand{size_t{0}}; hand < turns.size(); ++hand) {
            // Negative Z rotation moves clockwise from twelve.
            clock_shader->get_args()
                .at(HAND_UNIFORMS[hand])
                .set_uniform(
                    transform::Transform{
                        {0.0f, 0.0f, 0.0f},
                        {0.0f,
                         0.0f,
                         -2.0f * std::numbers::pi_v<float> * turns[hand]},
                        {1.0f, 1.0f, 1.0f}
                    }
                        .get_matrix()
                );
        }
    });
}

auto SceneFactory::add_object(
    const std::span<const float> data,
    const std::span<const object::model::VertexAttribute> attributes,
    const std::filesystem::path& vertex_path,
    const std::filesystem::path& fragment_path,
    transform::Transform transform
) -> object::shader::ShaderProgram& {
    core::assert_ne(glfwGetCurrentContext(), nullptr);
    core::assert_ne(glCreateProgram, nullptr);
    const auto [shader, inserted]{
        this->shader_programs.try_emplace({vertex_path, fragment_path})
    };
    if (inserted) {
        shader->second.compile(
            vertex_path, object::shader::ShaderType::Value::Vertex
        );
        shader->second.compile(
            fragment_path, object::shader::ShaderType::Value::Fragment
        );
        shader->second.link();
    }
    this->objects.emplace_back(
        object::model::Model::from_raw_data(
            data, attributes, object::model::MeshDrawMode::Value::Triangles
        ),
        &shader->second,
        std::move(transform)
    );
    return shader->second;
}

auto SceneFactory::build() && -> Scene {
    // Built-in object animations still run without a scene-specific handler.
    return std::move(*this).build(
        [](
            [[maybe_unused]] const std::span<object::DrawableObject> objects
        ) -> void {}
    );
}

auto SceneFactory::build(
    std::function<void(std::span<object::DrawableObject>)> update_handler
) && -> Scene {
    core::assert_ne(update_handler, nullptr);
    // Moving the map preserves the shader addresses held by the objects.
    return Scene{
        this->camera,
        std::move(this->shader_programs),
        std::move(this->objects),
        [object_updates = std::move(this->object_updates),
         update_handler = std::move(update_handler)](
            const std::span<object::DrawableObject> objects
        ) -> void {
            for (const auto& update : object_updates) {
                update();
            }
            update_handler(objects);
        }
    };
}

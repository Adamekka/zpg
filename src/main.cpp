#include "../models/CVI0014.hpp"
#include "../models/bushes.hpp"
#include "../models/sphere.hpp"
#include "../models/tree.hpp"
#include "app.hpp"
#include "core/assert.hpp"
#include <array>
#include <filesystem>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

auto main() -> int {
    App::instance().init_opengl();
    core::assert_ne(glfwGetCurrentContext(), nullptr);
    core::assert_ne(glCreateProgram, nullptr);

    const auto make_shader_program{
        [](
            const std::filesystem::path& fragment_path
        ) -> object::shader::ShaderProgram {
            auto shader_program{object::shader::ShaderProgram{}};
            shader_program.compile(
                "shaders/basic.vert", object::shader::ShaderType::Value::Vertex
            );
            shader_program.compile(
                fragment_path, object::shader::ShaderType::Value::Fragment
            );
            shader_program.link();
            return shader_program;
        }
    };

    // Triangle
    {
        auto shader_programs{
            std::unordered_map<std::string, object::shader::ShaderProgram>{}
        };
        auto* const triangle_shader{
            &shader_programs
                 .emplace("triangle", make_shader_program("shaders/basic.frag"))
                 .first->second
        };
        auto objects{std::vector<object::DrawableObject>{}};
        objects.emplace_back(
            object::mesh::Model{
                std::array{
                    object::mesh::Vertex{
                        {-0.6f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}
                    },
                    object::mesh::Vertex{
                        {0.6f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}
                    },
                    object::mesh::Vertex{{0.0f, 0.6f, 0.0f}, {0.0f, 0.0f, 1.0f}}
                },
                object::mesh::MeshDrawMode::Value::Triangles
            },
            triangle_shader
        );
        App::instance().switch_scene(
            App::instance().add_scene(
                Scene{
                    std::move(shader_programs),
                    std::move(objects),
                    [triangle_shader]() -> void {
                        triangle_shader->bind();
                        triangle_shader->get_args().at("offset").set_uniform(
                            math::Vector{0.0f, 0.0f, 0.0f}
                        );
                        triangle_shader->get_args().at("scale").set_uniform(
                            math::Vector{1.0f, 1.0f, 1.0f}
                        );
                        triangle_shader->get_args().at("angle").set_uniform(
                            static_cast<float>(glfwGetTime())
                        );
                    }
                }
            )
        );
    }

    // Sphere
    {
        auto shader_programs{
            std::unordered_map<std::string, object::shader::ShaderProgram>{}
        };
        auto* const sphere_shader{
            &shader_programs
                 .emplace("sphere", make_shader_program("shaders/basic.frag"))
                 .first->second
        };
        auto objects{std::vector<object::DrawableObject>{}};
        objects.emplace_back(
            object::mesh::Model::from_raw_data(
                SPHERE, object::mesh::MeshDrawMode::Value::Triangles
            ),
            sphere_shader
        );
        static_cast<void>(App::instance().add_scene(
            Scene{
                std::move(shader_programs),
                std::move(objects),
                [sphere_shader]() -> void {
                    sphere_shader->bind();
                    sphere_shader->get_args().at("offset").set_uniform(
                        math::Vector{0.0f, 0.0f, 0.0f}
                    );
                    sphere_shader->get_args().at("scale").set_uniform(
                        math::Vector{0.7f, 0.7f, 0.7f}
                    );
                    sphere_shader->get_args().at("angle").set_uniform(
                        static_cast<float>(glfwGetTime())
                    );
                }
            }
        ));
    }

    // Forest
    {
        struct ForestObject final {
            std::span<const float> model_data;
            math::Vector<float> offset;
            math::Vector<float> scale;

            ForestObject(
                std::span<const float> model_data,
                const math::Vector<float> offset,
                const math::Vector<float> scale
            )
                : model_data{model_data}
                , offset{offset}
                , scale{scale} {}
        };

        // The source tree is over seven units tall; fit it inside clip space.
        constexpr auto TREE_SCALE{math::Vector{0.18f, 0.18f, 0.18f}};
        constexpr auto BUSH_SCALE{math::Vector{0.5f, 0.5f, 0.5f}};
        const auto placements{std::array{
            ForestObject{
                TREE, math::Vector{-0.55f, -0.65f, -0.35f}, TREE_SCALE
            },
            ForestObject{TREE, math::Vector{0.0f, -0.65f, -0.35f}, TREE_SCALE},
            ForestObject{TREE, math::Vector{0.55f, -0.65f, -0.35f}, TREE_SCALE},
            ForestObject{TREE, math::Vector{-0.3f, -0.85f, 0.25f}, TREE_SCALE},
            ForestObject{TREE, math::Vector{0.3f, -0.85f, 0.25f}, TREE_SCALE},
            ForestObject{
                BUSHES, math::Vector{-0.6f, -0.85f, 0.65f}, BUSH_SCALE
            },
            ForestObject{BUSHES, math::Vector{0.0f, -0.85f, 0.65f}, BUSH_SCALE},
            ForestObject{BUSHES, math::Vector{0.6f, -0.85f, 0.65f}, BUSH_SCALE}
        }};
        auto shader_programs{
            std::unordered_map<std::string, object::shader::ShaderProgram>{}
        };
        auto objects{std::vector<object::DrawableObject>{}};
        auto shaders{std::vector<object::shader::ShaderProgram*>{}};

        // Each object needs its own program to retain its uniform transform.
        for (auto index{size_t{0}}; index < placements.size(); ++index) {
            // Map node addresses stay valid when the scene moves.
            auto* const shader{
                &shader_programs
                     .emplace(
                         std::to_string(index),
                         make_shader_program("shaders/green.frag")
                     )
                     .first->second
            };
            shaders.emplace_back(shader);
            objects.emplace_back(
                object::mesh::Model::from_raw_data(
                    placements.at(index).model_data,
                    object::mesh::MeshDrawMode::Value::Triangles
                ),
                shader
            );
        }

        static_cast<void>(App::instance().add_scene(
            Scene{
                std::move(shader_programs),
                std::move(objects),
                [shaders = std::move(shaders), placements]() -> void {
                    for (auto index{size_t{0}}; index < shaders.size();
                         ++index) {
                        shaders[index]->bind();
                        shaders[index]->get_args().at("offset").set_uniform(
                            placements.at(index).offset
                        );
                        shaders[index]->get_args().at("scale").set_uniform(
                            placements.at(index).scale
                        );
                        shaders[index]->get_args().at("angle").set_uniform(
                            0.0f
                        );
                    }
                }
            }
        ));
    }

    // CVI0014
    {
        auto shader_programs{
            std::unordered_map<std::string, object::shader::ShaderProgram>{}
        };
        auto* const shader{
            &shader_programs
                 .emplace("cvi0014", make_shader_program("shaders/basic.frag"))
                 .first->second
        };

        static_assert(
            CVI0014.size() % (object::mesh::Position::DIMENSION * 3) == 0,
            "CVI0014 must contain complete triangles of XYZ positions"
        );
        auto vertices{std::vector<object::mesh::Vertex>{}};
        vertices.reserve(CVI0014.size() / object::mesh::Position::DIMENSION);
        // XYZ-only data needs an explicit color for the vertex layout.
        for (auto index{size_t{0}}; index < CVI0014.size();
             index += object::mesh::Position::DIMENSION) {
            vertices.emplace_back(
                object::mesh::Position{
                    CVI0014.at(index),
                    CVI0014.at(index + 1),
                    CVI0014.at(index + 2)
                },
                object::mesh::Color{1.0f, 1.0f, 1.0f}
            );
        }
        auto objects{std::vector<object::DrawableObject>{}};
        objects.emplace_back(
            object::mesh::Model{
                std::move(vertices),
                object::mesh::MeshDrawMode::Value::Triangles
            },
            shader
        );
        static_cast<void>(App::instance().add_scene(
            Scene{
                std::move(shader_programs),
                std::move(objects),
                [shader]() -> void {
                    shader->bind();
                    shader->get_args().at("offset").set_uniform(
                        math::Vector{0.0f, 0.0f, 0.0f}
                    );
                    // Half scale fits the rotating model inside clip space.
                    shader->get_args().at("scale").set_uniform(
                        math::Vector{0.5f, 0.5f, 0.5f}
                    );
                    shader->get_args().at("angle").set_uniform(
                        static_cast<float>(glfwGetTime())
                    );
                }
            }
        ));
    }

    App::instance().run();
}

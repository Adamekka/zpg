#include "app.hpp"
#include "../models/bushes.hpp"
#include "../models/earth.hpp"
#include "../models/moon.hpp"
#include "../models/sun.hpp"
#include "../models/tree.hpp"
#include "input/input_manager.hpp"
#include <random>

auto App::instance() -> App& {
    static auto instance{App{}};
    return instance;
}

auto App::init_opengl() -> void {
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    this->window = glfwCreateWindow(800, 600, "ZPG", nullptr, nullptr);

    if (window == nullptr) {
        glfwTerminate();
        core::panic("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (gladLoadGL(static_cast<GLADloadfunc>(glfwGetProcAddress)) == 0) {
        core::panic("GLAD initialization failed");
    }

    glEnable(GL_DEPTH_TEST);
}

auto App::create_objects() -> void {
    core::assert_ne(glfwGetCurrentContext(), nullptr);
    core::assert_ne(glCreateProgram, nullptr);

    const auto make_shader_program{
        [](
            const std::filesystem::path& vertex_path,
            const std::filesystem::path& fragment_path
        ) -> object::shader::ShaderProgram {
            auto shader_program{object::shader::ShaderProgram{}};
            shader_program.compile(
                vertex_path, object::shader::ShaderType::Value::Vertex
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
        auto* const triangle_shader{&shader_programs
                                         .emplace(
                                             "triangle",
                                             make_shader_program(
                                                 "shaders/basic.vert",
                                                 "shaders/basic.frag"
                                             )
                                         )
                                         .first->second};
        // clang-format off
        constexpr auto TRIANGLE{std::array{
            -0.2f, -0.15f, 0.0f, 1.0f, 0.0f, 0.0f,
             0.2f, -0.15f, 0.0f, 0.0f, 1.0f, 0.0f,
             0.0f,  0.30f, 0.0f, 0.0f, 0.0f, 1.0f
        }};
        // clang-format on
        constexpr auto VERTEX_ATTRIBUTES{std::array{
            object::model::VertexAttribute{3}, // Position
            object::model::VertexAttribute{3}  // Color
        }};
        const auto translation{transform::Translation{{0.55f, 0.0f, 0.0f}}};
        const auto rotation{
            transform::Rotation{{0.0f, 0.0f, std::numbers::pi_v<float> / 2.0f}}
        };
        auto objects{std::vector<object::DrawableObject>{}};
        for (const auto& transformation : std::array{
                 transform::Transform{
                     std::make_shared<transform::CompositeTransform>(
                         std::vector<transform::TransformNode>{
                             translation, rotation
                         }
                     )
                 },
                 transform::Transform{
                     std::make_shared<transform::CompositeTransform>(
                         std::vector<transform::TransformNode>{
                             rotation, translation
                         }
                     )
                 }
             }) {
            objects.emplace_back(
                object::model::Model::from_raw_data(
                    TRIANGLE,
                    VERTEX_ATTRIBUTES,
                    object::model::MeshDrawMode::Value::Triangles
                ),
                triangle_shader,
                transformation
            );
        }
        this->switch_scene(this->add_scene(
            Scene{
                std::move(shader_programs),
                std::move(objects),
                []([[maybe_unused]] const std::span<object::DrawableObject>
                       objects) -> void {}
            }
        ));
    }

    // Forest
    {
        using transform::Transform;

        auto shader_programs{
            std::unordered_map<std::string, object::shader::ShaderProgram>{}
        };
        auto* const bush_shader{&shader_programs
                                     .emplace(
                                         "bush",
                                         make_shader_program(
                                             "shaders/basic.vert",
                                             "shaders/green.frag"
                                         )
                                     )
                                     .first->second};
        auto* const basic_shader{&shader_programs
                                      .emplace(
                                          "basic",
                                          make_shader_program(
                                              "shaders/basic.vert",
                                              "shaders/basic.frag"
                                          )
                                      )
                                      .first->second};
        constexpr auto VERTEX_ATTRIBUTES{std::array{
            object::model::VertexAttribute{3}, // Position
            object::model::VertexAttribute{3}  // Normal
        }};
        constexpr auto SUN_ATTRIBUTES{std::array{
            object::model::VertexAttribute{3}, // Position
            object::model::VertexAttribute{3}, // Color
            object::model::VertexAttribute{3}  // Normal
        }};
        constexpr auto TREE_COLUMNS{size_t{5}};
        constexpr auto TREE_ROWS{size_t{4}};
        constexpr auto BUSH_COUNT{size_t{30}};
        constexpr auto ORIGIN{math::Vector{0.0f, 0.0f, 0.0f}};
        constexpr auto UNIT_SCALE{math::Vector{1.0f, 1.0f, 1.0f}};

        // Reverse Z after tilting, so the foreground passes the depth test.
        const auto forest_transform{
            Transform{{0.0f, -0.42f, 0.0f}, ORIGIN, {1.0f, 1.0f, -0.6f}}
            * Transform{ORIGIN, {0.65f, -0.25f, 0.0f}, UNIT_SCALE}
        };
        auto random{std::mt19937{std::random_device{}()}};
        auto jitter{std::uniform_real_distribution<float>{-0.035f, 0.035f}};
        auto angle{std::uniform_real_distribution<float>{
            0.0f, 2.0f * std::numbers::pi_v<float>
        }};
        auto tree_scale{std::uniform_real_distribution<float>{0.045f, 0.065f}};
        auto bush_scale{std::uniform_real_distribution<float>{0.16f, 0.28f}};
        auto bush_x{std::uniform_real_distribution<float>{-0.68f, 0.68f}};
        auto bush_z{std::uniform_real_distribution<float>{-0.54f, 0.54f}};
        auto objects{std::vector<object::DrawableObject>{}};
        objects.reserve((TREE_COLUMNS * TREE_ROWS) + BUSH_COUNT + 1);
        // Jitter grid positions to keep random trees from sharing a trunk
        // position.
        for (auto row{size_t{0}}; row < TREE_ROWS; ++row) {
            for (auto column{size_t{0}}; column < TREE_COLUMNS; ++column) {
                const auto size{tree_scale(random)};
                objects.emplace_back(
                    object::model::Model::from_raw_data(
                        TREE,
                        VERTEX_ATTRIBUTES,
                        object::model::MeshDrawMode::Value::Triangles
                    ),
                    // Use the model's normals as colors with basic.frag.
                    basic_shader,
                    forest_transform
                        * Transform{
                            {-0.56f + (static_cast<float>(column) * 0.28f)
                                 + jitter(random),
                             0.0f,
                             -0.45f + (static_cast<float>(row) * 0.30f)
                                 + jitter(random)},
                            {0.0f, angle(random), 0.0f},
                            {size, size, size}
                        }
                );
            }
        }
        for (auto index{size_t{0}}; index < BUSH_COUNT; ++index) {
            const auto size{bush_scale(random)};
            objects.emplace_back(
                object::model::Model::from_raw_data(
                    BUSHES,
                    VERTEX_ATTRIBUTES,
                    object::model::MeshDrawMode::Value::Triangles
                ),
                bush_shader,
                // The bush model extends below zero; align its base with trees.
                forest_transform
                    * Transform{
                        {bush_x(random), 0.009f * size, bush_z(random)},
                        {0.0f, angle(random), 0.0f},
                        {size, size, size}
                    }
            );
        }
        objects.emplace_back(
            object::model::Model::from_raw_data(
                SUN,
                SUN_ATTRIBUTES,
                object::model::MeshDrawMode::Value::Triangles
            ),
            basic_shader,
            Transform{{-0.65f, 0.70f, -0.5f}, ORIGIN, {0.12f, 0.12f, 0.12f}}
        );
        static_cast<void>(this->add_scene(
            Scene{
                std::move(shader_programs),
                std::move(objects),
                []([[maybe_unused]] const std::span<object::DrawableObject>
                       objects) -> void {}
            }
        ));
    }

    // Solar system
    {
        auto shader_programs{
            std::unordered_map<std::string, object::shader::ShaderProgram>{}
        };
        auto* const sun_shader{&shader_programs
                                    .emplace(
                                        "sun",
                                        make_shader_program(
                                            "shaders/basic.vert",
                                            "shaders/basic.frag"
                                        )
                                    )
                                    .first->second};
        auto* const earth_shader{&shader_programs
                                      .emplace(
                                          "earth",
                                          make_shader_program(
                                              "shaders/basic.vert",
                                              "shaders/basic.frag"
                                          )
                                      )
                                      .first->second};
        auto* const moon_shader{&shader_programs
                                     .emplace(
                                         "moon",
                                         make_shader_program(
                                             "shaders/basic.vert",
                                             "shaders/basic.frag"
                                         )
                                     )
                                     .first->second};
        constexpr auto VERTEX_ATTRIBUTES{std::array{
            object::model::VertexAttribute{3}, // Position
            object::model::VertexAttribute{3}, // Color
            object::model::VertexAttribute{3}  // Normal
        }};
        const auto earth_offset{transform::Transform{
            {0.55f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}
        }};
        const auto earth_scale{transform::Transform{
            {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.08f, 0.08f, 0.08f}
        }};
        const auto moon_local{transform::Transform{
            {0.18f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.035f, 0.035f, 0.035f}
        }};
        auto objects{std::vector<object::DrawableObject>{}};
        objects.emplace_back(
            object::model::Model::from_raw_data(
                SUN,
                VERTEX_ATTRIBUTES,
                object::model::MeshDrawMode::Value::Triangles
            ),
            sun_shader,
            transform::Transform{
                {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.18f, 0.18f, 0.18f}
            }
        );
        objects.emplace_back(
            object::model::Model::from_raw_data(
                EARTH,
                VERTEX_ATTRIBUTES,
                object::model::MeshDrawMode::Value::Triangles
            ),
            earth_shader,
            earth_offset * earth_scale
        );
        objects.emplace_back(
            object::model::Model::from_raw_data(
                MOON,
                VERTEX_ATTRIBUTES,
                object::model::MeshDrawMode::Value::Triangles
            ),
            moon_shader,
            earth_offset * moon_local
        );
        static_cast<void>(this->add_scene(
            Scene{
                std::move(shader_programs),
                std::move(objects),
                [earth_offset, earth_scale, moon_local](
                    const std::span<object::DrawableObject> objects
                ) -> void {
                    core::assert_eq(objects.size(), size_t{3});
                    const auto time{static_cast<float>(glfwGetTime())};
                    const auto earth_angle{time * 0.35f};
                    const auto moon_angle{time * 1.5f};
                    constexpr auto ORIGIN{math::Vector{0.0f, 0.0f, 0.0f}};
                    constexpr auto UNIT_SCALE{math::Vector{1.0f, 1.0f, 1.0f}};

                    const auto earth_orbit{
                        transform::Transform{
                            ORIGIN, {0.0f, 0.0f, earth_angle}, UNIT_SCALE
                        }
                        * earth_offset
                    };
                    objects[1].transform
                        = earth_orbit
                        * transform::
                              Transform{ORIGIN, {0.0f, time, 0.0f}, UNIT_SCALE}
                        * earth_scale;

                    objects[2].transform
                        = earth_orbit
                        * transform::
                              Transform{ORIGIN, {0.0f, 0.0f, moon_angle}, UNIT_SCALE}
                        * moon_local;
                }
            }
        ));
    }
}

auto App::add_scene(Scene&& scene) -> size_t {
    this->scenes.emplace_back(std::move(scene));
    return this->scenes.size() - 1;
}

auto App::switch_scene(const size_t index) -> void {
    core::assert_that(index < this->scenes.size());
    this->active_scene_index = index;
}

auto App::run() -> void {
    core::assert_that(this->active_scene_index.has_value());

    while (glfwWindowShouldClose(window) == 0) {
        if (input::InputManager::is_key_down(GLFW_KEY_1)) {
            this->switch_scene(0);
        } else if (
            this->scenes.size() > 1
            && input::InputManager::is_key_down(GLFW_KEY_2)
        ) {
            this->switch_scene(1);
        } else if (
            this->scenes.size() > 2
            && input::InputManager::is_key_down(GLFW_KEY_3)
        ) {
            this->switch_scene(2);
        } else if (
            this->scenes.size() > 3
            && input::InputManager::is_key_down(GLFW_KEY_4)
        ) {
            this->switch_scene(3);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto scene_index{*this->active_scene_index};
        // An update can grow the scene vector, so reacquire the scene to draw
        // it.
        this->scenes.at(scene_index).update();
        this->scenes.at(scene_index).draw();

        glfwSwapBuffers(this->window);
        glfwPollEvents();
    }
}

App::~App() {
    // Release scene resources while their OpenGL context is still alive.
    this->scenes.clear();

    if (this->window != nullptr) {
        glfwDestroyWindow(this->window);
    }
    glfwTerminate();
}

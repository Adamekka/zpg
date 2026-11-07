#include "app.hpp"
#include "input/input_manager.hpp"
#include "scene_factory.hpp"
#include <array>
#include <cstdio>
#include <cstring>

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
    glfwWindowHint(GLFW_SAMPLES, 4);

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
    glEnable(GL_MULTISAMPLE);
}

auto App::create_objects() -> std::expected<void, std::string> {
    core::assert_ne(glfwGetCurrentContext(), nullptr);
    core::assert_ne(glCreateProgram, nullptr);

    auto framebuffer_width{0};
    auto framebuffer_height{0};
    glfwGetFramebufferSize(
        this->window, &framebuffer_width, &framebuffer_height
    );
    core::assert_that(framebuffer_width > 0 && framebuffer_height > 0);
    const auto camera{Camera{
        {0.0f, 0.0f, 2.5f},
        -std::numbers::pi_v<float> / 2.0f,
        0.0f,
        math::Matrix<float, 4, 4>::perspective(
            std::numbers::pi_v<float> / 4.0f,
            static_cast<float>(framebuffer_width)
                / static_cast<float>(framebuffer_height),
            0.1f,
            100.0f
        )
    }};

    // Triangle
    {
        auto scene{SceneFactory::new_scene(camera)};
        const auto translation{transform::Translation{{0.55f, 0.0f, 0.0f}}};
        const auto rotation{
            transform::Rotation{{0.0f, 0.0f, std::numbers::pi_v<float> / 2.0f}}
        };
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
            scene.add_triangle(transformation);
        }
        this->switch_scene(this->add_scene(std::move(scene).build()));
    }

    // Forest
    {
        using transform::Transform;

        const auto forest_camera{Camera{
            {0.0f, 2.0f, 27.0f},
            -std::numbers::pi_v<float> / 2.0f,
            -0.08f,
            camera.get_projection_matrix()
        }};
        auto scene{SceneFactory::new_scene(forest_camera)};
        constexpr auto TREE_COLUMNS{size_t{10}};
        constexpr auto TREE_ROWS{size_t{8}};
        static_assert(TREE_COLUMNS * TREE_ROWS >= 74);
        constexpr auto BUSH_COUNT{size_t{30}};
        constexpr auto ORIGIN{math::Vector{0.0f, 0.0f, 0.0f}};
        scene.add_ground(Transform{ORIGIN, ORIGIN, {34.0f, 1.0f, 28.0f}});
        for (auto row{size_t{0}}; row < TREE_ROWS; ++row) {
            for (auto column{size_t{0}}; column < TREE_COLUMNS; ++column) {
                const auto index{(row * TREE_COLUMNS) + column};
                const auto size{
                    0.75f + (0.05f * static_cast<float>(index % 7))
                };
                scene.add_tree(
                    Transform{
                        {-27.0f + (static_cast<float>(column) * 6.0f)
                             + (0.35f
                                * (static_cast<float>((row + column) % 3)
                                   - 1.0f)),
                         0.0f,
                         -21.0f + (static_cast<float>(row) * 6.0f)
                             + (0.4f
                                * (static_cast<float>(((2 * row) + column) % 3)
                                   - 1.0f))},
                        {0.0f, static_cast<float>(index) * 0.41f, 0.0f},
                        {size, size, size}
                    }
                );
            }
        }
        for (auto index{size_t{0}}; index < BUSH_COUNT; ++index) {
            const auto size{2.0f + (0.2f * static_cast<float>(index % 7))};
            scene.add_bush(
                // The bush model extends below zero; align its base with trees.
                Transform{
                    {-24.0f + (static_cast<float>(index % 6) * 9.6f),
                     0.009f * size,
                     -18.0f + (static_cast<float>(index / 6) * 9.0f)},
                    {0.0f, static_cast<float>(index) * 0.4f, 0.0f},
                    {size, size, size}
                }
            );
        }
        scene.add_sun(
            Transform{{0.0f, 18.0f, -32.0f}, ORIGIN, {2.0f, 2.0f, 2.0f}}
        );
        for (const auto position : std::array{
                 math::Vector{-1.3f, 1.8f, 19.0f},
                 math::Vector{1.3f, 1.8f, -11.0f}
             }) {
            // Equal scale magnitudes keep the clocks round; Z faces the camera.
            scene.add_clock(Transform{position, ORIGIN, {0.9f, 0.9f, -0.9f}});
        }
        static_cast<void>(this->add_scene(std::move(scene).build()));
    }

    // Solar system
    {
        const auto solar_camera{Camera{
            {0.0f, 1.5f, 3.5f},
            -std::numbers::pi_v<float> / 2.0f,
            -0.4f,
            camera.get_projection_matrix()
        }};
        auto scene{SceneFactory::new_scene(solar_camera)};
        const auto earth_offset{transform::Transform{
            {0.55f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}
        }};
        const auto earth_scale{transform::Transform{
            {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.08f, 0.08f, 0.08f}
        }};
        const auto moon_local{transform::Transform{
            {0.18f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.035f, 0.035f, 0.035f}
        }};
        const auto marisa_offset{transform::Transform{
            {0.95f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}
        }};
        // Center the asset before turning its +Z-facing broom along the orbit.
        const auto marisa_local{
            transform::Transform{
                {0.0f, 0.0f, 0.0f},
                {0.0f, std::numbers::pi_v<float>, 0.0f},
                {1.0f, 1.0f, 1.0f}
            }
            * transform::Transform{
                {0.0f, -0.38f, 0.12f},
                {0.0f, 0.0f, 0.0f},
                {0.004f, 0.004f, 0.004f}
            }
        };
        scene.add_sun(
            transform::Transform{
                {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.18f, 0.18f, 0.18f}
            }
        );
        scene.add_earth(earth_offset * earth_scale);
        scene.add_moon(earth_offset * moon_local);
        scene.add_login(
            transform::Transform{
                {0.0f, -1.35f, 0.0f}, {-0.4f, 0.0f, 0.0f}, {0.35f, 0.35f, 0.35f}
            }
        );
        if (auto result{scene.add_model(
                "models/touhou/marisa/scene.gltf", marisa_offset * marisa_local
            )};
            !result) {
            return result;
        }
        static_cast<void>(this->add_scene(
            std::move(scene).build(
                [earth_offset,
                 earth_scale,
                 moon_local,
                 marisa_offset,
                 marisa_local](
                    const std::span<object::DrawableObject> objects,
                    [[maybe_unused]] PointLight& light
                ) -> void {
                    core::assert_eq(objects.size(), size_t{5});
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

                    objects[4].transform
                        = transform::
                              Transform{ORIGIN, {0.0f, time * 2.8f, 0.0f}, UNIT_SCALE}
                        * marisa_offset * marisa_local;
                }
            )
        ));
    }

    // Clock
    {
        auto scene{SceneFactory::new_scene(camera)};
        scene.add_clock(
            // Reflect Z to face the camera without mirroring the numerals.
            transform::Transform{
                {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, -1.0f}
            }
        );
        static_cast<void>(this->add_scene(std::move(scene).build()));
    }

    // Ambient (left), Lambert (right), Phong (top), Blinn-Phong (bottom).
    {
        // A slight tilt avoids looking parallel to the camera's world-up axis.
        const auto lighting_camera{Camera{
            {0.0f, 8.0f, 0.08f},
            -std::numbers::pi_v<float> / 2.0f,
            (-std::numbers::pi_v<float> / 2.0f) + 0.01f,
            camera.get_projection_matrix()
        }};
        auto scene{SceneFactory::new_scene(lighting_camera)};
        using enum object::shader::LightingModel;
        for (const auto& [position, lighting] : std::array{
                 std::pair{math::Vector{-2.0f, 0.0f, 0.0f}, Ambient},
                 std::pair{math::Vector{2.0f, 0.0f, 0.0f}, Lambert},
                 std::pair{math::Vector{0.0f, 0.0f, -2.0f}, Phong},
                 std::pair{math::Vector{0.0f, 0.0f, 2.0f}, BlinnPhong}
             }) {
            scene.add_sphere(
                transform::Transform{
                    position, {0.0f, 0.0f, 0.0f}, {0.8f, 0.8f, 0.8f}
                },
                {0.385f, 0.647f, 0.812f},
                lighting
            );
        }
        static_cast<void>(this->add_scene(std::move(scene).build()));
    }

    // Rotated, stretched spheres demonstrate the inverse-transpose normal
    // matrix.
    {
        const auto normal_camera{Camera{
            {0.0f, 0.0f, 9.0f},
            -std::numbers::pi_v<float> / 2.0f,
            0.0f,
            camera.get_projection_matrix()
        }};
        auto scene{SceneFactory::new_scene(normal_camera)};
        scene.set_light(
            PointLight{
                {0.0f, 3.0f, 4.0f},
                {1.0f, 1.0f, 1.0f},
                {0.01f, 0.01f, 0.01f},
                {1.0f, 0.0f, 0.0f}
            }
        );
        for (const auto x : {-1.8f, 1.8f}) {
            scene.add_sphere(
                transform::Transform{
                    {x, 0.0f, 0.0f}, {0.2f, 0.5f, 1.2f}, {1.3f, 0.55f, 0.8f}
                },
                {0.385f, 0.647f, 0.812f},
                object::shader::LightingModel::Phong
            );
        }
        static_cast<void>(this->add_scene(std::move(scene).build()));
    }

    // The back side remains dark even with the broad highlight of shininess 1.
    {
        const auto specular_camera{Camera{
            {0.0f, 0.0f, 6.0f},
            -std::numbers::pi_v<float> / 2.0f,
            0.0f,
            camera.get_projection_matrix()
        }};
        auto scene{SceneFactory::new_scene(specular_camera)};
        scene.add_sphere(
            transform::Transform{
                {0.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}
            },
            {0.385f, 0.647f, 0.812f},
            object::shader::LightingModel::Phong,
            1.0f,
            1.0f
        );
        static_cast<void>(this->add_scene(std::move(scene).build()));
    }

    // A centered sphere should remain round as the window changes shape.
    {
        const auto resize_camera{Camera{
            {0.0f, 0.0f, 7.0f},
            -std::numbers::pi_v<float> / 2.0f,
            0.0f,
            camera.get_projection_matrix()
        }};
        auto scene{SceneFactory::new_scene(resize_camera)};
        scene.set_light(
            PointLight{
                {-2.0f, 3.0f, 4.0f},
                {1.0f, 1.0f, 1.0f},
                {0.01f, 0.01f, 0.01f},
                {1.0f, 0.0f, 0.0f}
            }
        );
        scene.add_sphere(
            transform::Transform{
                {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}
            },
            {0.385f, 0.647f, 0.812f},
            object::shader::LightingModel::Phong
        );
        static_cast<void>(this->add_scene(std::move(scene).build()));
    }

    // One moving firefly lights the ground, trunks, leaves, and bushes.
    {
        using transform::Transform;
        constexpr auto ORIGIN{math::Vector{0.0f, 0.0f, 0.0f}};
        const auto night_camera{Camera{
            {0.0f, 3.5f, 13.0f},
            -std::numbers::pi_v<float> / 2.0f,
            -0.18f,
            camera.get_projection_matrix()
        }};
        auto scene{SceneFactory::new_scene(night_camera)};
        scene.set_light(
            PointLight{
                {0.0f, 2.0f, 2.0f},
                {2.0f, 2.4f, 1.2f},
                {0.002f, 0.003f, 0.006f},
                {1.0f, 0.35f, 0.44f}
            }
        );
        scene.add_lit_ground(Transform{ORIGIN, ORIGIN, {28.0f, 1.0f, 28.0f}});
        constexpr auto TREE_COLUMNS{size_t{10}};
        constexpr auto TREE_ROWS{size_t{8}};
        static_assert(TREE_COLUMNS * TREE_ROWS >= 74);
        for (auto row{size_t{0}}; row < TREE_ROWS; ++row) {
            for (auto column{size_t{0}}; column < TREE_COLUMNS; ++column) {
                const auto index{(row * TREE_COLUMNS) + column};
                const auto size{0.7f + (0.05f * static_cast<float>(index % 7))};
                scene.add_lit_tree(
                    Transform{
                        {-22.5f + (5.0f * static_cast<float>(column)),
                         0.0f,
                         -25.0f + (4.0f * static_cast<float>(row))},
                        {0.0f, 0.41f * static_cast<float>(index), 0.0f},
                        {size, size, size}
                    }
                );
            }
        }
        for (auto index{size_t{0}}; index < 20; ++index) {
            const auto size{2.0f + (0.2f * static_cast<float>(index % 4))};
            scene.add_lit_bush(
                Transform{
                    {-18.0f + (9.0f * static_cast<float>(index % 5)),
                     0.009f * size,
                     -18.0f + (6.0f * static_cast<float>(index / 5))},
                    {0.0f, 0.4f * static_cast<float>(index), 0.0f},
                    {size, size, size}
                }
            );
        }
        // This unlit marker emits no additional light; the scene has one light.
        scene.add_sphere(
            Transform{{0.0f, 2.0f, 2.0f}, ORIGIN, {0.07f, 0.07f, 0.07f}},
            {1.0f, 1.0f, 0.35f},
            object::shader::LightingModel::Ambient
        );
        static_cast<void>(this->add_scene(
            std::move(scene).build(
                [](const std::span<object::DrawableObject> objects,
                   PointLight& light) -> void {
                    // Ground, 80 trees, 20 bushes, and the final light marker.
                    core::assert_eq(objects.size(), size_t{102});
                    const auto time{static_cast<float>(glfwGetTime())};
                    light.set_position(
                        {2.0f * std::sin(time * 0.45f),
                         1.8f + (0.4f * std::sin(time * 0.7f)),
                         1.0f + (1.5f * std::cos(time * 0.45f))}
                    );
                    objects.back().transform = Transform{
                        light.get_position(),
                        {0.0f, 0.0f, 0.0f},
                        {0.07f, 0.07f, 0.07f}
                    };
                }
            )
        ));
    }

    // Classroom
    {
        const auto room_camera{Camera{
            {2.05f, 0.85f, 0.0f},
            2.55f,
            -0.38f,
            // A wider view fits the class from inside the room's back corner.
            math::Matrix<float, 4, 4>::perspective(
                std::numbers::pi_v<float> / 3.0f,
                static_cast<float>(framebuffer_width)
                    / static_cast<float>(framebuffer_height),
                0.1f,
                100.0f
            )
        }};
        auto scene{SceneFactory::new_scene(room_camera)};
        if (auto result{scene.add_skybox("models/skybox/cubemap")}; !result) {
            return result;
        }
        scene.set_light(
            PointLight{
                {0.0f, 1.0f, 2.0f},
                {1.0f, 1.0f, 1.0f},
                {0.01f, 0.01f, 0.01f},
                {1.0f, 0.0f, 0.0f}
            }
        );
        // Share the room's transform so fixtures keep its scale and placement.
        const auto room_transform{transform::Transform{
            {0.0f, -0.8f, 2.0f}, {0.0f, 0.0f, 0.0f}, {0.6f, 0.6f, 0.6f}
        }};
        if (auto result{
                scene.add_model("models/EB408/scene.gltf", room_transform)
            };
            !result) {
            return result;
        }
        if (auto result{scene.add_model(
                "models/door/scene.gltf",
                // Face outward so the leaf sits at the wall's outer border
                // while the frame fills the doorway's depth.
                room_transform
                    * transform::
                        Transform{{-3.01019f, 0.0f, 3.84335f}, {0.0f, -0.013596f, 0.0f}, {1.0f, 1.0f, 1.0f}}
            )};
            !result) {
            return result;
        }
        if (auto result{scene.add_model(
                "models/whiteboard/scene.gltf",
                // Facing the door from inside, the right wall is the -X wall.
                // Match its angle and put the board's back against the wall.
                room_transform
                    * transform::
                        Transform{{-3.910386f, 0.0f, 2.000493f}, {0.0f, -0.021938f, 0.0f}, {1.0f, 1.0f, 1.0f}}
            )};
            !result) {
            return result;
        }
        if (auto result{scene.add_model(
                "models/touhou/cirno/scene.gltf",
                room_transform
                    * transform::
                        Transform{{-2.8f, 0.0f, 2.0f}, {0.0f, 2.1f, 0.0f}, {1.4f, 1.4f, 1.4f}}
            )};
            !result) {
            return result;
        }
        // Fit the 146.33317-unit source model to a 1.1-unit classroom height.
        constexpr auto MARISA_SCALE{1.1f / 146.33317f};
        if (auto result{scene.add_model(
                "models/touhou/marisa/scene.gltf",
                room_transform
                    * transform::
                        Transform{{2.25f, 0.0f, -2.8f}, {0.0f, -std::numbers::pi_v<float> / 2.0f, 0.0f}, {MARISA_SCALE, MARISA_SCALE, MARISA_SCALE}}
                    // The original broom sits above and behind the origin;
                    // center its footprint and align its base with the floor.
                    * transform::
                        Transform{{0.0f, -21.48372f, 30.52678f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}
            )};
            !result) {
            return result;
        }
        constexpr auto STUDENTS{std::array{
            "sanae",
            "rumia",
            "tsukasa",
            "junko",
            "koishi",
            "suwako",
            "remilia",
            "yuyuko",
            "mokou",
            "yuuka",
            "shion",
            "inu_sakuya",
            "youmu",
            "flandre",
            "alice",
            "reimu"
        }};
        for (auto index{size_t{0}}; index < STUDENTS.size(); ++index) {
            // Assets have unit height and a centered floor origin so their
            // different scan scales do not affect the classroom layout.
            if (auto result{scene.add_model(
                    std::format("models/touhou/{}/scene.gltf", STUDENTS[index]),
                    room_transform
                        * transform::
                            Transform{{-1.5f + (1.25f * static_cast<float>(index / 4)), 0.0f, -1.5f + (1.5f * static_cast<float>(index % 4))}, {0.0f, -std::numbers::pi_v<float> / 2.0f, 0.0f}, {1.0f, 1.0f, 1.0f}}
                )};
                !result) {
                return result;
            }
        }
        this->switch_scene(this->add_scene(std::move(scene).build()));
    }
    return {};
}

auto App::add_scene(Scene&& scene) -> size_t {
    this->scenes.emplace_back(std::move(scene));
    return this->scenes.size() - 1;
}

auto App::switch_scene(const size_t index) -> void {
    core::assert_that(index < this->scenes.size());
    this->active_scene_index = index;
    const auto title{
        std::format("ZPG | Scene {}/{}", index + 1, this->scenes.size())
    };
    glfwSetWindowTitle(this->window, title.c_str());
}

auto App::run() -> void {
    core::assert_that(this->active_scene_index.has_value());
    constexpr auto SCENE_KEYS{std::array{
        GLFW_KEY_1,
        GLFW_KEY_2,
        GLFW_KEY_3,
        GLFW_KEY_4,
        GLFW_KEY_5,
        GLFW_KEY_6,
        GLFW_KEY_7,
        GLFW_KEY_8,
        GLFW_KEY_9,
        GLFW_KEY_0
    }};
    auto previous_frame_time{glfwGetTime()};
    auto previous_cursor{input::InputManager::get_cursor_position()};
    auto was_looking{false};

    while (glfwWindowShouldClose(window) == 0) {
        const auto frame_time{glfwGetTime()};
        const auto delta_seconds{
            static_cast<float>(frame_time - previous_frame_time)
        };
        core::assert_that(std::isfinite(delta_seconds) && delta_seconds >= 0);
        previous_frame_time = frame_time;
        const auto previous_scene_index{*this->active_scene_index};
        const auto focused{
            glfwGetWindowAttrib(this->window, GLFW_FOCUSED) == GLFW_TRUE
        };
        if (focused) {
            for (auto index{size_t{0}};
                 index < this->scenes.size() && index < SCENE_KEYS.size();
                 ++index) {
                if (input::InputManager::is_key_down(SCENE_KEYS[index])) {
                    if (index != *this->active_scene_index) {
                        this->switch_scene(index);
                    }
                    break;
                }
            }
        }

        const auto scene_index{*this->active_scene_index};
        const auto cursor{input::InputManager::get_cursor_position()};
        const auto looking{
            focused
            && input::InputManager::is_mouse_button_down(
                GLFW_MOUSE_BUTTON_RIGHT
            )
        };
        if (focused) {
            if (input::InputManager::is_key_down(GLFW_KEY_ESCAPE)) {
                glfwSetWindowShouldClose(this->window, GLFW_TRUE);
            }
            auto& camera{this->scenes.at(scene_index).camera};
            // Starting a drag or switching scenes establishes a fresh baseline.
            if (looking && was_looking && scene_index == previous_scene_index) {
                constexpr auto SENSITIVITY{0.003f};
                camera.rotate(
                    static_cast<float>(cursor[0] - previous_cursor[0])
                        * SENSITIVITY,
                    static_cast<float>(previous_cursor[1] - cursor[1])
                        * SENSITIVITY
                );
            }
            const auto forward{
                static_cast<float>(input::InputManager::is_key_down(GLFW_KEY_W))
                - static_cast<float>(
                    input::InputManager::is_key_down(GLFW_KEY_S)
                )
            };
            const auto right{
                static_cast<float>(input::InputManager::is_key_down(GLFW_KEY_D))
                - static_cast<float>(
                    input::InputManager::is_key_down(GLFW_KEY_A)
                )
            };
            if (const auto length{std::hypot(forward, right)}; length > 0.0f) {
                // Normalization prevents diagonal movement from being faster.
                const auto distance{5.0f * delta_seconds / length};
                camera.move(forward * distance, right * distance);
            }
        }
        previous_cursor = cursor;
        was_looking = looking;
        // An update can grow the scene vector, so reacquire the scene to draw
        // it.
        this->scenes.at(scene_index).update();

        auto framebuffer_width{0};
        auto framebuffer_height{0};
        glfwGetFramebufferSize(
            this->window, &framebuffer_width, &framebuffer_height
        );
        // Minimized windows have no drawable area or valid perspective aspect.
        if (framebuffer_width > 0 && framebuffer_height > 0) {
            auto& scene{this->scenes.at(scene_index)};
            glViewport(0, 0, framebuffer_width, framebuffer_height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            scene.camera.set_aspect_ratio(
                static_cast<float>(framebuffer_width)
                / static_cast<float>(framebuffer_height)
            );
            scene.draw();
        }

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

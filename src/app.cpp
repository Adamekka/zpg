#include "app.hpp"
#include "input/input_manager.hpp"
#include "scene_factory.hpp"
#include <array>
#include <cstdio>
#include <cstring>
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
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
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

    auto stencil_bits{0};
    glGetFramebufferAttachmentParameteriv(
        GL_DRAW_FRAMEBUFFER,
        GL_STENCIL,
        GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
        &stencil_bits
    );
    if (stencil_bits < 8) {
        core::panic("Object picking requires an eight-bit stencil buffer.");
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    glfwSetWindowUserPointer(this->window, this);
    // Toggles react once per press; polling would flip them every frame.
    glfwSetKeyCallback(
        this->window,
        [](GLFWwindow* const window,
           const int32_t key,
           [[maybe_unused]] const int32_t scancode,
           const int32_t action,
           [[maybe_unused]] const int32_t modifiers) -> void {
            if (action != GLFW_PRESS) {
                return;
            }
            auto* const app{
                static_cast<App*>(glfwGetWindowUserPointer(window))
            };
            core::assert_ne(app, nullptr);
            auto& scene{app->scenes.at(*app->active_scene_index)};
            if (key == GLFW_KEY_F) {
                scene.flashlight_on = !scene.flashlight_on;
            } else if (key == GLFW_KEY_G) {
                auto& falloff{scene.flashlight.falloff};
                falloff = falloff == Light::Falloff::Hard
                            ? Light::Falloff::Smooth
                            : Light::Falloff::Hard;
            }
        }
    );
    // Queue presses rather than polling, which can miss a short click entirely.
    glfwSetMouseButtonCallback(
        this->window,
        [](GLFWwindow* const window,
           const int32_t button,
           const int32_t action,
           [[maybe_unused]] const int32_t modifiers) -> void {
            if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS
                || glfwGetWindowAttrib(window, GLFW_FOCUSED) != GLFW_TRUE) {
                return;
            }
            auto* const app{
                static_cast<App*>(glfwGetWindowUserPointer(window))
            };
            core::assert_ne(app, nullptr);
            auto position{std::array<double, 2>{}};
            glfwGetCursorPos(window, position.data(), &position[1]);
            app->pending_clicks.emplace_back(position);
        }
    );
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
        scene.enable_sphere_insertion(0.3f);
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
                    [[maybe_unused]] const std::span<Light> lights
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
        scene.set_lights({Light::point(
            {0.0f, 3.0f, 4.0f},
            {1.0f, 1.0f, 1.0f},
            {0.01f, 0.01f, 0.01f},
            {1.0f, 0.0f, 0.0f}
        )});
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
        scene.set_lights({Light::point(
            {-2.0f, 3.0f, 4.0f},
            {1.0f, 1.0f, 1.0f},
            {0.01f, 0.01f, 0.01f},
            {1.0f, 0.0f, 0.0f}
        )});
        scene.add_sphere(
            transform::Transform{
                {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}
            },
            {0.385f, 0.647f, 0.812f},
            object::shader::LightingModel::Phong
        );
        static_cast<void>(this->add_scene(std::move(scene).build()));
    }

    // Night forest lit by the moon, a street lamp, and fireflies.
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
        constexpr auto LAMP_POSITION{math::Vector{-4.0f, 3.6f, 4.0f}};
        constexpr auto FIREFLY_COUNT{size_t{6}};
        auto lights{std::vector{
            Light::directional(
                {0.4f, -1.0f, -0.6f},
                {0.10f, 0.13f, 0.22f},
                {0.004f, 0.006f, 0.012f}
            ),
            Light::spot(
                LAMP_POSITION,
                {0.0f, -1.0f, 0.0f},
                {3.0f, 2.3f, 1.2f},
                {0.0f, 0.0f, 0.0f},
                {1.0f, 0.09f, 0.032f},
                0.45f,
                0.7f,
                Light::Falloff::Smooth
            )
        }};

        // Fireflies fly in small loops around random centers.
        struct Firefly final {
            math::Vector<float> center;
            float phase;
        };

        auto random{std::mt19937{std::random_device{}()}};
        auto fireflies{std::vector<Firefly>{}};
        for (auto index{size_t{0}}; index < FIREFLY_COUNT; ++index) {
            const auto between{[&random](const float low, const float high) {
                return std::uniform_real_distribution{low, high}(random);
            }};
            fireflies.push_back(
                Firefly{
                    {between(-6.0f, 6.0f),
                     between(0.6f, 2.2f),
                     between(-6.0f, 5.0f)},
                    between(0.0f, 2.0f * std::numbers::pi_v<float>)
                }
            );
            lights.push_back(
                Light::point(
                    fireflies.back().center,
                    {1.0f, 1.4f, 0.35f},
                    {0.0f, 0.0f, 0.0f},
                    {1.0f, 0.7f, 1.8f}
                )
            );
        }
        scene.set_lights(lights);
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
        // Unlit markers show where the lamp and fireflies glow.
        scene.add_sphere(
            Transform{LAMP_POSITION, ORIGIN, {0.15f, 0.08f, 0.15f}},
            {1.0f, 0.85f, 0.5f},
            object::shader::LightingModel::Ambient
        );
        for (const auto& firefly : fireflies) {
            scene.add_sphere(
                Transform{firefly.center, ORIGIN, {0.05f, 0.05f, 0.05f}},
                {0.9f, 1.0f, 0.35f},
                object::shader::LightingModel::Ambient
            );
        }
        static_cast<void>(this->add_scene(
            std::move(scene).build(
                [fireflies](
                    const std::span<object::DrawableObject> objects,
                    const std::span<Light> lights
                ) -> void {
                    // Ground, 80 trees, 20 bushes, the lamp marker, then one
                    // marker per firefly.
                    core::assert_eq(
                        objects.size(), size_t{102} + fireflies.size()
                    );
                    // Moon and lamp, then one light per firefly.
                    core::assert_eq(
                        lights.size(), size_t{2} + fireflies.size()
                    );
                    const auto time{static_cast<float>(glfwGetTime())};
                    for (auto index{size_t{0}}; index < fireflies.size();
                         ++index) {
                        const auto& [center, phase]{fireflies[index]};
                        const auto angle{(0.5f * time) + phase};
                        auto& light{lights[2 + index]};
                        // A small loop that also bobs up and down.
                        light.position
                            = {center.x + (1.5f * std::cos(angle)),
                               center.y + (0.3f * std::sin(3.0f * angle)),
                               center.z + (1.5f * std::sin(angle))};
                        objects[objects.size() - fireflies.size() + index]
                            .transform = Transform{
                            light.position,
                            {0.0f, 0.0f, 0.0f},
                            {0.05f, 0.05f, 0.05f}
                        };
                    }
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
        scene.enable_sphere_insertion(0.1f);
        if (auto result{scene.add_skybox("models/skybox/cubemap")}; !result) {
            return result;
        }
        scene.set_lights({Light::point(
            {0.0f, 1.0f, 2.0f},
            {1.0f, 1.0f, 1.0f},
            {0.01f, 0.01f, 0.01f},
            {1.0f, 0.0f, 0.0f}
        )});
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
    constexpr auto MODE_SHORTCUTS{std::array{
        std::pair{GLFW_KEY_P, Scene::Mode::Select},
        std::pair{GLFW_KEY_I, Scene::Mode::Insert},
        std::pair{GLFW_KEY_M, Scene::Mode::Move}
    }};
    auto previous_title{std::string{}};

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
        for (const auto [key, mode] : MODE_SHORTCUTS) {
            if (focused && input::InputManager::is_key_down(key)) {
                this->scenes.at(scene_index).set_mode(mode);
            }
        }
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
        this->scenes.at(scene_index).update(delta_seconds);

        auto framebuffer_width{0};
        auto framebuffer_height{0};
        glfwGetFramebufferSize(
            this->window, &framebuffer_width, &framebuffer_height
        );
        // Minimized windows have no drawable area or valid perspective aspect.
        if (framebuffer_width > 0 && framebuffer_height > 0) {
            auto& scene{this->scenes.at(scene_index)};
            glViewport(0, 0, framebuffer_width, framebuffer_height);
            scene.camera.set_aspect_ratio(
                static_cast<float>(framebuffer_width)
                / static_cast<float>(framebuffer_height)
            );
            auto window_width{0};
            auto window_height{0};
            glfwGetWindowSize(this->window, &window_width, &window_height);
            if (focused && !looking && window_width > 0 && window_height > 0) {
                for (const auto& click : this->pending_clicks) {
                    if (click[0] < 0.0 || click[0] >= window_width
                        || click[1] < 0.0 || click[1] >= window_height) {
                        continue;
                    }
                    // GLFW uses top-left window coordinates; OpenGL reads
                    // bottom-left framebuffer pixels, including Retina scaling.
                    const auto x{static_cast<int32_t>(
                        click[0] * framebuffer_width / window_width
                    )};
                    const auto y{
                        framebuffer_height - 1
                        - static_cast<int32_t>(
                            click[1] * framebuffer_height / window_height
                        )
                    };
                    if (const auto hit{scene.pick(
                            x, y, framebuffer_width, framebuffer_height
                        )}) {
                        scene.click(*hit);
                    } else {
                        scene.miss();
                    }
                    std::println("{}", scene.interaction_status);
                }
            }
            // The pick pass changes depth/stencil, so redraw before swapping.
            glClear(
                GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
                | GL_STENCIL_BUFFER_BIT
            );
            scene.draw();
            const auto title{std::format(
                "ZPG | {} | Scene {}/{} | P select, I place, M path, "
                "F flashlight, G edge",
                scene.interaction_status,
                scene_index + 1,
                this->scenes.size()
            )};
            if (title != previous_title) {
                glfwSetWindowTitle(this->window, title.c_str());
                previous_title = title;
            }
        }
        this->pending_clicks.clear();

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

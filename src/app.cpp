#include "app.hpp"
#include "core/assert.hpp"
#include "core/panic.hpp"
#include "input/input_manager.hpp"

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
        } else if (input::InputManager::is_key_down(GLFW_KEY_2)) {
            this->switch_scene(1);
        } else if (input::InputManager::is_key_down(GLFW_KEY_3)) {
            this->switch_scene(2);
        } else if (input::InputManager::is_key_down(GLFW_KEY_4)) {
            this->switch_scene(3);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const auto scene_index{*this->active_scene_index};
        // An update can grow the scene vector, so reacquire the scene to draw it.
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

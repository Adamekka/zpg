#pragma once

#include "scene.hpp"

class App final {
  public:
    App(const App&) = delete;
    App(App&&) = delete;

    auto operator=(const App&) -> App& = delete;
    auto operator=(App&&) -> App& = delete;

    [[nodiscard]] static auto instance() -> App&;

    auto init_opengl() -> void;
    auto create_objects() -> void;

    [[nodiscard]] auto add_scene(Scene&& scene) -> size_t;
    auto switch_scene(size_t index) -> void;

    auto run() -> void;

  private:
    // nullptr means not ready
    GLFWwindow* window{nullptr};

    std::vector<Scene> scenes;
    std::optional<size_t> active_scene_index;

    App() = default;

    ~App();
};

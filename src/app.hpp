#pragma once

#include "gl.hpp"
#include "object/object.hpp"
#include <unordered_map>
#include <vector>

class App final {
  public:
    App(const App&) = delete;
    App(App&&) = delete;

    auto operator=(const App&) -> App& = delete;
    auto operator=(App&&) -> App& = delete;

    [[nodiscard]] static auto instance() -> App&;

    auto init_opengl() -> void;

    auto create_shaders() -> void;
    auto create_models() -> void;

    auto run() const -> void;

  private:
    // nullptr means not ready
    GLFWwindow* window{nullptr};

    std::unordered_map<std::string, object::shader::ShaderProgram>
        shader_programs;
    std::vector<object::Object> objects;

    App() = default;

    ~App();
};

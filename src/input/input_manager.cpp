#include "input_manager.hpp"
#include "../core/assert.hpp"
#include "../gl.hpp"

namespace input {

auto InputManager::is_key_down(const int32_t key) -> bool {
    core::assert_that(key >= 0);
    core::assert_that(key <= GLFW_KEY_LAST);
    auto* const window{glfwGetCurrentContext()};
    core::assert_ne(window, nullptr);
    return glfwGetKey(window, key) == GLFW_PRESS;
}

auto InputManager::is_mouse_button_down(const int32_t button) -> bool {
    core::assert_that(button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST);
    auto* const window{glfwGetCurrentContext()};
    core::assert_ne(window, nullptr);
    return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

auto InputManager::get_cursor_position() -> std::array<double, 2> {
    auto* const window{glfwGetCurrentContext()};
    core::assert_ne(window, nullptr);
    auto position{std::array<double, 2>{}};
    glfwGetCursorPos(window, position.data(), &position[1]);
    return position;
}

} // namespace input

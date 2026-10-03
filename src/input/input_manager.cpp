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

} // namespace input

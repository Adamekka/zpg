#pragma once

#include <cstdint>

namespace input {

class InputManager final {
  public:
    InputManager() = delete;

    InputManager(const InputManager&) = delete;
    InputManager(InputManager&&) = delete;

    ~InputManager() = delete;

    auto operator=(const InputManager&) -> InputManager& = delete;
    auto operator=(InputManager&&) -> InputManager& = delete;

    [[nodiscard]] static auto is_key_down(int32_t key) -> bool;
};

} // namespace input

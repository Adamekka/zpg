#pragma once

#include "vbo.hpp"

namespace object::mesh {

class VAO final {
  public:
    explicit VAO(const VBO& vbo);

    VAO(const VAO&) = delete;
    VAO(VAO&&) noexcept;

    ~VAO();

    auto operator=(const VAO&) -> VAO& = delete;
    auto operator=(VAO&&) noexcept -> VAO&;

    auto bind() const -> void;
    static auto unbind() -> void;

  private:
    uint32_t id{0};
};

} // namespace object::mesh

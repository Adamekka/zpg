#pragma once

#include "vertex.hpp"
#include <span>

namespace object::mesh {

class VBO final {
  public:
    explicit VBO(std::span<const Vertex> vertices);

    VBO(const VBO&) = delete;
    VBO(VBO&&) noexcept;

    ~VBO();

    auto operator=(const VBO&) -> VBO& = delete;
    auto operator=(VBO&&) noexcept -> VBO&;

    auto bind() const -> void;
    static auto unbind() -> void;

  private:
    uint32_t id{0};
};

} // namespace object::mesh

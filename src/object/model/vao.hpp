#pragma once

#include "vbo.hpp"
#include "vertex_attribute.hpp"

namespace object::model {

class VAO final {
  public:
    VAO(const VBO& vbo,
        std::span<const VertexAttribute> attributes,
        int32_t stride_bytes);

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

} // namespace object::model

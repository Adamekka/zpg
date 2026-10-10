#include "vao.hpp"
#include "../../gl.hpp"
#include <utility>

namespace object::model {

VAO::VAO(
    const VBO& vbo,
    const std::span<const VertexAttribute> attributes,
    const int32_t stride_bytes
) {
    glGenVertexArrays(1, &this->id);

    this->bind();
    vbo.bind();

    auto offset_bytes{size_t{0}};
    for (auto index{uint32_t{0}}; index < attributes.size(); ++index) {
        const auto& attribute{attributes[index]};
        glVertexAttribPointer(
            index,
            attribute.component_count,
            GL_FLOAT,
            GL_FALSE,
            stride_bytes,
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,performance-no-int-to-ptr)
            reinterpret_cast<const void*>(offset_bytes)
        );
        glEnableVertexAttribArray(index);
        offset_bytes
            += static_cast<size_t>(attribute.component_count) * sizeof(float);
    }
}

VAO::VAO(VAO&& other) noexcept
    : id{std::exchange(other.id, 0)} {}

VAO::~VAO() {
    if (this->id != 0) {
        glDeleteVertexArrays(1, &this->id);
    }
}

auto VAO::operator=(VAO&& other) noexcept -> VAO& {
    if (this != &other) {
        glDeleteVertexArrays(1, &this->id);
        this->id = std::exchange(other.id, 0);
    }

    return *this;
}

auto VAO::bind() const -> void {
    glBindVertexArray(this->id);
}

auto VAO::unbind() -> void {
    glBindVertexArray(0);
}

} // namespace object::model

#include "vbo.hpp"
#include "../../core/assert.hpp"
#include "../../gl.hpp"

namespace object::model {

VBO::VBO(const std::span<const float> data) {
    core::assert_that(
        data.size()
        <= static_cast<size_t>(std::numeric_limits<GLsizeiptr>::max())
               / sizeof(float)
    );
    glGenBuffers(1, &this->id);
    this->bind();
    glBufferData(
        GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(data.size_bytes()),
        data.data(),
        GL_STATIC_DRAW
    );
}

VBO::VBO(VBO&& other) noexcept
    : id{std::exchange(other.id, 0)} {}

VBO::~VBO() {
    if (this->id != 0) {
        glDeleteBuffers(1, &this->id);
    }
}

auto VBO::operator=(VBO&& other) noexcept -> VBO& {
    if (this != &other) {
        glDeleteBuffers(1, &this->id);
        this->id = std::exchange(other.id, 0);
    }

    return *this;
}

auto VBO::bind() const -> void {
    glBindBuffer(GL_ARRAY_BUFFER, this->id);
}

auto VBO::unbind() -> void {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace object::model

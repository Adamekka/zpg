#include "object.hpp"

namespace object {

auto Object::draw() const -> void {
    this->shader_program->bind();
    this->mesh.draw();
}

} // namespace object

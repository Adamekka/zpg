#include "drawable_object.hpp"

namespace object {

auto DrawableObject::draw() const -> void {
    this->shader_program->bind();
    this->mesh.draw();
}

} // namespace object

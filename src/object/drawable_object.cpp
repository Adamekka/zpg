#include "drawable_object.hpp"

namespace object {

auto DrawableObject::draw() const -> void {
    this->shader_program->bind();
    this->shader_program->get_args()
        .at("model_matrix")
        .set_uniform(this->transform.get_matrix());
    this->mesh.draw();
}

} // namespace object

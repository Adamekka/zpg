#include "drawable_object.hpp"

namespace object {

auto DrawableObject::draw(
    const math::Matrix<float, 4, 4>& view_matrix,
    const math::Matrix<float, 4, 4>& projection_matrix
) const -> void {
    this->shader_program->bind();
    this->shader_program->get_args().at("view_matrix").set_uniform(view_matrix);
    this->shader_program->get_args()
        .at("projection_matrix")
        .set_uniform(projection_matrix);
    this->shader_program->get_args()
        .at("model_matrix")
        .set_uniform(this->transform.get_matrix());
    this->mesh.draw();
}

} // namespace object

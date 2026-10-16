#include "scene.hpp"
#include "core/assert.hpp"
#include <utility>

Scene::Scene(
    const Camera& camera,
    ShaderPrograms&& shader_programs,
    std::vector<object::DrawableObject>&& objects,
    std::function<void(std::span<object::DrawableObject>)> update_handler
)
    : camera{camera}
    , shader_programs{std::move(shader_programs)}
    , objects{std::move(objects)}
    , update_handler{std::make_unique<
          std::function<void(std::span<object::DrawableObject>)>>(
          std::move(update_handler)
      )} {
    core::assert_ne(*this->update_handler, nullptr);
}

auto Scene::update() -> void {
    core::assert_ne(this->update_handler, nullptr);
    (*this->update_handler)(this->objects);
}

auto Scene::draw() const -> void {
    const auto view_matrix{this->camera.get_view_matrix()};
    for (const auto& object : this->objects) {
        object.draw(view_matrix, this->camera.projection_matrix);
    }
}

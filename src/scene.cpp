#include "scene.hpp"
#include "core/assert.hpp"
#include <utility>

Scene::Scene(
    std::unordered_map<std::string, object::shader::ShaderProgram>&&
        shader_programs,
    std::vector<object::DrawableObject>&& objects,
    std::function<void(std::span<object::DrawableObject>)> update_handler
)
    : shader_programs{std::move(shader_programs)}
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
    for (const auto& object : this->objects) {
        object.draw();
    }
}

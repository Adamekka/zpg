#include "scene.hpp"
#include "core/assert.hpp"
#include <utility>

Scene::Scene(
    Camera camera,
    const PointLight& light,
    ShaderPrograms&& shader_programs,
    std::vector<object::DrawableObject>&& objects,
    Background&& background,
    std::function<void(std::span<object::DrawableObject>, PointLight&)>
        update_handler
)
    : camera{std::move(camera)}
    , light{light}
    , shader_programs{std::move(shader_programs)}
    , objects{std::move(objects)}
    , background{std::move(background)}
    , update_handler{std::make_unique<
          std::function<void(std::span<object::DrawableObject>, PointLight&)>>(
          std::move(update_handler)
      )} {
    core::assert_ne(*this->update_handler, nullptr);
    for (auto& [paths, shader] : this->shader_programs) {
        this->camera.add_observer(shader);
    }
}

auto Scene::update() -> void {
    core::assert_ne(this->update_handler, nullptr);
    (*this->update_handler)(this->objects, this->light);
}

auto Scene::draw() -> void {
    for (auto& [paths, shader] : this->shader_programs) {
        shader.bind();
        this->light.bind(shader);
    }
    const auto view_matrix{this->camera.get_view_matrix()};
    for (const auto& object : this->objects) {
        object.draw(view_matrix);
    }
    if (auto* const skybox{std::get_if<object::Skybox>(&this->background)}) {
        skybox->draw();
    }
}

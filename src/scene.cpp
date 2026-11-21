#include "scene.hpp"
#include "../models/sphere.hpp"
#include "core/assert.hpp"
#include "core/defer.hpp"
#include <algorithm>
#include <boost/uuid/uuid_io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>

Scene::Scene(
    Camera camera,
    std::vector<Light>&& lights,
    ShaderPrograms&& shader_programs,
    std::vector<object::DrawableObject>&& objects,
    Background&& background,
    const Insertion& insertion,
    std::function<void(std::span<object::DrawableObject>, std::span<Light>)>
        update_handler
)
    : camera{std::move(camera)}
    , lights{std::move(lights)}
    , shader_programs{std::move(shader_programs)}
    , objects{std::move(objects)}
    , background{std::move(background)}
    , insertion{insertion}
    , update_handler{std::make_unique<std::function<
          void(std::span<object::DrawableObject>, std::span<Light>)>>(
          std::move(update_handler)
      )} {
    core::assert_ne(*this->update_handler, nullptr);
    if (const auto* const settings{
            std::get_if<SphereInsertion>(&this->insertion)
        }) {
        core::assert_that(
            std::isfinite(settings->radius) && settings->radius > 0.0f
        );
    }
    for (auto& [paths, shader] : this->shader_programs) {
        this->camera.add_observer(shader);
    }
}

auto Scene::update(const float delta_seconds) -> void {
    core::assert_that(std::isfinite(delta_seconds) && delta_seconds >= 0.0f);
    core::assert_ne(this->update_handler, nullptr);
    (*this->update_handler)(this->objects, this->lights);
    this->flashlight.position = this->camera.get_position();
    this->flashlight.direction = this->camera.get_direction();
    for (auto& placed : this->placed_objects) {
        auto* const motion{std::get_if<Motion>(&placed.motion)};
        if (motion == nullptr) {
            continue;
        }
        // A round trip takes six seconds. Reflecting t keeps it in [0, 1].
        motion->elapsed_seconds
            = std::fmod(motion->elapsed_seconds + delta_seconds, 6.0);
        const auto phase{static_cast<float>(motion->elapsed_seconds / 3.0)};
        const auto t{phase <= 1.0f ? phase : 2.0f - phase};
        const auto radius{std::get<SphereInsertion>(this->insertion).radius};
        placed.object.transform = transform::Transform{
            {motion->start.x + t * (motion->end.x - motion->start.x),
             motion->start.y + t * (motion->end.y - motion->start.y),
             motion->start.z + t * (motion->end.z - motion->start.z)},
            {0.0f, 0.0f, 0.0f},
            {radius, radius, radius}
        };
    }
}

auto Scene::draw() -> void {
    auto lights{this->lights};
    if (this->flashlight_on) {
        lights.push_back(this->flashlight);
    }
    for (auto& [paths, shader] : this->shader_programs) {
        shader.bind();
        Light::bind(shader, lights);
    }
    const auto view_matrix{this->camera.get_view_matrix()};
    for (auto index{size_t{0}};
         index < this->objects.size() + this->placed_objects.size();
         ++index) {
        this->object_at(index).draw(view_matrix, object::DrawPass::Color);
    }
    if (auto* const skybox{std::get_if<object::Skybox>(&this->background)}) {
        skybox->draw();
    }
    if (this->selected_id) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // Bias the wire overlay toward the camera to avoid coplanar flicker.
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);
        this->find_object(*this->selected_id)
            .draw_selection(this->shader_programs.at(
                {"shaders/basic.vert", "shaders/selection.frag"}
            ));
        glDisable(GL_POLYGON_OFFSET_LINE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

auto Scene::pick(
    const int32_t x, const int32_t y, const int32_t width, const int32_t height
) const -> std::optional<Hit> {
    core::assert_that(width > 0 && height > 0);
    core::assert_that(x >= 0 && x < width && y >= 0 && y < height);
    // Pick at the pixel center, without averaging coverage at MSAA edges.
    glDisable(GL_MULTISAMPLE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, 1, 1);
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    defer({
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_MULTISAMPLE);
    });
    const auto view{this->camera.get_view_matrix()};
    // Zero identifies the background. Batches keep UUIDs independent of the
    // eight-bit stencil limit and keep every object usable beyond 255 objects.
    constexpr auto BATCH_SIZE{size_t{255}};
    const auto object_count{this->objects.size() + this->placed_objects.size()};
    for (auto first{size_t{0}}; first < object_count; first += BATCH_SIZE) {
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        for (auto index{size_t{0}}; index < object_count; ++index) {
            const auto token{
                index >= first && index - first < BATCH_SIZE
                    ? static_cast<int32_t>(index - first + 1)
                    : int32_t{0}
            };
            glStencilFunc(GL_ALWAYS, token, 0xFF);
            // Objects outside this batch still occlude objects inside it.
            this->object_at(index).draw(view, object::DrawPass::Picking);
        }
        auto token{uint32_t{0}};
        auto depth{1.0f};
        glReadPixels(x, y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &token);
        glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
        if (token == 0) {
            continue;
        }
        const auto index{first + token - 1};
        core::assert_that(index < object_count);
        const auto point{glm::unProject(
            glm::vec3{
                static_cast<float>(x) + 0.5f,
                static_cast<float>(y) + 0.5f,
                depth
            },
            glm::make_mat4(view.values.data()),
            glm::make_mat4(this->camera.get_projection_matrix().values.data()),
            glm::vec4{
                0.0f,
                0.0f,
                static_cast<float>(width),
                static_cast<float>(height)
            }
        )};
        return Hit{this->object_at(index).id, {point.x, point.y, point.z}};
    }
    return std::nullopt;
}

auto Scene::object_at(const size_t index) const
    -> const object::DrawableObject& {
    if (index < this->objects.size()) {
        return this->objects[index];
    }
    core::assert_that(
        index - this->objects.size() < this->placed_objects.size()
    );
    return this->placed_objects[index - this->objects.size()].object;
}

auto Scene::find_object(const core::types::ID id) const
    -> const object::DrawableObject& {
    for (auto index{size_t{0}};
         index < this->objects.size() + this->placed_objects.size();
         ++index) {
        const auto& object{this->object_at(index)};
        if (object.id == id) {
            return object;
        }
    }
    core::panic("Object ID does not belong to this scene.");
}

auto Scene::place_sphere(const math::Vector<float> position)
    -> core::types::ID {
    const auto radius{std::get<SphereInsertion>(this->insertion).radius};
    this->placed_objects.emplace_back(
        object::DrawableObject::from_colored_data(
            SPHERE,
            {1.0f, 0.35f, 0.08f},
            // Keep placed spheres visible in the forest's unlit scene as well.
            this->shader_programs.at(
                {"shaders/phong.vert", "shaders/ambient.frag"}
            ),
            transform::Transform{
                position, {0.0f, 0.0f, 0.0f}, {radius, radius, radius}
            },
            32.0f,
            0.5f
        )
    );
    this->selected_id = this->placed_objects.back().object.id;
    return this->placed_objects.back().object.id;
}

auto Scene::set_mode(const Mode mode) -> void {
    // Mode keys are polled every frame, so holding one must not reset status.
    if (const auto* const current{std::get_if<Mode>(&this->interaction)};
        current != nullptr && *current == mode) {
        return;
    }
    if (mode != Mode::Select
        && std::holds_alternative<NoInsertion>(this->insertion)) {
        this->interaction_status
            = "Placement is available in the forest (2) and classroom (0)";
        return;
    }
    if (const auto* const pending{std::get_if<MoveEnd>(&this->interaction)}) {
        const auto removed{
            std::erase_if(this->placed_objects, [pending](const auto& placed) {
                return placed.object.id == pending->id;
            })
        };
        core::assert_eq(removed, size_t{1});
        this->selected_id.reset();
    }
    this->interaction = mode;
    switch (mode) {
        case Mode::Select:
            this->interaction_status = "Select | Left click an object";
            break;
        case Mode::Insert:
            this->interaction_status = "Place sphere | Click a surface";
            break;
        case Mode::Move:
            this->interaction_status = "Moving sphere | Click the start point";
            break;
        default: core::panic("Invalid interaction mode");
    }
}

auto Scene::click(const Hit& hit) -> void {
    // Hits must come from this scene, even after its object vectors grow.
    static_cast<void>(this->find_object(hit.id));
    const auto* const mode{std::get_if<Mode>(&this->interaction)};
    if (mode != nullptr && *mode == Mode::Select) {
        const auto moving{std::ranges::find_if(
            this->placed_objects, [&hit](const auto& placed) {
                return placed.object.id == hit.id
                    && std::holds_alternative<Motion>(placed.motion);
            }
        )};
        if (moving != this->placed_objects.end()) {
            this->placed_objects.erase(moving);
            this->selected_id.reset();
            this->interaction_status
                = "Removed moving sphere " + boost::uuids::to_string(hit.id);
        } else {
            this->selected_id = hit.id;
            this->interaction_status
                = "Selected " + boost::uuids::to_string(hit.id);
        }
        return;
    }
    auto position{hit.position};
    const auto radius{std::get<SphereInsertion>(this->insertion).radius};
    // Lift each endpoint by the radius so spheres rest on horizontal surfaces.
    position.y += radius;
    if (mode == nullptr) {
        const auto pending{std::get<MoveEnd>(this->interaction)};
        // Reject near-coincident endpoints rather than creating a sphere whose
        // movement is too small to see.
        if (hit.id == pending.id
            || std::hypot(
                   position.x - pending.start.x,
                   position.y - pending.start.y,
                   position.z - pending.start.z
               ) < 0.001f) {
            this->interaction_status
                = "Choose an end point away from the start sphere";
            return;
        }
        const auto placed{std::ranges::find_if(
            this->placed_objects, [&pending](const auto& candidate) {
                return candidate.object.id == pending.id;
            }
        )};
        core::assert_that(placed != this->placed_objects.end());
        core::assert_that(std::holds_alternative<Stationary>(placed->motion));
        placed->motion = Motion{pending.start, position};
        this->interaction = Mode::Move;
        this->interaction_status = "Moving "
                                 + boost::uuids::to_string(pending.id)
                                 + " | P then click to remove";
        return;
    }
    const auto id{this->place_sphere(position)};
    if (*mode == Mode::Insert) {
        this->interaction_status = "Placed " + boost::uuids::to_string(id);
    } else {
        core::assert_that(*mode == Mode::Move);
        this->interaction = MoveEnd{id, position};
        this->interaction_status
            = "Moving sphere | Click a different end point | P cancels";
    }
}

auto Scene::miss() -> void {
    if (const auto* const mode{std::get_if<Mode>(&this->interaction)};
        mode != nullptr && *mode == Mode::Select) {
        this->selected_id.reset();
        this->interaction_status = "Select | No object here";
    } else {
        this->interaction_status = "Click a scene surface";
    }
}

#pragma once

#include "camera.hpp"
#include "object/drawable_object.hpp"
#include "object/skybox.hpp"
#include "point_light.hpp"
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <utility>
#include <variant>
#include <vector>

class Scene final {
  public:
    using ShaderPrograms = std::map<
        std::pair<std::filesystem::path, std::filesystem::path>,
        object::shader::ShaderProgram>;

    struct NoSkybox final {};

    using Background = std::variant<NoSkybox, object::Skybox>;

    struct Hit final {
        core::types::ID id;
        math::Vector<float> position;
    };

    struct NoInsertion final {};

    struct SphereInsertion final {
        float radius;
    };

    using Insertion = std::variant<NoInsertion, SphereInsertion>;
    enum class Mode { Select, Insert, Move };

    Camera camera;
    PointLight light;
    std::string interaction_status{"Select | Left click an object"};

    Scene(
        Camera camera,
        const PointLight& light,
        ShaderPrograms&& shader_programs,
        std::vector<object::DrawableObject>&& objects,
        Background&& background,
        const Insertion& insertion,
        std::function<void(std::span<object::DrawableObject>, PointLight&)>
            update_handler
    );

    Scene(const Scene&) = delete;
    Scene(Scene&&) = default;

    ~Scene() = default;

    auto operator=(const Scene&) -> Scene& = delete;
    auto operator=(Scene&&) -> Scene& = default;

    auto update(float delta_seconds) -> void;
    auto draw() -> void;

    [[nodiscard]] auto
    pick(int32_t x, int32_t y, int32_t width, int32_t height) const
        -> std::optional<Hit>;

    auto set_mode(Mode mode) -> void;
    auto click(const Hit& hit) -> void;
    auto miss() -> void;

  private:
    struct Stationary final {};

    struct Motion final {
        math::Vector<float> start;
        math::Vector<float> end;
        double elapsed_seconds{0.0};
    };

    struct PlacedObject final {
        object::DrawableObject object;
        std::variant<Stationary, Motion> motion{Stationary{}};
    };

    ShaderPrograms shader_programs;
    std::vector<object::DrawableObject> objects;
    // Scene setup animations retain their original object list and indices.
    std::vector<PlacedObject> placed_objects;
    Background background;
    Insertion insertion;
    std::optional<core::types::ID> selected_id;

    struct MoveEnd final {
        core::types::ID id;
        math::Vector<float> start;
    };

    // MoveEnd keeps the first endpoint mandatory while awaiting the second.
    std::variant<Mode, MoveEnd> interaction{Mode::Select};

    std::unique_ptr<
        std::function<void(std::span<object::DrawableObject>, PointLight&)>>
        update_handler;

    [[nodiscard]] auto object_at(size_t index) const
        -> const object::DrawableObject&;

    [[nodiscard]] auto find_object(core::types::ID id) const
        -> const object::DrawableObject&;
    [[nodiscard]] auto place_sphere(math::Vector<float> position)
        -> core::types::ID;
};

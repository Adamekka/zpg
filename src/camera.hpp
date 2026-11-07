#pragma once

#include "math/matrix.hpp"
#include <algorithm>
#include <concepts>
#include <functional>
#include <vector>

template<typename T>
concept CameraObserver = requires(
    T& observer,
    const math::Matrix<float, 4, 4>& view_matrix,
    const math::Matrix<float, 4, 4>& projection_matrix
) {
    {
        observer.on_camera_changed(view_matrix, projection_matrix)
    } -> std::same_as<void>;
};

class Camera final {
  public:
    constexpr Camera(
        const math::Vector<float> position,
        const float yaw,
        const float pitch,
        const math::Matrix<float, 4, 4>& projection_matrix
    )
        : projection_matrix{projection_matrix}
        , position{position}
        , yaw{yaw}
        , pitch{pitch} {
        core::assert_that(
            std::isfinite(position.x) && std::isfinite(position.y)
            && std::isfinite(position.z) && std::isfinite(yaw)
            && std::isfinite(pitch) && std::abs(pitch) <= PITCH_LIMIT
        );
    }

    Camera(const Camera&) = delete;
    Camera(Camera&&) = default;

    ~Camera() = default;

    auto operator=(const Camera&) -> Camera& = delete;
    auto operator=(Camera&&) -> Camera& = default;

    [[nodiscard]] auto clone() const -> Camera {
        // A new scene shares the starting pose, not another scene's observers.
        return Camera{
            this->position, this->yaw, this->pitch, this->projection_matrix
        };
    }

    template<CameraObserver Observer>
    auto add_observer(Observer& observer) -> void {
        this->observers.emplace_back(
            [&observer](
                const math::Matrix<float, 4, 4>& view_matrix,
                const math::Matrix<float, 4, 4>& projection_matrix
            ) -> void {
                observer.on_camera_changed(view_matrix, projection_matrix);
            }
        );
        observer.on_camera_changed(
            this->get_view_matrix(), this->projection_matrix
        );
    }

    [[nodiscard]] auto get_projection_matrix() const
        -> const math::Matrix<float, 4, 4>& {
        return this->projection_matrix;
    }

    [[nodiscard]] auto get_view_matrix() const -> math::Matrix<float, 4, 4> {
        const auto direction{this->get_direction()};
        return math::Matrix<float, 4, 4>::look_at(
            this->position,
            {this->position.x + direction.x,
             this->position.y + direction.y,
             this->position.z + direction.z},
            {0.0f, 1.0f, 0.0f}
        );
    }

    auto move(const float forward, const float right) -> void {
        core::assert_that(std::isfinite(forward) && std::isfinite(right));
        if (forward == 0.0f && right == 0.0f) {
            return;
        }
        const auto direction{this->get_direction()};
        const auto side{
            direction.cross(math::Vector{0.0f, 1.0f, 0.0f}).normalized()
        };
        this->position.x += (direction.x * forward) + (side.x * right);
        this->position.y += (direction.y * forward) + (side.y * right);
        this->position.z += (direction.z * forward) + (side.z * right);
        this->notify_observers();
    }

    auto rotate(const float yaw_delta, const float pitch_delta) -> void {
        core::assert_that(
            std::isfinite(yaw_delta) && std::isfinite(pitch_delta)
        );
        const auto yaw{std::remainder(
            this->yaw + yaw_delta, 2.0f * std::numbers::pi_v<float>
        )};
        const auto pitch{
            std::clamp(this->pitch + pitch_delta, -PITCH_LIMIT, PITCH_LIMIT)
        };
        if (yaw == this->yaw && pitch == this->pitch) {
            return;
        }
        this->yaw = yaw;
        this->pitch = pitch;
        this->notify_observers();
    }

    auto set_aspect_ratio(const float aspect_ratio) -> void {
        core::assert_that(std::isfinite(aspect_ratio) && aspect_ratio > 0);
        // Only horizontal scaling changes, preserving the FOV and clipping
        // planes.
        const auto horizontal_scale{
            this->projection_matrix.get<1, 1>() / aspect_ratio
        };
        if (horizontal_scale == this->projection_matrix.get<0, 0>()) {
            return;
        }
        this->projection_matrix.get<0, 0>() = horizontal_scale;
        this->notify_observers();
    }

  private:
    // Keep the viewing direction away from world up so look_at stays valid.
    static constexpr auto PITCH_LIMIT{
        (std::numbers::pi_v<float> / 2.0f) - 0.01f
    };

    math::Matrix<float, 4, 4> projection_matrix;
    math::Vector<float> position;
    float yaw;
    float pitch;
    // Observers belong to the scene's shader map; its node addresses survive
    // scene moves, so these borrowed references stay valid.
    std::vector<std::function<void(
        const math::Matrix<float, 4, 4>&, const math::Matrix<float, 4, 4>&
    )>>
        observers;

    auto notify_observers() -> void {
        const auto view_matrix{this->get_view_matrix()};
        for (const auto& observer : this->observers) {
            observer(view_matrix, this->projection_matrix);
        }
    }

    [[nodiscard]] auto get_direction() const -> math::Vector<float> {
        return {
            std::cos(this->yaw) * std::cos(this->pitch),
            std::sin(this->pitch),
            std::sin(this->yaw) * std::cos(this->pitch)
        };
    }
};

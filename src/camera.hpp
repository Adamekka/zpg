#pragma once

#include "math/matrix.hpp"
#include <algorithm>

class Camera final {
  public:
    math::Matrix<float, 4, 4> projection_matrix;

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
        const auto direction{this->get_direction()};
        const auto side{
            direction.cross(math::Vector{0.0f, 1.0f, 0.0f}).normalized()
        };
        this->position.x += (direction.x * forward) + (side.x * right);
        this->position.y += (direction.y * forward) + (side.y * right);
        this->position.z += (direction.z * forward) + (side.z * right);
    }

    auto rotate(const float yaw_delta, const float pitch_delta) -> void {
        core::assert_that(
            std::isfinite(yaw_delta) && std::isfinite(pitch_delta)
        );
        this->yaw = std::remainder(
            this->yaw + yaw_delta, 2.0f * std::numbers::pi_v<float>
        );
        this->pitch
            = std::clamp(this->pitch + pitch_delta, -PITCH_LIMIT, PITCH_LIMIT);
    }

    auto set_aspect_ratio(const float aspect_ratio) -> void {
        core::assert_that(std::isfinite(aspect_ratio) && aspect_ratio > 0);
        // Only horizontal scaling changes, preserving the FOV and clipping
        // planes.
        this->projection_matrix.get<0, 0>()
            = this->projection_matrix.get<1, 1>() / aspect_ratio;
    }

  private:
    // Keep the viewing direction away from world up so look_at stays valid.
    static constexpr auto PITCH_LIMIT{
        (std::numbers::pi_v<float> / 2.0f) - 0.01f
    };

    math::Vector<float> position;
    float yaw;
    float pitch;

    [[nodiscard]] auto get_direction() const -> math::Vector<float> {
        return {
            std::cos(this->yaw) * std::cos(this->pitch),
            std::sin(this->pitch),
            std::sin(this->yaw) * std::cos(this->pitch)
        };
    }
};

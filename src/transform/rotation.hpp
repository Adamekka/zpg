#pragma once

#include "component.hpp"

namespace transform {

class Rotation final {
  public:
    constexpr explicit Rotation(const math::Vector<float> rotation)
        : rotation{rotation} {
        detail::assert_finite(rotation);
    }

    [[nodiscard]] constexpr auto get_matrix() const
        -> math::Matrix<float, 4, 4> {
        auto matrix{math::Matrix<float, 4, 4>::identity()};
        // Column vectors apply the rightmost rotation first: X, then Y, then Z.
        matrix.rotate_z(this->rotation.z);
        matrix.rotate_y(this->rotation.y);
        matrix.rotate_x(this->rotation.x);
        return matrix;
    }

  private:
    const math::Vector<float> rotation;
};

} // namespace transform

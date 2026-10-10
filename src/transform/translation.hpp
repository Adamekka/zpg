#pragma once

#include "component.hpp"

namespace transform {

class Translation final {
  public:
    constexpr explicit Translation(const math::Vector<float> position)
        : position{position} {
        detail::assert_finite(position);
    }

    [[nodiscard]] constexpr auto get_matrix() const
        -> math::Matrix<float, 4, 4> {
        auto matrix{math::Matrix<float, 4, 4>::identity()};
        matrix.translate(
            {this->position.x, this->position.y, this->position.z}
        );
        return matrix;
    }

  private:
    const math::Vector<float> position;
};

} // namespace transform

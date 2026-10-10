#pragma once

#include "component.hpp"

namespace transform {

class Scale final {
  public:
    constexpr explicit Scale(const math::Vector<float> scale)
        : scale{scale} {
        detail::assert_finite(scale);
    }

    [[nodiscard]] constexpr auto get_matrix() const
        -> math::Matrix<float, 4, 4> {
        auto matrix{math::Matrix<float, 4, 4>::identity()};
        matrix.scale({this->scale.x, this->scale.y, this->scale.z});
        return matrix;
    }

  private:
    const math::Vector<float> scale;
};

} // namespace transform

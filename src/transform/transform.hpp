#pragma once

#include "composite_transform.hpp"

namespace transform {

class Transform final {
  public:
    Transform(
        const math::Vector<float> position,
        const math::Vector<float> rotation,
        const math::Vector<float> scale
    )
        : Transform{
              std::make_shared<CompositeTransform>(std::vector<TransformNode>{
                  Translation{position}, Rotation{rotation}, Scale{scale}
              })
          } {}

    explicit Transform(std::shared_ptr<const CompositeTransform> root)
        : root{std::move(root)} {
        core::assert_ne(this->root, nullptr);
    }

    Transform(const Transform&) = default;
    Transform(Transform&&) = default;

    ~Transform() = default;

    auto operator=(const Transform&) -> Transform& = default;
    auto operator=(Transform&&) -> Transform& = default;

    [[nodiscard]] auto get_matrix() const -> math::Matrix<float, 4, 4> {
        core::assert_ne(this->root, nullptr);
        return this->root->get_matrix();
    }

    // NOLINTNEXTLINE(fuchsia-overloaded-operator)
    [[nodiscard]] auto operator*(const Transform& other) const -> Transform {
        return Transform{std::make_shared<CompositeTransform>(
            std::vector<TransformNode>{this->root, other.root}
        )};
    }

  private:
    std::shared_ptr<const CompositeTransform> root;
};

} // namespace transform

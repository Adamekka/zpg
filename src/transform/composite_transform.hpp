#pragma once

#include "rotation.hpp"
#include "scale.hpp"
#include "translation.hpp"
#include <memory>
#include <utility>
#include <variant>
#include <vector>

namespace transform {

class CompositeTransform;

using TransformNode = std::variant<
    Translation,
    Rotation,
    Scale,
    std::shared_ptr<const CompositeTransform>>;

class CompositeTransform final {
  public:
    explicit CompositeTransform(std::vector<TransformNode> children)
        : children{std::move(children)} {
        core::assert_that(!this->children.empty());
        for (const auto& child : this->children) {
            if (const auto* const composite{
                    std::get_if<std::shared_ptr<const CompositeTransform>>(
                        &child
                    )
                };
                composite != nullptr) {
                core::assert_ne(*composite, nullptr);
            }
        }
    }

    CompositeTransform(const CompositeTransform&) = delete;
    CompositeTransform(CompositeTransform&&) = delete;

    ~CompositeTransform() = default;

    auto operator=(const CompositeTransform&) -> CompositeTransform& = delete;
    auto operator=(CompositeTransform&&) -> CompositeTransform& = delete;

    [[nodiscard]] auto get_matrix() const -> math::Matrix<float, 4, 4> {
        auto matrix{math::Matrix<float, 4, 4>::identity()};
        // Column vectors apply the last child first, preserving a * b order.
        for (const auto& child : this->children) {
            matrix *= std::visit(
                []<typename T>(const T& value) -> math::Matrix<float, 4, 4> {
                    if constexpr (
                        std::same_as<
                            T,
                            std::shared_ptr<const CompositeTransform>>
                    ) {
                        return value->get_matrix();
                    } else {
                        static_assert(Component<T>);
                        return value.get_matrix();
                    }
                },
                child
            );
        }
        for (const auto value : matrix.values) {
            core::assert_that(std::isfinite(value));
        }
        return matrix;
    }

  private:
    const std::vector<TransformNode> children;
};

static_assert(Component<CompositeTransform>);

} // namespace transform

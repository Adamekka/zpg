#include "model.hpp"
#include "../../core/assert.hpp"

namespace object::model {

auto Model::from_raw_data(
    const std::span<const float> data,
    const std::span<const VertexAttribute> attributes,
    const MeshDrawMode draw_mode
) -> Model {
    core::assert_that(!attributes.empty());

    auto max_attributes{int32_t{0}};
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_attributes);
    core::assert_that(max_attributes > 0);
    core::assert_that(attributes.size() <= static_cast<size_t>(max_attributes));

    auto floats_per_vertex{size_t{0}};
    for (const auto& attribute : attributes) {
        core::assert_that(
            attribute.component_count >= 1 && attribute.component_count <= 4
        );
        floats_per_vertex += static_cast<size_t>(attribute.component_count);
    }

    core::assert_that(
        floats_per_vertex
        <= static_cast<size_t>(std::numeric_limits<int32_t>::max())
               / sizeof(float)
    );
    core::assert_eq(data.size() % floats_per_vertex, size_t{0});
    core::assert_that(
        data.size() / floats_per_vertex
        <= static_cast<size_t>(std::numeric_limits<int32_t>::max())
    );

    return Model{data, attributes, floats_per_vertex, draw_mode};
}

Model::Model(
    const std::span<const float> data,
    const std::span<const VertexAttribute> attributes,
    const size_t floats_per_vertex,
    const MeshDrawMode draw_mode
)
    : vertex_count{static_cast<int32_t>(data.size() / floats_per_vertex)}
    , vbo{data}
    , vao{this->vbo,
          attributes,
          static_cast<int32_t>(floats_per_vertex * sizeof(float))}
    , draw_mode{draw_mode} {}

auto Model::draw() const -> void {
    this->vao.bind();

    glDrawArrays(this->draw_mode.get(), 0, this->vertex_count);
}

} // namespace object::model

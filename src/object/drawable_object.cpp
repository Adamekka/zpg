#include "drawable_object.hpp"
#include "importer/model_importer.hpp"
#include <algorithm>

namespace object {

DrawableObject::DrawableObject(
    model::Model mesh,
    shader::ShaderProgram& shader_program,
    transform::Transform transform
)
    : transform{std::move(transform)}
    , materials{material::Material{shader_program}} {
    this->submeshes.emplace_back(
        std::move(mesh), size_t{0}, math::Vector{0.0f, 0.0f, 0.0f}
    );
}

DrawableObject::DrawableObject(
    std::vector<material::texture::Texture> textures,
    std::vector<material::Material> materials,
    std::vector<Submesh> submeshes,
    transform::Transform transform
)
    : transform{std::move(transform)}
    , textures{std::move(textures)}
    , materials{std::move(materials)}
    , submeshes{std::move(submeshes)} {
    core::assert_that(!this->submeshes.empty());
    for (const auto& submesh : this->submeshes) {
        core::assert_that(submesh.material_index < this->materials.size());
    }
}

auto DrawableObject::load(
    const std::filesystem::path& path,
    shader::ShaderProgram& shader_program,
    transform::Transform transform
) -> std::expected<DrawableObject, std::string> {
    auto imported{importer::ModelImporter::import(path)};
    if (!imported) {
        return std::unexpected{std::move(imported.error())};
    }
    auto textures{std::vector<material::texture::Texture>{}};
    textures.reserve(imported->textures.size() + 1);
    for (const auto& texture : imported->textures) {
        auto loaded{std::visit(
            [](const auto& source)
                -> std::expected<material::texture::Texture, std::string> {
                return material::texture::Texture::load(source);
            },
            texture.source
        )};
        if (!loaded) {
            return std::unexpected{
                "Model '" + path.string() + "': " + loaded.error()
            };
        }
        textures.emplace_back(std::move(*loaded));
    }
    // glTF defines absent texture components as 1. A white texel expresses
    // that identity and keeps both sampler bindings complete on OpenGL drivers.
    textures.emplace_back(
        material::texture::Texture::solid_color({255, 255, 255, 255})
    );
    const auto texture_binding{
        [&textures](
            const importer::TextureIndex& index
        ) -> const material::texture::Texture& {
            if (const auto* const value{std::get_if<size_t>(&index)}) {
                core::assert_that(*value < textures.size());
                return textures[*value];
            }
            return textures.back();
        }
    };
    auto materials{std::vector<material::Material>{}};
    materials.reserve(imported->materials.size());
    for (const auto& material : imported->materials) {
        materials.emplace_back(
            shader_program,
            material.properties,
            texture_binding(material.color_texture),
            texture_binding(material.emissive_texture)
        );
    }
    constexpr auto ATTRIBUTES{std::array{
        model::VertexAttribute{3},
        model::VertexAttribute{4},
        model::VertexAttribute{3},
        model::VertexAttribute{2}
    }};
    auto submeshes{std::vector<Submesh>{}};
    submeshes.reserve(imported->submeshes.size());
    for (const auto& submesh : imported->submeshes) {
        submeshes.emplace_back(
            model::Model::from_raw_data(
                submesh.vertices,
                ATTRIBUTES,
                model::MeshDrawMode::Value::Triangles
            ),
            submesh.material_index,
            submesh.center
        );
    }
    return DrawableObject{
        std::move(textures),
        std::move(materials),
        std::move(submeshes),
        std::move(transform)
    };
}

auto DrawableObject::draw(
    const math::Matrix<float, 4, 4>& view_matrix,
    const math::Matrix<float, 4, 4>& projection_matrix
) const -> void {
    const auto model_matrix{this->transform.get_matrix()};
    const auto model_view{view_matrix * model_matrix};
    const auto draw{[&](const Submesh& submesh) -> void {
        this->materials[submesh.material_index].bind(
            model_matrix, view_matrix, projection_matrix
        );
        submesh.mesh.draw();
    }};
    auto transparent{std::vector<std::pair<float, const Submesh*>>{}};
    for (const auto& submesh : this->submeshes) {
        if (this->materials[submesh.material_index].is_blended()) {
            const auto depth{
                (model_view.get<0, 2>() * submesh.center.x)
                + (model_view.get<1, 2>() * submesh.center.y)
                + (model_view.get<2, 2>() * submesh.center.z)
                + model_view.get<3, 2>()
            };
            transparent.emplace_back(depth, &submesh);
        } else {
            draw(submesh);
        }
    }
    if (!transparent.empty()) {
        // Blend after opaque meshes, back to front, without hiding later glass.
        std::ranges::stable_sort(
            transparent, {}, &std::pair<float, const Submesh*>::first
        );
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);
        for (const auto& entry : transparent) {
            draw(*entry.second);
        }
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}

} // namespace object

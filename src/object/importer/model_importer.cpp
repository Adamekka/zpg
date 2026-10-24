#include "model_importer.hpp"
#include <array>
#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cmath>
#include <limits>
#include <map>
#include <string_view>
#include <utility>

namespace object::importer {

auto ModelImporter::import(const std::filesystem::path& path)
    -> std::expected<ImportedModel, std::string> {
    auto importer{Assimp::Importer{}};
    const auto failure{
        [&path](const std::string_view detail) -> std::unexpected<std::string> {
            return std::unexpected{
                "Failed to import model '" + path.string()
                + "': " + std::string{detail}
            };
        }
    };
    // Flip Assimp's bottom-left UVs to match stb_image's top-left image rows.
    const auto* const loaded{importer.ReadFile(
        path.c_str(),
        aiProcess_Triangulate | aiProcess_GenSmoothNormals
            | aiProcess_ValidateDataStructure | aiProcess_FlipUVs
    )};
    if (loaded == nullptr) {
        return failure(importer.GetErrorString());
    }
    if (loaded->HasAnimations()) {
        return failure(
            "only static models are supported; animations cannot be imported"
        );
    }
    for (auto index{size_t{0}}; index < loaded->mNumMeshes; ++index) {
        if (loaded->mMeshes[index]->HasBones()) {
            return failure(
                "only static models are supported; skinning cannot be imported"
            );
        }
    }
    // Bake node transforms into static meshes so children keep their placement.
    const auto* const scene{importer.ApplyPostProcessing(
        aiProcess_PreTransformVertices | aiProcess_GenBoundingBoxes
    )};
    if (scene == nullptr) {
        return failure(importer.GetErrorString());
    }
    if (scene->mRootNode == nullptr || scene->mNumMeshes == 0) {
        return failure("model has no root node or meshes");
    }

    auto result{ImportedModel{}};
    auto texture_indices{std::map<std::string, size_t>{}};
    const auto load_texture{
        [&](
            const aiMaterial& material, const aiTextureType type
        ) -> std::expected<TextureIndex, std::string> {
            if (material.GetTextureCount(type) == 0) {
                return material::NoTexture{};
            }
            auto texture_path{aiString{}};
            auto uv_channel{uint32_t{0}};
            if (material.GetTexture(
                    type, 0, &texture_path, nullptr, &uv_channel
                )
                != AI_SUCCESS) {
                return failure("cannot read material texture");
            }
            if (uv_channel != 0) {
                return failure(
                    "only texture coordinate channel 0 is supported"
                );
            }
            if (const auto found{texture_indices.find(texture_path.C_Str())};
                found != texture_indices.end()) {
                return found->second;
            }
            const auto index{result.textures.size()};
            if (const auto* const texture{
                    scene->GetEmbeddedTexture(texture_path.C_Str())
                }) {
                if (texture->mHeight != 0 || texture->mWidth == 0
                    || texture->pcData == nullptr) {
                    return failure(
                        "embedded texture must contain encoded PNG or JPEG data"
                    );
                }
                // Assimp stores encoded image bytes in its aiTexel buffer.
                const auto* const begin{
                    reinterpret_cast<const uint8_t*>(texture->pcData)
                };
                result.textures.emplace_back(
                    std::vector<uint8_t>{begin, begin + texture->mWidth}
                );
            } else {
                result.textures.emplace_back(
                    path.parent_path() / texture_path.C_Str()
                );
            }
            texture_indices.emplace(texture_path.C_Str(), index);
            return index;
        }
    };

    constexpr auto UNIMPORTED{std::numeric_limits<size_t>::max()};
    auto material_indices{
        std::vector<size_t>(scene->mNumMaterials, UNIMPORTED)
    };
    for (auto mesh_index{size_t{0}}; mesh_index < scene->mNumMeshes;
         ++mesh_index) {
        const auto* const mesh{scene->mMeshes[mesh_index]};
        if (mesh == nullptr || mesh->mVertices == nullptr
            || mesh->mNormals == nullptr || mesh->mNumFaces == 0
            || mesh->mMaterialIndex >= scene->mNumMaterials) {
            return failure("mesh has missing geometry or an invalid material");
        }
        auto& material_index{material_indices[mesh->mMaterialIndex]};
        if (material_index == UNIMPORTED) {
            const auto* const material{scene->mMaterials[mesh->mMaterialIndex]};
            if (material == nullptr) {
                return failure("material is missing");
            }
            auto color{aiColor4D{}};
            auto specular{aiColor3D{}};
            auto emissive{aiColor3D{}};
            auto shininess{float{}};
            auto alpha_mode{aiString{}};
            // Assimp exposes glTF specular/glossiness diffuse factors here,
            // including the cornea's alpha, rather than in BASE_COLOR.
            if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) != AI_SUCCESS
                || material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive)
                       != AI_SUCCESS
                || material->Get(AI_MATKEY_SHININESS, shininess) != AI_SUCCESS
                || material->Get(AI_MATKEY_GLTF_ALPHAMODE, alpha_mode)
                       != AI_SUCCESS) {
                return failure(
                    "material is missing supported glTF color, shininess, or "
                    "alpha properties"
                );
            }
            if (material->Get(AI_MATKEY_COLOR_SPECULAR, specular)
                != AI_SUCCESS) {
                auto metallic{float{}};
                if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic)
                        != AI_SUCCESS
                    || !std::isfinite(metallic) || metallic < 0.0f
                    || metallic > 1.0f) {
                    return failure(
                        "material has neither a specular color nor valid "
                        "metalness"
                    );
                }
                // Map glTF metallic/roughness F0 to a Phong specular color.
                // glTF fixes nonmetal reflectance at 0.04 for its default IOR.
                specular = (aiColor3D{0.04f, 0.04f, 0.04f} * (1.0f - metallic))
                         + (aiColor3D{color.r, color.g, color.b} * metallic);
            }
            if (color.r > 1.0f || color.g > 1.0f || color.b > 1.0f
                || color.a > 1.0f) {
                return failure("material color is outside [0, 1]");
            }
            for (const auto value : std::array{
                     color.r,
                     color.g,
                     color.b,
                     color.a,
                     specular.r,
                     specular.g,
                     specular.b,
                     emissive.r,
                     emissive.g,
                     emissive.b,
                     shininess
                 }) {
                if (!std::isfinite(value) || value < 0.0f) {
                    return failure(
                        "material contains a nonfinite or negative value"
                    );
                }
            }
            const auto mode{
                [&]() -> std::expected<material::AlphaMode, std::string> {
                    if (alpha_mode == aiString{"OPAQUE"}) {
                        return material::AlphaMode::Opaque;
                    }
                    if (alpha_mode == aiString{"BLEND"}) {
                        return material::AlphaMode::Blend;
                    }
                    return failure(
                        "unsupported alpha mode '"
                        + std::string{alpha_mode.C_Str()} + "'"
                    );
                }()
            };
            if (!mode) {
                return std::unexpected{mode.error()};
            }
            // Assimp uses DIFFUSE for specular/glossiness and BASE_COLOR for
            // metallic/roughness glTF materials.
            const auto color_type{
                material->GetTextureCount(aiTextureType_BASE_COLOR) > 0
                    ? aiTextureType_BASE_COLOR
                    : aiTextureType_DIFFUSE
            };
            auto color_texture{load_texture(*material, color_type)};
            if (!color_texture) {
                return std::unexpected{std::move(color_texture.error())};
            }
            auto emissive_texture{
                load_texture(*material, aiTextureType_EMISSIVE)
            };
            if (!emissive_texture) {
                return std::unexpected{std::move(emissive_texture.error())};
            }
            material_index = result.materials.size();
            result.materials.emplace_back(
                material::SurfaceProperties{
                    {color.r, color.g, color.b, color.a},
                    {specular.r, specular.g, specular.b},
                    {emissive.r, emissive.g, emissive.b},
                    shininess,
                    *mode
                },
                *color_texture,
                *emissive_texture
            );
        }
        const auto& material{result.materials[material_index]};
        if (mesh->mTextureCoords[0] == nullptr
            && (std::holds_alternative<size_t>(material.color_texture)
                || std::holds_alternative<size_t>(material.emissive_texture))) {
            return failure("textured mesh has no UV coordinates");
        }

        auto vertices{std::vector<float>{}};
        vertices.reserve(static_cast<size_t>(mesh->mNumFaces) * 3 * 12);
        for (auto face_index{size_t{0}}; face_index < mesh->mNumFaces;
             ++face_index) {
            const auto& face{mesh->mFaces[face_index]};
            if (face.mNumIndices != 3 || face.mIndices == nullptr) {
                return failure("mesh contains a non-triangle face");
            }
            for (auto corner{size_t{0}}; corner < 3; ++corner) {
                const auto index{face.mIndices[corner]};
                if (index >= mesh->mNumVertices) {
                    return failure("mesh contains an invalid vertex index");
                }
                const auto position{mesh->mVertices[index]};
                const auto normal{mesh->mNormals[index]};
                // glTF omits vertex colors when material color alone is used.
                const auto color{
                    mesh->mColors[0] != nullptr
                        ? mesh->mColors[0][index]
                        : aiColor4D{1.0f, 1.0f, 1.0f, 1.0f}
                };
                // UVs are unused for explicitly untextured materials.
                const auto uv{
                    mesh->mTextureCoords[0] != nullptr
                        ? mesh->mTextureCoords[0][index]
                        : aiVector3D{0.0f, 0.0f, 0.0f}
                };
                for (const auto value : std::array{
                         position.x,
                         position.y,
                         position.z,
                         color.r,
                         color.g,
                         color.b,
                         color.a,
                         normal.x,
                         normal.y,
                         normal.z,
                         uv.x,
                         uv.y
                     }) {
                    if (!std::isfinite(value)) {
                        return failure(
                            "mesh contains a nonfinite vertex attribute"
                        );
                    }
                    vertices.emplace_back(value);
                }
            }
        }
        const auto center{(mesh->mAABB.mMin + mesh->mAABB.mMax) * 0.5f};
        result.submeshes.emplace_back(
            std::move(vertices),
            material_index,
            math::Vector{center.x, center.y, center.z}
        );
    }
    return result;
}

} // namespace object::importer

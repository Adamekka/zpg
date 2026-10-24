# Model import

- Import static glTF models with Assimp, using the importer and material design
  ported from the sibling `goon` project. Bake the node hierarchy's transforms
  into the geometry. Reject animation and skinning explicitly.
- Keep the project's explicit runtime vertex layouts. Imported vertices carry
  position, RGBA color, normal, and UV attributes in that order.
- Import material colors, specular color, shininess, emissive color, and base
  color and emissive textures. Support external image files and encoded embedded
  images. A material without a texture uses its own color factor.
- Shade imported materials with the same Phong lighting model as the other
  lit objects. This is a Phong material renderer, not a glTF PBR renderer.
  Metallic/roughness materials map their normal-incidence reflectance to Phong
  specular color with `F0 = 0.04 * (1 - metallic) + baseColor * metallic`, as
  defined by the [glTF specification](https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html).
- Support opaque and alpha blend materials. Reject alpha-masked materials.
  Draw transparent submeshes after opaque ones, sorted by their centers from
  back to front. Intersecting transparent triangles are not sorted individually.
- Keep imported textures, materials, and submeshes owned by their drawable
  object. Finalize texture storage before materials capture references to it.
- Report import and texture decoding failures through `std::expected`. Required
  startup assets produce a diagnostic and a failed application exit when absent
  or unsupported.

# Marisa

Use the supplied Marisa model as the additional object in the solar system.
Animate its horizontal orbit around the Sun using the transformation API,
outside Earth's orbit, at 2.8 radians per second. Keep her body upright and her
face and broom pointing in the direction of flight. Keep the complete path
visible from the initial camera.

This work is based on
[Marisa for Touhou Multi Scroll Shooting 2](https://sketchfab.com/3d-models/marisa-for-touhou-multi-scroll-shooting-2-998e923e32bc4e29a8fa206b4e474945)
by [skydash](https://sketchfab.com/SriSkydash), licensed under
[CC BY 4.0](https://creativecommons.org/licenses/by/4.0/).
The original model, images, and license are preserved. Scene placement, scale,
lighting, and orbital motion are applied by this project.

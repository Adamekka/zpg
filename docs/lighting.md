# Lighting demonstrations

Exercise 7's incorrect normal, specular, and resize examples are preserved in
the captured screenshots. The application retains only corrected rendering;
do not keep switches or scenes that intentionally reproduce those errors.
Preserve the earlier scenes on keys 1 through 5 and start in the normal scene.
Number keys 1 through 9 select scenes directly. Show the selected scene's number
in the window title. Retain the flying controls described in
[Camera](camera.md).

| Scene | Content |
| --- | --- |
| 1 through 5 | Earlier exercises |
| 6 | Correct normals, inverse transpose |
| 7 | Correct back-side specular clipping |
| 8 | Correct viewport and projection updates |
| 9 | Night forest |

## Normal transformation

Use rotated spheres with unequal axis scales in scene 6 to demonstrate correct
normal transformation. The earlier incorrect results remain in the submission
screenshots; the shader always applies the inverse transpose.

The correct world-space normal is
`transpose(inverse(mat3(model_matrix))) * normal`. This keeps the normal
perpendicular to transformed surface tangents under nonuniform scale. Transform
it into view space using the rotational part of the view matrix. Compute light
and viewing directions in that same space, and normalize interpolated normals
again in the fragment shader. All lit scenes use the correct transformation.

## Back-side specular light

Scene 7 shows a sphere with shininess 1. Place the light behind the sphere,
opposite the camera. Evaluate the specular term only when `dot(N, L) > 0`, so
the dark hemisphere stays dark. Preserve the earlier screenshot of the incorrect
highlight alongside the corrected screenshot.

## Window resizing

Scene 8 shows a sphere. Resize the window, including into a narrow portrait
shape. Every scene always updates `glViewport` and the perspective aspect ratio
using the current framebuffer width and height. Skip drawing when the
framebuffer has no area, as during minimization. Preserve the earlier incorrect
resize screenshots for comparison with the corrected result.

## Night forest

Scene 9 is a separate night scene with ground, 80 trees, 20 bushes, and one
moving point light. Use low ambient illumination, subdued specular materials,
and Phong lighting for the ground, wood, leaves, and bushes. Retain
the tree model's vertex-ID split between wood and leaves.

The point light owns its position, RGB intensity, ambient color, and constant,
linear, and quadratic attenuation coefficients. Apply attenuation to both direct
diffuse and specular light, leaving the small ambient term separate:

```text
attenuation(d) = 1 / (constant + linear * d + quadratic * d²)
color = ambient + attenuation(d) * light_color * (diffuse * surface_color + specular)
```

The night scene uses coefficients `1`, `0.35`, and `0.44`. Its intensity decreases
continuously with distance. The firefly's unlit marker shares the light's animated
position and does not create another light. Material owns shininess and specular
strength. Validate these values in its constructor; a separate Phong properties
type is unnecessary.

Represent coloring as `Material::Coloring`, a `std::variant` of `ModelColors`
and `TreeColors`. `ModelColors` holds the RGB color; do not pass color separately
alongside the variant. During object creation, the factory uses `phong.vert` for
model colors and `tree_phong.vert` for tree colors. The tree shader reads the
original position/normal layout and assigns wood/leaf colors from `gl_VertexID`.
Keep the vertex shaders separate, with no tree-color uniform or mode switch.
Duplicating their normal and light-direction calculations is intentional.

## Submission

Keep the Czech completion summary in the root `cvi0014.md` required by the
submission format. Screenshots use the student's login followed by the task
number and subtask letter. Include source files, shaders, models with their
textures and license, summary, and screenshots; exclude libraries and build
outputs. Uploading to the course and posting the planet screenshot to Discord
remain separate submission actions.

# Object interaction

Exercise 9 adds mouse selection, screen-to-world conversion, object placement,
and two-point movement. Use the same Boost.UUID identity system as `../goon`.
Keep the interaction code simple enough for a school project.

## Required behavior

- Assign a unique ID to every scene model. Preserve it when objects move or
  their owning containers grow.
- Identify the frontmost model under a left mouse click using the stencil
  buffer. Objects behind other geometry must not be selected through it.
- Convert the clicked screen point and its depth to world coordinates using
  GLM's `unProject`. Insert a model at that location.
- Let users choose two different scene points and create a model that travels
  along the segment between them using a parameter in `[0, 1]`.
- Clicking a moving model in selection mode must cause an interaction. This
  project removes it.
- Use keyboard controls to switch between selection and placement. Provide
  selection and insertion in the forest and EB408 classroom.

## Controls

| Input | Action |
| --- | --- |
| `0` | Classroom |
| `2` | Day forest |
| `P` | Select objects, or cancel an unfinished path |
| `I` | Place a stationary sphere on a clicked surface |
| `M` | Choose a start point, then a different end point for a moving sphere |
| Left click in selection mode | Select a model, or remove a moving sphere |
| Left click on empty background | Clear selection, or keep waiting for a valid placement point |
| W/S/A/D and right mouse drag | Existing flying camera controls |
| Escape | Close the application |

Switching placement modes removes an unfinished path's start sphere. Scene
switching preserves each scene's objects, interaction state, and camera.
Other demonstration scenes allow selection and direct users to the forest or
classroom when they request placement.

The title shows the current action and the selected, inserted, or removed UUID.
A gold wire overlay marks the selected object. Click actions also print their
status and full UUID to the console.

## Identity and picking

Model identity uses `boost::uuids::uuid`, generated with
`boost::uuids::random_generator`. Stencil values are temporary numeric lookup
tokens, not shortened or hashed UUIDs.

Request an eight-bit stencil buffer at startup and fail clearly if it is
unavailable. Reserve stencil zero for the background. On a click, render
depth and stencil in batches of 255 models. Render every model in every batch,
including models with token zero, so occlusion works across batches. Read the
stencil token with `glReadPixels` and map it back to that model's UUID. This
allows more than 255 models without ID reuse or an insertion cap.

The pick pass disables multisampling so coverage and depth come from the same
pixel center. It writes depth for blended geometry and ignores fully transparent
texels. The skybox is background and cannot receive inserted models.

Convert GLFW's top-left window coordinates to bottom-left framebuffer pixels
using the actual window and framebuffer sizes, including Retina scaling.
Unproject the pixel center and its sampled depth using the active camera's
view matrix, projection matrix, and current viewport. Resize the viewport and
projection before picking. Redraw the visible frame after picking.

## Placement and movement

Place unlit orange spheres with radius `0.3` in the day forest and `0.1` in the
classroom so their color stays visible in both scenes. Add the radius to each
clicked point's world Y coordinate so spheres rest on horizontal surfaces.
Walls and other surfaces also accept placement;
the offset remains vertical.

The first path click creates the sphere at endpoint `A`. The second click sets
endpoint `B`. Reject a click on the start sphere or an endpoint less than
`0.001` world units away, and keep waiting for a different endpoint.

For elapsed time `s`, reduce it modulo six seconds and calculate:

```text
phase = s / 3
t = phase <= 1 ? phase : 2 - phase
position = A + t * (B - A)
```

Each leg takes three seconds. Movement uses frame elapsed time, keeps `t` in
`[0, 1]`, and reverses continuously at both endpoints. Moving spheres retain
their UUID and remain selectable at their current position. Inserted models
have their own scene-owned list so existing scene animation indices stay valid.
Each inserted model owns its stationary or moving state, so removing the model
also removes its movement state.

## Build and hand-in

Build the application with:

```sh
cmake -S . -B build
cmake --build build
```

Assignment evidence belongs in `screenshots/exercise9/`, with
screenshots named `cvi0014_1a.png`, `cvi0014_2a.png`, and so on. Keep the Czech
completion summary in the root `cvi0014.md`. Submit source files, shaders, and
models without bundled libraries. Uploading the hand-in and posting a
demonstration to Discord are separate external submission steps.

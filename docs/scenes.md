# Scene ownership

- Keep the entry point minimal. The application creates the scene content during startup.
- Each scene owns its objects and shader programs.
- The application owns a collection of scenes and selects the active scene.
- Scenes handle their own per-frame updates and rendering.
- Scene selection is exposed through an application API.
- Render scenes with 4x multisample antialiasing.

# Scene creation

- Create scenes through a factory, then add objects through methods named for
  their types. The factory handles model data, vertex layouts, and shaders.
- Keep object counts, placement loops, and transforms explicit in scene setup.
  Use fixed transforms for now.
- Share shader programs within each scene when their vertex and fragment paths
  match. Finished scenes retain ownership of their objects and shaders.

```cpp
auto scene{SceneFactory::new_scene(camera)};
scene.add_tree(transform::Transform{
    {3.0f, 0.0f, -6.0f}, {0.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}
});
auto completed_scene{std::move(scene).build()};
```

Pass a `transform::Transform` to control each object's position, rotation, and
scale. All scenes use flying camera controls. Object types with built-in
animations update automatically. Pass a per-frame update handler to `build` for
scene-specific animation; it runs alongside object updates.
The handler receives the scene's objects and its point light by reference.

# Lighting comparison scene

- Place four equally sized blue spheres symmetrically around a point light at
  the world origin, in the XZ plane.
- Start the camera above the spheres so their inward-facing surfaces and
  specular highlights are visible. Retain the standard flying controls.
- Use a different lighting model on each sphere: Ambient on the left, Lambert
  on the right, Phong at the top, and Blinn-Phong at the bottom in the initial
  view. Keep a black background and no visible light mesh.
- Share the vertex shader. Keep each lighting calculation inline in its
  fragment shader's `main` function.
- Update both the viewport and the projection aspect ratio when the framebuffer
  changes size, including narrow portrait windows, so spheres stay round.

# Solar system lighting

- Use constant ambient lighting on the Sun and Phong lighting on the Earth and
  Moon. The Sun is the point light at the world origin.
- Include [Marisa](model-import.md), flying around the Sun outside Earth's orbit.
- Place the supplied login lettering below the solar system.

# Classroom scene

- Load the supplied textured EB408 classroom and start the camera inside.
  Align the characters with its floor. Preserve the original
  Blender file and texture alongside the glTF conversion used by the scene.
- Fit the supplied textured door and frame into the classroom's existing
  doorway, with the leaf at the outer wall border and the frame filling the
  wall's depth. Mount the whiteboard against the wall on the right when facing
  the door from inside the classroom, retaining its original mounting height.
  Match both fixtures to the room's scale and wall angles. Preserve their
  original Blender files and textures alongside their glTF conversions.
- Surround the classroom with the supplied six-face cubemap skybox. The skybox
  follows camera rotation, ignores camera translation, and renders without
  lighting behind the scene objects.
- Require all six skybox faces to load at the same square dimensions. Report
  missing, undecodable, or incompatible faces as startup asset failures.
- Start on this scene. Press `0` to return to it; keys `1` through `9` select
  the existing scenes. Retain the standard flying camera controls.
- Place Cirno beside the whiteboard, facing rows of the sixteen other supplied
  Touhou characters. Keep them static and align their bases with the floor.
  Start the camera inside the room looking across the class toward Cirno.
- Place Marisa and her broom next to Youmu, using the same asset as the
  solar-system scene.
- Put the classroom point light just below the ceiling at the room's center.
- Show the supplied Czech Phong lighting lecture image on the whiteboard,
  including its ambient, diffuse, specular, and combined lighting equations.
  Preserve the image's proportions and the original blank texture alongside
  the classroom version. Omit the university footer from the displayed image.

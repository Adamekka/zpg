# Scene ownership

- Keep the entry point minimal. The application creates the scene content during startup.
- Each scene owns its objects and shader programs.
- The application owns a collection of scenes and selects the active scene.
- Scenes handle their own per-frame updates and rendering.
- Scene selection is exposed through an application API.

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

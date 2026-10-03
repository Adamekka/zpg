# Scene ownership

- Each scene owns its objects and shader programs.
- The application owns a collection of scenes and selects the active scene.
- Scenes handle their own per-frame updates and rendering.
- Scene selection is exposed through an application API.

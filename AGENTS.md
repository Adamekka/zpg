# Project purpose

- This is a school project. Keep code and architecture simple and easy to understand.

# Vertex data

- Objects can use different vertex layouts. Supply each object's float attributes explicitly at runtime.
- Attribute order determines sequential shader input locations, starting at zero.

# Transformations

- Use the Composite design pattern for transformations and their combinations.
- Use compile-time transformation contracts and a fixed set of runtime node types, without inheritance.
- Each object owns one composable transformation and supplies its model matrix when drawing.
- Construct transformations from position, X/Y/Z rotation angles in radians, and scale. Apply scaling, then X/Y/Z rotations, then translation.
- Animate objects by replacing their transformations with newly composed values.
- Scene setup and updates should create and combine transformations through the transformation API without constructing matrices directly.
- Do not expose transformation-order settings in the object transformation API.

# Scene ownership

- Keep the entry point minimal. The application creates the scene content during startup.
- Each scene owns its objects and shader programs.
- The application owns a collection of scenes and selects the active scene.
- Scenes handle their own per-frame updates and rendering.
- Scene selection is exposed through an application API.

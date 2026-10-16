# Clock

- Create clocks through the scene factory. The clock method handles the model,
  shader, and hand animation; scene setup supplies the object's transform.
- Update clock hands automatically each frame, including in scenes with a custom
  update handler.
- Clock hands display the current local time and move independently.
- Render the complete clock with one draw call. Use the model's original vertex
  colors to identify parts for transformations, separately from displayed colors.
- Use black hour and second hands, a red minute hand, and #05C3DE for the KI logo.

See [scenes](scenes.md) for scene setup and ownership requirements, and
[transformations](transformations.md) for object animation requirements.

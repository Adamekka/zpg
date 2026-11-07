# Camera

- The camera derives its view matrix from position, yaw, and pitch, and holds the
  projection matrix. Objects supply their own model matrices.
- Supply model, view, and projection matrices separately to shaders.
- The camera is the subject in the Observer pattern. Scene-owned shader
  programs observe it through a concept requiring
  `on_camera_changed(view_matrix, projection_matrix) -> void`, without an
  inheritance hierarchy. Registration immediately supplies the current matrices.
- Notify observers after movement, rotation, or an actual aspect-ratio change.
  Repeating the same aspect ratio or making a zero movement does not notify.
  Shader observers upload view and projection matrices; materials upload only
  the model matrix when drawing. Transparent mesh sorting still uses the view
  matrix on the CPU.
- Keep projection storage private so changes cannot bypass notifications.
  Read it through a const accessor and resize it through `set_aspect_ratio`.
- A factory explicitly clones the camera's starting pose and projection
  without observers. Moving a camera transfers its subscriptions. Register
  observers when the completed scene is constructed; keep them in the scene's
  shader map so their addresses survive scene moves. The skybox borrows its
  shader from the same map and removes view translation in its vertex shader.
  Observers must remain alive throughout camera notifications.
- Camera controls always use flying movement. W/S move along the full viewing
  direction, including vertically; A/D strafe. Normalize diagonal movement and
  scale movement by elapsed time.
- Hold the right mouse button to look around. Keep the cursor unlocked and do
  not recenter it. Start each drag from the current cursor position, and limit
  pitch to avoid a viewing direction parallel to world up.
- The active scene responds to these controls. Preserve each scene's camera
  when switching scenes and ignore movement while the window is unfocused.

See [transformations](transformations.md) for model matrix requirements.

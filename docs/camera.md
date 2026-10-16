# Camera

- The camera derives its view matrix from position, yaw, and pitch, and holds the
  projection matrix. Objects supply their own model matrices.
- Supply model, view, and projection matrices separately to shaders.
- Camera controls always use flying movement. W/S move along the full viewing
  direction, including vertically; A/D strafe. Normalize diagonal movement and
  scale movement by elapsed time.
- Hold the right mouse button to look around. Keep the cursor unlocked and do
  not recenter it. Start each drag from the current cursor position, and limit
  pitch to avoid a viewing direction parallel to world up.
- The active scene responds to these controls. Preserve each scene's camera
  when switching scenes and ignore movement while the window is unfocused.

See [transformations](transformations.md) for model matrix requirements.

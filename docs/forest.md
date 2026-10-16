# Forest

- Place a ground plane, bushes, and at least 74 trees in a level 3D scene. Vary
  each tree's position, rotation, and scale with explicit, fixed scene setup.
- Use a dedicated tree shader. In the supplied model, vertices 0 through 36443
  are trunk and branches; later vertices are leaves. Use `gl_VertexID` to color
  the wood brown and leaves green.
- Shade the tree colors using `mix(0.3, 1.0, abs(normal.y))`, equivalent to the
  assignment's `0.3 + 0.7 * abs(normal.y)`. The factory selects this shader and
  the position/normal vertex layout.
- Place clocks in at least two different forest locations. Use equal scale
  magnitudes on all axes so their models remain undeformed; reflecting Z to face
  the camera is allowed. Keep each complete clock to one draw call.

See [camera](camera.md) for flying controls and [clock](clock.md) for clock
animation and colors. The Observer pattern is optional; direct matrix uniforms
are sufficient.

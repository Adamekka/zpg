# Transformations

- Use the Composite design pattern for transformations and their combinations.
- Use compile-time transformation contracts and a fixed set of runtime node types, without inheritance.
- Each object owns one composable transformation and supplies its model matrix when drawing.
- Construct transformations from position, X/Y/Z rotation angles in radians, and scale. Apply scaling, then X/Y/Z rotations, then translation.
- Animate objects by replacing their transformations with newly composed values.
- Scene setup and updates should create and combine transformations through the transformation API without constructing matrices directly.
- Do not expose transformation-order settings in the object transformation API.

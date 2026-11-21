# Multiple lights

Exercise 10 lets a scene use several lights of different types at once.

## Light types

- **Point**: position, color, ambient color, and distance attenuation.
- **Directional**: direction, color, and ambient color. No position and no
  attenuation.
- **Spot**: point light limited to a cone around its direction. Inner and outer
  cone half-angles set where the light starts to fade and where it ends.

A scene uses up to 16 lights, including the flashlight. `phong.frag`
loops over all of them in view space and sums each light's ambient, diffuse,
and specular terms. Shaders written for one light keep using only the first
light's position.

## Spot light edge

The fragment's angle from the spot axis, `theta`, is compared as a cosine with
the inner and outer cutoffs. Two edge variants are available:

| Variant | Formula |
| --- | --- |
| Hard | `step(outer, theta)` |
| Smooth | `smoothstep(outer, inner, theta)` |

Distance attenuation applies on top of the edge factor.

## Flashlight

Every scene has a spot light flashlight that follows the camera's position and
viewing direction. It starts off. `F` toggles it and `G` switches between the
hard and smooth edge. Only objects shaded with `phong.frag` react to it.

## Night forest lights

Scene 9 contains:

- dim bluish moonlight as a directional light,
- a street lamp: a spot light pointing down, with a smooth edge,
- six fireflies as small point lights that fly in small loops around random
  centers, each starting at a random point of its loop.

The lamp and fireflies each have an unlit marker sphere.

#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec4 vertex_color;
out vec3 view_position;
out vec3 view_normal;
out vec3 view_light_direction;
out vec2 texture_uv;

void main() {
    mat4 model_view = view_matrix * model_matrix;
    vec4 position = model_view * vec4(pos, 1.0);
    vertex_color = color;
    texture_uv = uv;
    view_position = position.xyz;
    // The point light is at the world origin; interpolate its unnormalized
    // vector.
    view_light_direction
        = (view_matrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz - position.xyz;
    // Inverse transpose keeps normals perpendicular after nonuniform scaling.
    view_normal = normalize(transpose(inverse(mat3(model_view))) * normal);
    gl_Position = projection_matrix * position;
}

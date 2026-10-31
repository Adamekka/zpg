#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform vec3 light_position;

out vec4 vertex_color;
out vec3 view_position;
out vec3 view_normal;
out vec3 view_light_direction;

void main() {
    mat4 model_view = view_matrix * model_matrix;
    vec4 position = model_view * vec4(pos, 1.0);
    // The supplied tree stores trunk/branches first, followed by leaves.
    vertex_color = vec4(
        gl_VertexID > 36443 ? vec3(0.10, 0.65, 0.08) : vec3(0.38, 0.20, 0.08),
        1.0
    );
    view_position = position.xyz;
    // Interpolate the unnormalized direction so fragment distances stay valid.
    view_light_direction
        = (view_matrix * vec4(light_position, 1.0)).xyz - position.xyz;
    // Inverse transpose keeps normals perpendicular after nonuniform scaling.
    vec3 world_normal = transpose(inverse(mat3(model_matrix))) * normal;
    view_normal = normalize(mat3(view_matrix) * world_normal);
    gl_Position = projection_matrix * position;
}

#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec3 vertex_color;

void main() {
    vec3 color
        = gl_VertexID > 36443 ? vec3(0.10, 0.65, 0.08) : vec3(0.38, 0.20, 0.08);

    // Map range [0, 1] -> [0.3, 1]
    float shade = mix(0.3, 1.0, abs(normal.y));
    vertex_color = shade * color;
    gl_Position
        = projection_matrix * view_matrix * model_matrix * vec4(pos, 1.0);
}

#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

uniform vec3 offset;
uniform vec3 scale;
uniform float angle;

out vec3 vertex_color;

void main() {
    vertex_color = color;

    vec3 scaled_pos = pos * scale;
    vec3 rotated_pos = vec3(
        cos(angle) * scaled_pos.x - sin(angle) * scaled_pos.z,
        scaled_pos.y,
        sin(angle) * scaled_pos.x + cos(angle) * scaled_pos.z
    );
    vec3 transformed_pos = rotated_pos + offset;

    gl_Position = vec4(transformed_pos.xy, -transformed_pos.z, 1.0);
}

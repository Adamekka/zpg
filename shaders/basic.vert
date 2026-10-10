#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

uniform mat4 model_matrix;

out vec3 vertex_color;

void main() {
    vertex_color = color;

    gl_Position = model_matrix * vec4(pos, 1.0);
}

#version 410 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

uniform mat4 hour_matrix;
uniform mat4 minute_matrix;
uniform mat4 second_matrix;

out vec3 vertex_color;

void main() {
    vec4 position = vec4(pos, 1.0);
    vertex_color = color;

    if (color == vec3(0.0, 0.0, 1.0)) {
        position = hour_matrix * position;
        vertex_color = vec3(0.0);
    } else if (color == vec3(0.0, 1.0, 0.0)) {
        position = minute_matrix * position;
        vertex_color = vec3(0.0);
    } else if (color == vec3(1.0, 0.0, 0.0)) {
        position = second_matrix * position;
        vertex_color = vec3(1.0, 0.0, 0.0);
    } else if (color == vec3(1.0, 1.0, 0.0)) {
        // Logo
        vertex_color = vec3(5.0, 195.0, 222.0) / 255.0;
    }

    gl_Position = projection_matrix * view_matrix * model_matrix * position;
}

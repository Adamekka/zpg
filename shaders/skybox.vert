#version 410 core

layout(location = 0) in vec3 pos;

uniform mat4 view_matrix;
uniform mat4 projection_matrix;

out vec3 texture_direction;

void main() {
    texture_direction = pos;
    // Remove translation so flying moves within an infinitely distant sky.
    vec4 position
        = projection_matrix * mat4(mat3(view_matrix)) * vec4(pos, 1.0);
    // Setting clip-space Z to W places the sky at the far depth.
    gl_Position = position.xyww;
}

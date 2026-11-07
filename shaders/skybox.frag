#version 410 core

in vec3 texture_direction;

uniform samplerCube skybox_texture;

out vec4 frag_color;

void main() {
    frag_color = texture(skybox_texture, texture_direction);
}

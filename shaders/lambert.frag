#version 410 core

in vec4 vertex_color;
in vec3 view_normal;
in vec3 view_light_direction;

out vec4 frag_color;

void main() {
    vec3 light = normalize(view_light_direction);
    vec3 normal = normalize(view_normal);
    float diffuse = max(dot(normal, light), 0.0);
    frag_color = vec4(diffuse * vertex_color.rgb, 1.0);
}

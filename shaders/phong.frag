#version 410 core

in vec4 vertex_color;
in vec3 view_position;
in vec3 view_normal;
in vec3 view_light_direction;

out vec4 frag_color;

const float AMBIENT_STRENGTH = 0.1;
const float SPECULAR_STRENGTH = 0.5;
const float SHININESS = 32.0;

void main() {
    vec3 light = normalize(view_light_direction);
    vec3 view = normalize(-view_position);
    vec3 normal = normalize(view_normal);
    float diffuse = max(dot(normal, light), 0.0);
    float specular = 0.0;
    // Suppress specular highlights when the light is behind the surface.
    if (diffuse > 0.0) {
        specular = pow(max(dot(view, reflect(-light, normal)), 0.0), SHININESS);
    }
    frag_color = vec4(
        (AMBIENT_STRENGTH + diffuse) * vertex_color.rgb
            + SPECULAR_STRENGTH * specular,
        1.0
    );
}

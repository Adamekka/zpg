#version 410 core

in vec4 vertex_color;
in vec3 view_position;
in vec3 view_normal;
in vec3 view_light_direction;

uniform vec3 light_color;
uniform vec3 ambient_color;
uniform vec3 light_attenuation;
uniform float shininess;
uniform float specular_strength;

out vec4 frag_color;

void main() {
    vec3 light = normalize(view_light_direction);
    vec3 view = normalize(-view_position);
    vec3 normal = normalize(view_normal);
    float diffuse = max(dot(normal, light), 0.0);
    float specular = 0.0;
    // Suppress specular highlights when the light is behind the surface.
    if (diffuse > 0.0) {
        specular = pow(max(dot(view, reflect(-light, normal)), 0.0), shininess);
    }
    float distance_to_light = length(view_light_direction);
    float attenuation = 1.0 / dot(
        light_attenuation,
        vec3(1.0, distance_to_light, distance_to_light * distance_to_light)
    );
    frag_color = vec4(
        ambient_color + attenuation * light_color
            * (diffuse * vertex_color.rgb + specular_strength * specular),
        1.0
    );
}

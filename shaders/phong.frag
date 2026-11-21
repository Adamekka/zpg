#version 410 core

// Must match Light::MAX_COUNT.
const int MAX_LIGHTS = 16;

// Must match Light::Type.
const int POINT = 0;
const int DIRECTIONAL = 1;
const int SPOT = 2;

// Must match Light::Falloff::Hard; the other value is Smooth.
const int HARD = 0;

struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec3 color;
    vec3 ambient;
    vec3 attenuation;
    float inner_cutoff;
    float outer_cutoff;
    int falloff;
};

in vec4 vertex_color;
in vec3 view_position;
in vec3 view_normal;

uniform mat4 view_matrix;
uniform Light lights[MAX_LIGHTS];
uniform int light_count;
uniform float shininess;
uniform float specular_strength;

out vec4 frag_color;

void main() {
    vec3 view = normalize(-view_position);
    vec3 normal = normalize(view_normal);
    vec3 color = vec3(0.0);
    for (int i = 0; i < light_count; ++i) {
        color += lights[i].ambient;

        // Lights are given in world space; shade in view space.
        vec3 light_direction = mat3(view_matrix) * lights[i].direction;
        vec3 to_light = -light_direction;
        float intensity = 1.0;
        if (lights[i].type != DIRECTIONAL) {
            to_light = (view_matrix * vec4(lights[i].position, 1.0)).xyz
                - view_position;
            float d = length(to_light);
            intensity = 1.0 / dot(lights[i].attenuation, vec3(1.0, d, d * d));
        }
        vec3 light = normalize(to_light);

        if (lights[i].type == SPOT) {
            // Cosine of the angle between the spot axis and this fragment.
            float theta = dot(-light, normalize(light_direction));
            float inner = lights[i].inner_cutoff;
            float outer = lights[i].outer_cutoff;
            intensity *= lights[i].falloff == HARD
                ? step(outer, theta)
                : smoothstep(outer, inner, theta);
        }

        float diffuse = max(dot(normal, light), 0.0);
        float specular = 0.0;
        // Suppress specular highlights when the light is behind the surface.
        if (diffuse > 0.0) {
            specular
                = pow(max(dot(view, reflect(-light, normal)), 0.0), shininess);
        }
        color += intensity * lights[i].color
            * (diffuse * vertex_color.rgb + specular_strength * specular);
    }
    frag_color = vec4(color, 1.0);
}

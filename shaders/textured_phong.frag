#version 410 core

in vec4 vertex_color;
in vec2 texture_uv;
in vec3 view_position;
in vec3 view_normal;
in vec3 view_light_direction;

uniform vec4 base_color_factor;
uniform vec3 specular_color;
uniform vec3 emissive_color;
uniform float shininess;
uniform sampler2D color_texture;
uniform sampler2D emissive_texture;
uniform int alpha_mode;

out vec4 frag_color;

const float AMBIENT_STRENGTH = 0.1;
const int ALPHA_BLEND = 2;

void main() {
    vec4 color
        = base_color_factor * vertex_color * texture(color_texture, texture_uv);
    vec3 light = normalize(view_light_direction);
    vec3 view = normalize(-view_position);
    // Reverse normals on the back faces of two-sided surfaces.
    vec3 normal = normalize(view_normal) * (gl_FrontFacing ? 1.0 : -1.0);
    float diffuse = max(dot(normal, light), 0.0);
    float specular = 0.0;
    // Suppress specular highlights when the light is behind the surface.
    if (diffuse > 0.0) {
        float cosine = dot(view, reflect(-light, normal));
        // Imported materials can have zero shininess; avoid pow(0, 0).
        specular = cosine > 0.0 ? pow(cosine, shininess) : 0.0;
    }
    vec3 emission = emissive_color * texture(emissive_texture, texture_uv).rgb;
    frag_color = vec4(
        (AMBIENT_STRENGTH + diffuse) * color.rgb + specular_color * specular
            + emission,
        alpha_mode == ALPHA_BLEND ? color.a : 1.0
    );
}

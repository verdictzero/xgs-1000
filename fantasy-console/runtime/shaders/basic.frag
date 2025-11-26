#version 100
precision mediump float;

varying vec3 v_normal;
varying vec2 v_texcoord;
varying vec3 v_world_pos;

uniform sampler2D u_texture;
uniform vec3 u_light_pos;
uniform vec3 u_light_color;
uniform vec3 u_ambient;

void main() {
    // Simple directional lighting (Gouraud-style)
    vec3 normal = normalize(v_normal);
    vec3 light_dir = normalize(u_light_pos - v_world_pos);
    float diff = max(dot(normal, light_dir), 0.0);

    vec3 lighting = u_ambient + u_light_color * diff;

    // Point-sampled texture (retro pixelated look)
    vec4 tex_color = texture2D(u_texture, v_texcoord);

    gl_FragColor = vec4(lighting * tex_color.rgb, tex_color.a);

    // Alpha test for sprite cutout
    if (gl_FragColor.a < 0.5) {
        discard;
    }
}
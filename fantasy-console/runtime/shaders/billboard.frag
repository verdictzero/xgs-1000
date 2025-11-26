#version 100
precision mediump float;

varying vec2 v_texcoord;

uniform sampler2D u_texture;
uniform vec4 u_color;
uniform float u_flip_horizontal;

void main() {
    vec2 texcoord = v_texcoord;
    
    // Handle horizontal flipping for sprite mirroring
    if (u_flip_horizontal > 0.5) {
        texcoord.x = 1.0 - texcoord.x;
    }
    
    vec4 tex_color = texture2D(u_texture, texcoord);
    gl_FragColor = tex_color * u_color;

    // Alpha test
    if (gl_FragColor.a < 0.1) {
        discard;
    }
}
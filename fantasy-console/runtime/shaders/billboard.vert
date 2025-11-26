#version 100
precision mediump float;

attribute vec3 a_position;
attribute vec2 a_texcoord;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec3 u_billboard_pos;
uniform vec2 u_billboard_size;

varying vec2 v_texcoord;

void main() {
    // Extract right and up vectors from view matrix
    vec3 right = vec3(u_view[0][0], u_view[1][0], u_view[2][0]);
    vec3 up = vec3(u_view[0][1], u_view[1][1], u_view[2][1]);

    // Calculate billboard world position
    vec3 billboard_pos = u_billboard_pos +
        right * a_position.x * u_billboard_size.x +
        up * a_position.y * u_billboard_size.y;

    v_texcoord = a_texcoord;
    gl_Position = u_projection * u_view * vec4(billboard_pos, 1.0);
}
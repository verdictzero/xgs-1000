#version 100
precision mediump float;

attribute vec3 a_position;
attribute vec3 a_normal;
attribute vec2 a_texcoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

varying vec3 v_normal;
varying vec2 v_texcoord;
varying vec3 v_world_pos;

void main() {
    vec4 world_pos = u_model * vec4(a_position, 1.0);
    v_world_pos = world_pos.xyz;
    v_normal = mat3(u_model) * a_normal;
    v_texcoord = a_texcoord;
    gl_Position = u_projection * u_view * world_pos;
}
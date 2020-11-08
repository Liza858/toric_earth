#version 330 core


in vec3 out_normal;
in vec3 out_position;
in vec2 out_tex_coords;

uniform sampler2D obj_texture;
uniform samplerCube cubemap_texture;


void main() {
    gl_FragColor =  vec4(texture(obj_texture, out_tex_coords).rgb, 1.0);
}
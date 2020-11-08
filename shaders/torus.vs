#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 norm;
out vec3 texture_coords;
out float dist;

void main() {

    norm = normal;
    texture_coords = tex_coords;
    gl_Position = projection * view * model * vec4(position, 1.0);
    dist = gl_Position.w;
    
}
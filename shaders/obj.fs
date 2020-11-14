#version 330 core


in vec3 out_normal;
in vec3 out_position;
in vec2 out_tex_coords;

uniform sampler2D near_shadow_map;
uniform samplerCube cubemap_texture;
uniform sampler2D obj_texture;

uniform mat4 mvp_near;
vec3 global_light_direction = vec3(0, 0, 1);
float global_light_coef = 0.2;

void main() {
    vec4 pos = mvp_near * vec4(out_position, 1);
    vec3 point = pos.xyz / pos.w * 0.5 + 0.5;
    float depth = texture(near_shadow_map, point.xy).r;

    float light = max(dot(out_normal, normalize(global_light_direction)), 0);

    vec4 color = vec4(texture(obj_texture, out_tex_coords).rgb, 1.0);

    color = depth < point.z - 0.001 ? color * global_light_coef : color * global_light_coef + color * (1 - global_light_coef) * light ;

    gl_FragColor =  color;
}
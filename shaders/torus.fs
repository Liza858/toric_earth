#version 330 core


in vec3 norm;
in vec3 texture_coords;
in float dist;
in vec3 pos;

uniform int enable;

uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

uniform sampler2D detail_tex1;
uniform sampler2D detail_tex2;
uniform sampler2D detail_tex3;
uniform sampler2D near_shadow_map;
uniform sampler2D far_shadow_map;
uniform sampler2D object_shadow_map;


uniform float detail_coef;
uniform int detail_repeat_count;
uniform float detail_dist;
uniform int tex1_repeat_count;
uniform int tex2_repeat_count;
uniform int tex3_repeat_count;
uniform mat4 mvp_near;
uniform mat4 mvp_far;
uniform mat4 mvp_object;

vec3 global_light_direction = vec3(0, 0, 1);
float global_light_coef = 0.15;


void main() {
    vec4 position = mvp_near * vec4(pos, 1);
    vec3 point1 = position.xyz / position.w * 0.5 + 0.5;
    position = mvp_far * vec4(pos, 1);
    vec3 point2 = position.xyz / position.w * 0.5 + 0.5;
    position = mvp_object * vec4(pos, 1);
    vec3 point3 = position.xyz / position.w * 0.5 + 0.5;
    float depth1 = texture(near_shadow_map, point1.xy).r;
    float depth2 = texture(far_shadow_map, point2.xy).r;
    float depth3 = texture(object_shadow_map, point3.xy).r;

    int coef = 2;

    float x1 = texture_coords.x * tex1_repeat_count - floor(texture_coords.x * tex1_repeat_count);
    float y1 = texture_coords.y * tex1_repeat_count * coef - floor(texture_coords.y * tex1_repeat_count * coef);
    float x2 = texture_coords.x * tex2_repeat_count - floor(texture_coords.x * tex2_repeat_count);
    float y2 = texture_coords.y * tex2_repeat_count * coef - floor(texture_coords.y * tex2_repeat_count * coef);
    float x3 = texture_coords.x * tex3_repeat_count - floor(texture_coords.x * tex3_repeat_count);
    float y3 = texture_coords.y * tex3_repeat_count * coef - floor(texture_coords.y * tex3_repeat_count * coef);


    vec4 tex_color1 = texture(tex1, vec2(x1, y1));
    vec4 tex_color2 = texture(tex2, vec2(x2, y2));
    vec4 tex_color3 = texture(tex3, vec2(x3, y3));

    float xx = texture_coords.x * detail_repeat_count - floor(texture_coords.x * detail_repeat_count);
    float yy = texture_coords.y * detail_repeat_count * coef - floor(texture_coords.y * detail_repeat_count * coef);

    vec4 detail_color1 = texture(detail_tex1, vec2(xx, yy));
    vec4 detail_color2 = texture(detail_tex1, vec2(xx, yy));
    vec4 detail_color3 = texture(detail_tex1, vec2(xx, yy));

    vec4 color;

    if (texture_coords.z < 2.0 / 3.0 * 2.2) {
        color = tex_color1;
    } else if (texture_coords.z < 2 / 3.0 * 2.5) {
        color = tex_color3;
    } else {
        color = tex_color3;
    }

    float light = max(dot(norm, normalize(global_light_direction)), 0);



    if (dist < detail_dist) {
        color *= detail_coef * detail_color2.x;
    }

    float c = 0;
    if (depth3 < (point3.z - 0.001) && 
         point3.x > 0 && point3.x < 1 && point3.y > 0 && point3.y < 1 && point3.z > 0 && point3.z < 1) {
       c = max(0, 0.3 * point3.z);
    }
 
    if (enable == 1) {
        if (point1.x > 0 && point1.x < 1 && point1.y > 0 && point1.y < 1 && point1.z > 0 && point1.z < 1) {
            color = depth1 < (point1.z - 0.001) ? color * (global_light_coef + c) : color * (global_light_coef + c) + color * (1 - (global_light_coef + c)) * light ;
        } else {
            color = depth2 < (point2.z - 0.001) ? color * (global_light_coef + c) : color * (global_light_coef + c) + color * (1 - (global_light_coef + c)) * light ;
        }
    } else {
        color = color * (global_light_coef + c) + color * (1 - (global_light_coef + c)) * light;
    }

  

   // color = vec4(depth_far, depth_far, depth_far, 1.0);
 
    gl_FragColor = color;

   // gl_FragColor = vec4(0.5, 0.5, 0.5, 1.0);

}
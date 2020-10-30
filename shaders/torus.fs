#version 330 core


in vec3 n;
in vec3 texture_coords;
in float dist;


uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;

uniform sampler2D detail_tex1;
uniform sampler2D detail_tex2;
uniform sampler2D detail_tex3;


uniform float detail_coef;
uniform int detail_repeat_count;
uniform float detail_dist;
uniform int tex1_repeat_count;
uniform int tex2_repeat_count;
uniform int tex3_repeat_count;


void main() {
    int coef = 5;

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

    if (texture_coords.z < 4.0 / 3.0) {
        gl_FragColor = tex_color1;
    } else if (texture_coords.z < 4.0 / 3.0 * 2) {
        gl_FragColor = tex_color2;
    } else {
        gl_FragColor = tex_color3;
    }

    if (dist < detail_dist) {
        gl_FragColor *= detail_coef * detail_color2.x;
    }

    // gl_FragColor = vec4(0.5, 0.5, 0.5, 1.0);

}
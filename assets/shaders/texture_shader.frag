#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "global.vert.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in in_data {
  vec3 in_frag_pos;
  vec3 in_normal;
  vec2 in_tex_coord;
};

layout(set = 0, binding = 1) uniform sampler2D diffuse_sampler;

// vec3 directional_light_calculate(DirectionalLight light, )

void main() {
  vec3 norm = normalize(in_normal);
  vec3 frag_pos = in_frag_pos;
  vec4 texture_color = texture(diffuse_sampler, in_tex_coord);
  vec3 view_dir = normalize(vec3(0) - frag_pos);

  float specularStrength = 0.5;
  vec3 ambient = vec3(0.1);

  vec3 total_light = vec3(0);

  for (int i = 0; i < g_directional_light_count; ++i) {
    DirectionaltLight light = g_directional_lights[i];
    vec3 light_pos = vec3(g_view * vec4(light.pos, 1));

    vec3 light_dir = normalize(light_pos - frag_pos);
    vec3 reflect_dir = reflect(-light_dir, norm);

    float spec = specularStrength * pow(max(dot(view_dir, reflect_dir), 0.0), 10);
    float diff = max(dot(norm, light_dir), 0.0);

    vec3 light_contrib = light.color * (diff + spec);

    total_light += light_contrib;
  }
  vec3 final_color = total_light + ambient;
  final_color = clamp(total_light, 0.0, 1.0);

  out_color = vec4(texture_color.rgb * final_color, texture_color.a);
} 

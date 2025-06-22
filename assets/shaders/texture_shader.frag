#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "global.vert.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in in_data {
  vec3 in_frag_pos;
  vec3 in_normal;
  vec2 in_texcoord;
};

layout(set = 0, binding = 1) uniform sampler2D diffuse_sampler;

vec3 point_light_calculate(int light_id) {
  PointLight light = g.point_lights[light_id];
  light.pos = vec3(g.view * vec4(light.pos, 1));

  vec3 light_dir = normalize(light.pos - frag_pos);
  vec3 reflect_dir = reflect(-light_dir, norm);

  float spec = g.entities[u_entity_id].specular_strength * pow(max(dot(view_dir, reflect_dir), 0.0), 10);
  float diff = max(dot(norm, light_dir), 0.0);

  vec3 light_contrib = light.color * (diff + spec);

  return light_contrib;
}

void main() {
  norm = normalize(in_normal);
  frag_pos = in_frag_pos;
  view_dir = normalize(vec3(0) - frag_pos);
  vec4 texture_color = texture(diffuse_sampler, in_texcoord);

  float specular_strength = 0.5;
  vec3 ambient = vec3(0.1);
  vec3 total_light = vec3(0.0);

  for (int i = 0; i < g.point_light_count; ++i) {
    total_light += point_light_calculate(i);
  }

  // for (int i = 0; i < g.point_light_count; ++i) {
  //   PointLight light = g.point_lights[i];
  //   vec3 light_pos = vec3(g.view * vec4(light.pos, 1));

  //   vec3 light_dir = normalize(light_pos - frag_pos);
  //   vec3 reflect_dir = reflect(-light_dir, norm);

  //   float spec = specular_strength * pow(max(dot(view_dir, reflect_dir), 0.0), 10);
  //   float diff = max(dot(norm, light_dir), 0.0);

  //   vec3 light_contrib = light.color * (diff + spec);

  //   total_light += light_contrib;
  // }
  vec3 final_color = total_light + ambient;
  final_color = clamp(final_color, 0.0, 1.0);

  vec3 color = texture_color.rgb*final_color + final_color*0.2;
  out_color = vec4(color, texture_color.a);
} 

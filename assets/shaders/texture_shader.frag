#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "global.vert.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in in_data {
  vec2 in_tex_coord;
};

layout(set = 0, binding = 1) uniform sampler2D diffuse_sampler;

void main() {
  out_color = texture(diffuse_sampler, in_tex_coord);
  // out_color.x += g_entities[u_entity_index].intensity;
  // out_color.x += g_entities[0].intensity;
  // out_color.y += g_entities[0].intensity;
  // out_color.z += g_entities[0].intensity;
  out_color.x += g_entities[u_entity_index].intensity;
  out_color.y += g_entities[u_entity_index].intensity;
  out_color.z += g_entities[u_entity_index].intensity;
  out_color.x += sin(g_time);
  out_color.y += sin(g_time);
  out_color.z += sin(g_time);
} 

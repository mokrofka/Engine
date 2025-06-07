#version 450 core

#extension GL_GOOGLE_include_directive : enable
#include "global.vert.glsl"

layout(location = 0) out vec4 out_color;

void main() {
  float color = g_entities[u_entity_index].intensity;
  vec3 rgb_color = g_entities[u_entity_index].color;
  // out_color = vec4(color,color,color, 1.0f);
  out_color = vec4(rgb_color, 1.0f);
} 

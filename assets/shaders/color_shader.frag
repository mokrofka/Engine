#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/fragdef.glsl"

void main() {
  // vec3 rgb_color = g.entities[u_entity_id].color;

  vec3 rgb_color = vec3(0.3);
  out_color = vec4(rgb_color, 1.0f);
} 


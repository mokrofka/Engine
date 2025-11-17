#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/fragdef.glsl"

layout(set = 0, binding = 1) uniform sampler2D texture_target;

void main() {
  // vec3 rgb_color = g.entities[u_entity_id].color;

  v4 texture_color = texture(texture_target, in_uv);
  out_color = texture_color;


  // vec3 rgb_color = vec3(0.3);
  // out_color = vec4(rgb_color, 1.0f);
} 


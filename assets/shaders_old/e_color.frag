#version 460
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/fragdef.glsl"

void main() {
  Entity e = entities[in_entity_id];
  out_color = vec4(e.color);
  f32 val = 0.6;
  // out_color = vec4(val, val, val, 0.6f);
}

#version 460
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/fragdef.glsl"
void main() {
  Entity e = entities[drawinfo[in_drawcall_id].entity_id];
  Material material = materials[e.material];

  v4 texture_color = texture(sampler2D(textures[material.texture], samplers[0]), in_uv);
  out_color = texture_color;
} 




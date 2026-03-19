#version 460
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/fragdef.glsl"
#extension GL_ARB_shader_draw_parameters : require
void main() {
  // Material material = st.materials[push.material];
  // Material material = st.materials[st.entities[push.id].material];

  Entity e = st.entities[drawinfo[in_drawcall_id].entity_id];
  // Entity e = st.entities[st.drawinfo[in_drawcall_id].entity_id];
  Material material = st.materials[e.material];

  v4 texture_color = texture(sampler2D(textures[material.texture], samplers[0]), in_uv);
  out_color = texture_color;
} 




#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"
#include "defines/fragdef.glsl"

void main() {
  Material material = st.materials[push.material];

  v4 texture_color = texture(sampler2D(textures[material.texture], samplers[0]), in_uv);
  out_color = texture_color;
  // out_color = v4(material.ambient, 1);
} 




#version 450 core
#extension GL_GOOGLE_include_directive : enable
#include "global.vert.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex_coord;

layout(location = 0) out out_data {
  vec2 out_tex_coord;
};

void main() {
  gl_Position = g_projection_view * u_model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
  
  out_tex_coord = in_tex_coord;
}

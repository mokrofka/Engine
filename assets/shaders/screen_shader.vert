#version 450 core

#extension GL_GOOGLE_include_directive : enable
#include "defines/global.glsl"

v2 positions[] = {
  {-1.0, -1.0},
  { 3.0, -1.0},
  {-1.0,  3.0},
};

v2 tex_coords[] = {
  { 0.0,  0.0},
  { 2.0,  0.0},
  { 0.0, -2.0} // reverse y-coord since gpu reads from bottom of texture
};

layout(location = 0) out out_data {
  v2 out_tex_coord;
};

void main() {
  gl_Position = v4(positions[gl_VertexIndex], 0.0, 1.0);
  out_tex_coord = tex_coords[gl_VertexIndex];
}

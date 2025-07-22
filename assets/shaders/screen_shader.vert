#version 450 core

vec2 positions[3] = vec2[](
  vec2(-1.0, -1.0),
  vec2( 3.0, -1.0),
  vec2(-1.0,  3.0)
);

vec2 tex_coords[3] = {
  { 0.0,  0.0},
  {2.0,  0.0},
  { 0.0, -2.0} // reverse y-coord since gpu reads from bottom of texture
};

layout(location = 0) out out_data {
  vec2 out_tex_coord;
};

void main() {
  gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
  out_tex_coord = tex_coords[gl_VertexIndex];
  
}

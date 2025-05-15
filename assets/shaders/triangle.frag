#version 450 core

layout(location = 0) out vec4 out_color;

layout(location = 0) in in_data {
  vec3 color;
  vec2 tex_coord;
} ind;

layout(set = 0, binding = 0) uniform sampler2D diffuse_sampler;

void main() {
  // out_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
  // out_color = vec4(in_dto.vec.x, in_dto.vec.y, in_dto.vec.z, 1.0f);
  // out_color = vec4(ubo.colors[0], ubo_new.colors[0], 0, 1.0f);
  
  // out_color = vec4(ubo.colors[1], ubo.colors[1], 0, 1.0f);
  // out_color = vec4(ubo.colors[512], ubo_new.colors[1000], 0, 1.0f);
  out_color = vec4(ind.color, 1.0f);
  // out_color = texture(diffuse_sampler, ind.tex_coord);
} 

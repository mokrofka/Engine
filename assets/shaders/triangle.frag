#version 450 core

layout(location = 0) out vec4 out_color;

layout(location = 0) in dto {
  vec3 vec;
} in_dto;

void main() {
  // out_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
  out_color = vec4(in_dto.vec.x, in_dto.vec.y, in_dto.vec.z, 1.0f);
} 

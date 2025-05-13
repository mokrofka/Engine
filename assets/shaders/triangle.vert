#version 450 core

layout(location = 0) in vec3 in_pos;

layout(push_constant) uniform push_const_ubo {
  mat4 model;
} ubo;

layout(location = 0) out dto {
  vec3 vec;
} out_dto;

void main() {
  gl_Position = ubo.model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
  out_dto.vec = (ubo.model * vec4(in_pos, 1.0)).xyz;
}

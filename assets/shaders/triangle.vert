#version 450 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;

layout(set = 0, binding = 0) uniform ObjectBuffer {
  float x[];
  float y[];
} ubo;

// per draw
layout(push_constant) uniform push_const_ubo {
  mat4 model;
} draw_ubo;

layout(location = 0) out dto {
  vec3 color;
} out_dto;

void main() {
  float x = ubo.x[0];
  float y = ubo.y[0];
  gl_Position = vec4(in_pos.x + x, in_pos.y + y, in_pos.z, 1.0);
  out_dto.color = in_color;
}

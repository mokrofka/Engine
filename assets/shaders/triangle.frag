#version 450 core

layout(location = 0) out vec4 out_color;

// per object
layout(set = 0, binding = 0) uniform ObjectBuffer {
  float colors[];
} ubo;

layout(set = 1, binding = 0) uniform ObjectBuffer_new {
  float colors[];
} ubo_new;

layout(location = 0) in dto {
  vec3 color;
} in_dto;

void main() {
  // out_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
  // out_color = vec4(in_dto.vec.x, in_dto.vec.y, in_dto.vec.z, 1.0f);
  // out_color = vec4(ubo.colors[0], ubo_new.colors[0], 0, 1.0f);
  
  
  // out_color = vec4(ubo.colors[1], ubo.colors[1], 0, 1.0f);
  // out_color = vec4(ubo.colors[512], ubo_new.colors[1000], 0, 1.0f);
  out_color = vec4(in_dto.color, 1.0f);
} 

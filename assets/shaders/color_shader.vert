#version 450 core

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;

layout(set = 0, binding = 0) uniform UniformBuffer {
  mat4 projection_view;
} ubo;

// per draw
layout(push_constant) uniform push_const_ubo {
  mat4 model;
} push_ubo;

layout(location = 0) out out_data {
  vec3 color;
  vec2 tex_coord;
} outd;

void main() {
  gl_Position = ubo.projection_view * push_ubo.model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
  
  // gl_Position = push_ubo.model * vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);
  // gl_Position = vec4(in_pos.x, in_pos.y, in_pos.z, 1.0);

  // outd.color = in_color;
  
  
  outd.color = in_color * 1.3;
  
  
  
  
  
  
  
  
  
  
  
}

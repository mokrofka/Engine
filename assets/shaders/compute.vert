#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;

layout(set = 0, binding = 0) uniform UniformBuffer {
  mat4 projection_view;
} ubo;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform push_const_ubo {
  mat4 model;
} push_ubo;

void main() {
  float distance = length((ubo.projection_view * vec4(in_position, 1.0)).z);
  gl_PointSize = clamp(10.0 / distance, 1.0, 32.0);
  
  // gl_PointSize = 14.0;
  // gl_Position = ubo.projection_view * vec4(in_position.xy, 1.0, 1.0);
  // gl_Position = ubo.projection_view * push_ubo.model * vec4(in_position, 1.0);
  gl_Position = ubo.projection_view * push_ubo.model * vec4(in_position, 1.0);
  fragColor = in_color.xyz;
}

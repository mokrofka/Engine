#version 450 core

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform UniformBuffer {
  mat4 projection_view;
} ubo;

void main() {
  float val = 0.5;
  float alpha = 0.5f;
  out_color = vec4(val, val, val, alpha);
}

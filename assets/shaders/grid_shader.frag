#version 450 core

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform UniformBuffer {
  mat4 projection_view;
};

void main() {
  projection_view;
  float val = 0.1;
  out_color = vec4(val, val, val, 0.4f);
}

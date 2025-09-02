#version 450 core

layout(location = 0) out vec4 out_color;

void main() {
  float val = 0.5;
  float alpha = 0.5f;
  out_color = vec4(vec3(val), alpha);
}

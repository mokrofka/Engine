#version 450 core

layout(location = 0) out vec4 out_color;

layout(location = 0) in in_data {
  vec2 in_texcoord;
};

layout(set = 0, binding = 1) uniform sampler2D screen;

void main() {
  vec4 texture_color = texture(screen, in_texcoord);
  out_color = texture_color;
}

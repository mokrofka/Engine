#version 450 core

layout(location = 0) out vec4 out_color;

layout(location = 0) in in_data {
  vec3 in_color;
  vec2 in_tex_coord;
};

layout(set = 0, binding = 1) uniform sampler2D diffuse_sampler;

void main() {
  out_color = vec4(in_color, 1.0f);
} 

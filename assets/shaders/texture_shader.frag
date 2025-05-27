#version 450 core

layout(location = 0) out vec4 out_color;

layout(location = 0) in in_data {
  vec3 color;
  vec2 tex_coord;
} ind;

layout(set = 0, binding = 1) uniform sampler2D diffuse_sampler;

void main() {
  out_color = texture(diffuse_sampler, ind.tex_coord);
  float value = 0.5;
  // out_color = vec4(value, value, value, 1.0);
  
  // vec3 gamma_encoded = texture(diffuse_sampler, ind.tex_coord).rgb;
  // vec3 linear_color = pow(gamma_encoded, vec3(2.2));
  // out_color = vec4(linear_color, 1.0);
} 

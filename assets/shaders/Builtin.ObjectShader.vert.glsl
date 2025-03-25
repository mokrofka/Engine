#version 450
// #extension GL_ARB_seperate_shader_objects : enable

layout(location = 0) in vec3 in_position;

void main() {
  gl_Position = vec4(in_position, 1.0);
}

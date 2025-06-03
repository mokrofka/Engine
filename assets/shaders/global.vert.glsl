
struct Entity {
  float intensity;
  float pad1;
  float pad2;
  float pad3;
};

layout(set = 0, binding = 0) readonly buffer Buffer {
  mat4 g_projection_view;
  float g_time;
  Entity g_entities[];
};

layout(push_constant) uniform PushConstants {
  mat4 u_model;
  uint u_entity_index;
};

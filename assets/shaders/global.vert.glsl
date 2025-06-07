
#define KB(n)  ((n) << 10)

struct Entity {
  float intensity;
  vec3 color;
  vec4 padd;
};

struct DirectionaltLight {
  vec3 pos;
  vec3 direction;
  vec3 color;
};

layout(set = 0, binding = 0) readonly buffer Buffer {
  mat4 g_projection_view;
  mat4 g_view;
  vec4 g_ambient_color;
  float g_time;
  uint g_directional_light_count;
  Entity g_entities[KB(20)];
  DirectionaltLight g_directional_lights[KB(1)];
};

layout(push_constant) uniform PushConstants {
  mat4 u_model;
  uint u_entity_index;
};

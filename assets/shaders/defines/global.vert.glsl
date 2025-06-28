
#define KB(n) ((n) << 10)

struct Entity {
  vec3 color;
  vec3 ambient;
  // vec3 diffuse;
  // vec3 specular;
  // float shininess;
};

struct PointLight {
  vec3 color;
  vec3 pos;
  float intensity;
  float rad;
};

struct DirLight {
  vec3 color;
  vec3 direction;
  float intensity;
};

struct SpotLight {
  vec3 color;
  vec3 pos;
  vec3 direction;
  float intensity;
  float inner_cutoff; // Cosine of inner cone angle
  float outer_cutoff; // Cosine of outer cone angle
};

// Global state
layout(std430, set = 0, binding = 0) readonly buffer Buffer {
  mat4 projection_view;
  mat4 view;
  vec4 ambient_color;
  float time;
  uint point_light_count;
  uint dir_light_count;
  uint spot_light_count;

  Entity entities[KB(20)];

  PointLight point_lights[KB(1)];
  DirLight dir_lights[KB(1)];
  SpotLight spot_lights[KB(1)];
  
} g;

layout(push_constant) uniform PushConstants {
  mat4 u_model;
  uint u_entity_id;
};

vec3 norm;
vec3 frag_pos;
vec3 view_dir;
Entity e;


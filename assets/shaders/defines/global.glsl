
#define KB(n) ((n) << 10)

#define i32 int
#define u32 uint

#define f32 float

#define v2 vec2
#define v3 vec3
#define v4 vec4

struct Entity {
  v3 color;
  // v3 ambient;
  // v3 diffuse;
  // v3 specular;
  // f32 shininess;
};

struct PointLight {
  v3 color;
  v3 pos;
  f32 intensity;
  f32 rad;
};

struct DirLight {
  v3 color;
  v3 direction;
  f32 intensity;
};

struct SpotLight {
  v3 color;
  v3 pos;
  v3 direction;
  f32 intensity;
  f32 inner_cutoff; // Cosine of inner cone angle
  f32 outer_cutoff; // Cosine of outer cone angle
};

// Global state
layout(std430, set = 0, binding = 0) readonly buffer Buffer {
  mat4 projection_view;
  mat4 projection;
  mat4 view;
  v4 ambient_color;
  f32 time;
  u32 point_light_count;
  u32 dir_light_count;
  u32 spot_light_count;

  Entity entities[KB(1)];

  PointLight point_lights[KB(1)];
  DirLight dir_lights[KB(1)];
  SpotLight spot_lights[KB(1)];
} st;

layout(push_constant) uniform PushConstants {
  mat4 model;
  u32 id;
} push;

v3 norm;
v3 frag_pos;
v3 view_dir;
Entity e;


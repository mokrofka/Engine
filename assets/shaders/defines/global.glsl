#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_ARB_shader_draw_parameters : enable

#define KB(n) ((n) << 10)
#define MaxEntities KB(1)
#define MaxMaterials KB(1)
#define MaxLights KB(1)

#define i32 int
#define u32 uint

#define f32 float

#define v2 vec2
#define v3 vec3
#define v4 vec4

struct Entity {
  mat4 model;
  v4 color;
  u32 material;
};

struct Material {
  v3 ambient;
  v3 diffuse;
  v3 specular;
  f32 shininess;
  u32 texture;
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

struct DrawCallInfo {
  u32 vertexCount;
  u32 instanceCount;
  u32 firstVertex;
  u32 firstInstance;
  u32 entity_id;
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

  Entity entities[MaxEntities];
  Material materials[MaxMaterials];
  PointLight point_lights[MaxLights];
  DirLight dir_lights[MaxLights];
  SpotLight spot_lights[MaxLights];
} st;

layout(set = 0, binding = 1) uniform texture2D textures[];
layout(set = 0, binding = 2) uniform sampler samplers[];
layout(set = 0, binding = 3) uniform textureCube cube_texture;
layout(set = 0, binding = 4) readonly buffer DrawInfoBuffer { DrawCallInfo drawinfo[]; };

layout(push_constant) uniform PushConstants {
  u32 id;
  u32 material;
} push;

v3 norm;
v3 frag_pos;
v3 view_dir;



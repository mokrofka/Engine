
struct GpuState {
  alignas(16) mat4 projection_view;
  alignas(16) mat4 projection;
  alignas(16) mat4 view;
  mat4 mat;
  alignas(16) v4 ambient_color;
  f32 time;
  u32 point_light_count;
  u32 dir_light_count;
  u32 spot_light_count;
  u32 cubemap;
};

enum Bindings {
  State,
  Textures,
  Samplers,
  CubeTextures,
  Drawinfo,
  Entities,
  Materials,
  PointLights,
  DirLights,
  SpotLights,
  DrawCtx,
};


struct GpuState {
  alignas(16) mat4 projection_view;
  alignas(16) mat4 projection;
  alignas(16) mat4 view;
  mat4 mat;
  alignas(16) v4 ambient_color;
  u64 p;
  v2 res;
  f32 time;
  u32 point_light_count;
  u32 dir_light_count;
  u32 spot_light_count;
  u32 cubemap;
  // u32 win_width;
  // u32 win_height;
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
  SoftwareRender,
  UI_RectBinding,
};

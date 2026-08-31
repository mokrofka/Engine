
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
  Vertices,
};

enum class ShaderType {
  Cube = 1,
  E_Color,
  Texture,
  VertColor,
  Line,
  Screen,
  SoftwareRender,
};

typedef u32 DrawCallFlags;
enum {
  DrawCallFlag_Indexed,
};

struct GpuState {
  alignas(16) m4x4 projection_view;
  alignas(16) m4x4 projection;
  alignas(16) m4x4 view;
  m4x4 mat;
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

struct Glue(R_, Vertex) {
  alignas(16) v3 pos;
  alignas(16) v3 norm;
  alignas(8)  v2 uv;
  alignas(16) v4 color;
};

struct Glue(Gpu, DrawCall) {
  alignas(16) m4x4 model;
  alignas(16) v4 color;
  alignas(16) m4x4 m;
  ShaderType type;
  // u32 m;
  u32 tex;
  u32 mat;
  u32 cur_resolve_idx;
  DrawCallFlags flags;
};

struct Glue(Gpu, Entity) {
};

struct Glue(Gpu, Material) {
  alignas(16) v3 ambient;
  alignas(16) v3 diffuse;
  alignas(16) v3 specular;
  f32 shininess;
  u32 tex;
};

struct Glue(Gpu, PointLight) {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  f32 intensity;
  f32 rad;
};

struct Glue(Gpu, DirLight) {
  alignas(16) v3 color;
  alignas(16) v3 dir;
  f32 intensity;
};

struct Glue(Gpu, SpotLight) {
  alignas(16) v3 color;
  alignas(16) v3 pos;
  alignas(16) v3 dir;
  f32 intensity;
  f32 inner_cutoff;
  f32 outer_cutoff;
};

global const u32 GpuUI_RectFlag_IsFont = Bit(0);

struct Glue(Gpu, UI_Rect) {
  alignas(8) v2 dst_p0;
  alignas(8) v2 dst_p1;
  alignas(8) v2 src_p0;
  alignas(8) v2 src_p1;
  alignas(16) v4 colors[4];
  u32 texture;
  u32 flags;
  f32 corner_radius;
  f32 edge_softness;
};





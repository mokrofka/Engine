#include "lib.slang"

static v3 triangle[] = {
  v3(-0.5, -0.5, 0),
  v3(0.5, -0.5, 0),
  v3(0.0, 0.5, 0),
};

enum CoolType {
  bad,
}

void foo2(in i32 a) {
}

VkBind(Bindings.data) StructuredBuffer<u32> data;
VkBind(Bindings.textures) Texture2D textures[];
VkBind(Bindings.texture_cubes) TextureCube texture_cubes[];
VkBind(Bindings.samplers) SamplerState samplers[];

struct ShaderStorage {
  void* res1;
  void* res2;
  void* res3;
}

VS_Entry VSOutput vs_main(VSInput in) {
  // u32 a = SV_VertexID;
  VSOutput o;
  i32 a = 1;
  v3 pos = triangle[in.vert_id];
  o.pos = v4(pos, 1.0);
  return o;
}

FS_Entry v4 fs_main(VSOutput in) : SV_Target {
  return v4(1.0, 0.0, 0.0, 1.0);
}

void foo1() {
  Thing thing;
}

struct Thing {
  u32 a;
}


enum MetaType {
  MetaType_Null,
  MetaType_u32,
  MetaType_i32,
  MetaType_b32,
  MetaType_f32,
  MetaType_v2,
  MetaType_v3,
  MetaType_Rng2,
  MetaType_Rng3,
  MetaType_GpuMeshId,
  MetaType_GpuMaterialId,
  MetaType_RenderId,
  MetaType_String,
  MetaType_EntityFlags
};

struct MemberDefinition {
  MetaType type;
  String name;
  u64 offset;
};

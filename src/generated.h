MemberDefinition members_of_Camera[] = {
  {MetaType_v3, "pos", OffsetOf(Camera,pos)},
  {MetaType_v3, "dir", OffsetOf(Camera,dir)},
  {MetaType_f32, "yaw", OffsetOf(Camera,yaw)},
  {MetaType_f32, "pitch", OffsetOf(Camera,pitch)},
  {MetaType_f32, "fov", OffsetOf(Camera,fov)},
  {MetaType_f32, "speed", OffsetOf(Camera,speed)},
};
MemberDefinition members_of_Entity[] = {
  {MetaType_String, "name", OffsetOf(Entity,name)},
  {MetaType_EntityFlags, "flags", OffsetOf(Entity,flags)},
  {MetaType_v3, "pos", OffsetOf(Entity,pos)},
  {MetaType_v3, "rot", OffsetOf(Entity,rot)},
  {MetaType_v3, "scale", OffsetOf(Entity,scale)},
  {MetaType_Rng3, "aabb", OffsetOf(Entity,aabb)},
  {MetaType_v3, "vel", OffsetOf(Entity,vel)},
  {MetaType_GpuMeshId, "mesh_id", OffsetOf(Entity,mesh_id)},
  {MetaType_GpuMaterialId, "material_id", OffsetOf(Entity,material_id)},
  {MetaType_RenderId, "render_id", OffsetOf(Entity,render_id)},
};
MemberDefinition members_of_StaticEntity[] = {
  {MetaType_v3, "pos", OffsetOf(StaticEntity,pos)},
  {MetaType_v3, "rot", OffsetOf(StaticEntity,rot)},
  {MetaType_v3, "scale", OffsetOf(StaticEntity,scale)},
  {MetaType_GpuMeshId, "mesh_id", OffsetOf(StaticEntity,mesh_id)},
  {MetaType_GpuMaterialId, "material_id", OffsetOf(StaticEntity,material_id)},
  {MetaType_RenderId, "render_id", OffsetOf(StaticEntity,render_id)},
};

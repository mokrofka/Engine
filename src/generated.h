MemberDefinition members_of_Camera[] = {
  {MetaType_v3, "pos", OffsetOf(Camera,pos)},
  {MetaType_v3, "dir", OffsetOf(Camera,dir)},
  {MetaType_f32, "yaw", OffsetOf(Camera,yaw)},
  {MetaType_f32, "pitch", OffsetOf(Camera,pitch)},
  {MetaType_f32, "fov", OffsetOf(Camera,fov)},
  {MetaType_f32, "speed", OffsetOf(Camera,speed)},
};
MemberDefinition members_of_Entity[] = {
  {MetaType_v3, "vel", OffsetOf(Entity,vel)},
  {MetaType_Rng3, "aabb", OffsetOf(Entity,aabb)},
  {MetaType_u32, "a", OffsetOf(Entity,a)},
  {MetaType_u32, "b", OffsetOf(Entity,b)},
  {MetaType_Rng2, "rect", OffsetOf(Entity,rect)},
  {MetaType_u32, "pa", OffsetOf(Entity,pa)},
};

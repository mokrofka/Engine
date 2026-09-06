MemberDefinition members_of_Camera[] = {
	{MetaType_v3, "pos", OffsetOf(Camera,pos)},
	{MetaType_v3, "dir", OffsetOf(Camera,dir)},
	{MetaType_f32, "yaw", OffsetOf(Camera,yaw)},
	{MetaType_f32, "pitch", OffsetOf(Camera,pitch)},
	{MetaType_f32, "fov", OffsetOf(Camera,fov)},
	// {MetaType_f32, "speed", OffsetOf(Camera,speed)},
};
MemberDefinition members_of_Entity[] = {
	{MetaType_String, "name", OffsetOf(Thing,name)},
	{MetaType_EntityFlags, "flags", OffsetOf(Thing,flags)},
	{MetaType_v3, "pos", OffsetOf(Thing,pos)},
	{MetaType_v3, "rot", OffsetOf(Thing,rot)},
	{MetaType_v3, "scale", OffsetOf(Thing,scale)},
	{MetaType_Rng3, "aabb", OffsetOf(Thing,aabb)},
	{MetaType_v3, "vel", OffsetOf(Thing,vel)},
	{MetaType_MeshId, "mesh_id", OffsetOf(Thing,mesh)},
	{MetaType_MaterialId, "material_id", OffsetOf(Thing,mat)},
	{MetaType_v4, "color", OffsetOf(Thing,color)},
};

#include "vk_types.h"
#include "vk_buffer.h"

u32 mesh_count;

void vk_r_geometry_create(Geometry geom) {
  u64 size = geom.vertex_size * geom.vertex_count;
  u64 offset = freelist_gpu_alloc(vk.vert_buffer.freelist, size);
  
  Range range = {offset, size};
  VK_Mesh mesh = {
    .offset = offset,
    .vert_count = geom.vertex_count,
  };
  vk.meshes[mesh_count++] = mesh;
  vk_upload_to_gpu(vk.vert_buffer, range, geom.vertices);
}

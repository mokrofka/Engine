#include "vk_buffer.h"
#include "vk_types.h"

#include "res/res_types.h"

void vk_r_create_geometry(Geometry* geometry, u32 vertex_count, Vertex3D* vertices, u32 index_count, u32* indices) {
  // if (!vertex_count || !vertices) {
  //   Error("vulkan_renderer_create_geometry requires vertex data, and none was supplied. vertex_count=%d, vertices=%p", vertex_count, vertices);
  // }

  // // Check if this is a re-upload. If it is, need to free old data afterward.
  // b32 is_reupload = geometry->internal_id != INVALID_ID;
  // VK_GeometryData old_range;

  // VK_GeometryData* internal_data = 0;
  // if (is_reupload) {
  //   internal_data = &vk->render.geometries[geometry->internal_id];

  //   // Take a copy of the old range.
  //   old_range.index_buffer_offset = internal_data->index_buffer_offset;
  //   old_range.index_count = internal_data->index_count;
  //   old_range.index_size = internal_data->index_size;
  //   old_range.vertex_buffer_offset = internal_data->vertex_buffer_offset;
  //   old_range.vertex_count = internal_data->vertex_count;
  //   old_range.vertex_size = internal_data->vertex_size;
  // } else {
  //   Loop (i, VK_MAX_GEOMETRY_COUNT) {
  //     if (vk->render.geometries[i].id == INVALID_ID) {
  //       // Found a free index.
  //       geometry->internal_id = i;
  //       vk->render.geometries[i].id = i;
  //       internal_data = &vk->render.geometries[i];
  //       break;
  //     }
  //   }
  // }
  // if (!internal_data) {
  //   Error("vulkan_renderer_create_geometry failed to find a free index for a new geometry upload. Adjust config to allow for more"_);
  // }

  // VkCommandPool pool = vk->device.gfx_cmd_pool;
  // VkQueue queue = vk->device.graphics_queue;

  // // Vertex data.
  // internal_data->vertex_buffer_offset = vk->render.geometry_vertex_offset;
  // internal_data->vertex_count = vertex_count;
  // internal_data->vertex_size = sizeof(Vertex3D) * vertex_count;
  // upload_data_range(pool, 0, queue, &vk->render.obj_vertex_buffer, internal_data->vertex_buffer_offset, internal_data->vertex_size, vertices);
  // // TODO: should maintain a free list instead of this.
  // vk->render.geometry_vertex_offset += internal_data->vertex_size;

  // // Index data, if applicable
  // if (index_count && indices) {
  //   internal_data->index_buffer_offset = vk->render.geometry_index_offset;
  //   internal_data->index_count = index_count;
  //   internal_data->index_size = sizeof(u32) * index_count;
  //   upload_data_range(pool, 0, queue, &vk->render.obj_index_buffer, internal_data->index_buffer_offset, internal_data->index_size, indices);
  //   // TODO: should maintain a free list instead of this.
  //   vk->render.geometry_index_offset += internal_data->index_size;
  // }

  // if (internal_data->generation == INVALID_ID) {
  //   internal_data->generation = 0;
  // } else {
  //   internal_data->generation++;
  // }

  // if (is_reupload) {
  //   // Free vertex data
  //   free_data_range(&vk->render.obj_vertex_buffer, old_range.vertex_buffer_offset, old_range.vertex_size);

  //   // Free index data, if applicable
  //   if (old_range.index_size > 0) {
  //     free_data_range(&vk->render.obj_index_buffer, old_range.index_buffer_offset, old_range.index_size);
  //   }
  // }
}

u32 mesh_count;

void vk_r_geometry_create(Geometry* geom) {
  u64 size = geom->vertex_size * geom->vertex_count;
  u64 offset = vk_buffer_alloc(&vk->vert_buffer, size, 64);
  
  MemRange range = {offset, size};
  VK_Mesh mesh = {
    .offset = offset,
    .vert_count = geom->vertex_count,
  };
  vk->meshes[mesh_count++] = mesh;
  upload_data_range(&vk->vert_buffer, range, geom->vertices);
}

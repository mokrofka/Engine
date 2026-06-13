
struct IndirectCursor {
  u32 drawcall_cursor;
};

struct IndirectInstanceCtx {
  u32* base_index;
  u32 entity_cursor;
};

IndirectInstanceCtx ctx;
IndirectCursor w;

void gfx_push_indirect(Gfx_Mesh mesh, u32 id, u32 instance_count = 1) {
  VK_DrawCallInfo info = {};
  if (mesh.index_count) {
    info.index_draw_command = (VkDrawIndexedIndirectCommand){
      .indexCount = mesh.index_count,
      .instanceCount = instance_count,
      .firstIndex = mesh.base_index,
      .vertexOffset = (i32)mesh.base_vert,
      .firstInstance = 0,
    };
  } else {
    info.draw_command = (VkDrawIndirectCommand){
      .vertexCount = mesh.vert_count,
      .instanceCount = instance_count,
      .firstVertex = mesh.base_vert,
      .firstInstance = 0,
    };
  }
  info.base_instance = id;
  VK_DrawCallInfo* drawcalls = (VK_DrawCallInfo*)st->vk.indirect_draw_buffer.base;
  drawcalls[w.drawcall_cursor++] = info;
}

void gfx_push_indirect_instanced(Gfx_Mesh mesh, u32 count) {
  gfx_push_indirect(mesh, ctx.entity_cursor, count);
  ctx.entity_cursor += count;
}

u32* gfx_indirect_indices() { return ctx.base_index + ctx.entity_cursor; }
u32 gfx_indirect_begin()    { return w.drawcall_cursor; }

Gfx_IndirectDrawcall gfx_indirect_end(u32 base) {
  Gfx_IndirectDrawcall res = {
    .base = base,
    .count = w.drawcall_cursor - base,
  };
  return res;
}

void drawcall() {
  Gfx_Mesh mesh0 = {};
  Gfx_Mesh mesh1 = {};
  Gfx_Mesh mesh2 = {};
  u32 entity0 = 0;
  u32 entity1 = 1;
  u32 entity2 = 1;

  u32 base = gfx_indirect_begin();

  gfx_push_indirect(mesh0, entity0);
  gfx_push_indirect(mesh1, entity1);
  gfx_push_indirect(mesh2, entity2);

  Gfx_IndirectDrawcall drawcall = gfx_indirect_end(base);
  gfx_draw_indirect(drawcall);
}

void instanced_drawcall() {
  Gfx_Mesh mesh0 = {};
  Gfx_Mesh mesh1 = {};
  Gfx_Mesh mesh2 = {};

  u32 entities0[100];
  for EachElement(i, entities0) {
    entities0[i] = i;
  }
  u32 entities1[100];
  for EachElement(i, entities1) {
    entities1[i] = i + 100;
  }
  u32 entities2[100];
  for EachElement(i, entities2) {
    entities2[i] = i + 200;
  }


  // gfx_instance_ctx_set(st->vk.gpu_entities_indices);

  u32 base = gfx_indirect_begin();

  {
    u32* indices = gfx_indirect_indices();
    Loop (i, 100) {
      indices[i] = entities0[i];
    }
    gfx_push_indirect_instanced(mesh0, 100);
  }
  {
    u32* indices = gfx_indirect_indices();
    Loop (i, 100) {
      indices[i] = entities1[i];
    }
    gfx_push_indirect(mesh1, 100);
  }
  {
    u32* indices = gfx_indirect_indices();
    Loop (i, 100) {
      indices[i] = entities2[i];
    }
    gfx_push_indirect(mesh2, 100);
  }

  Gfx_IndirectDrawcall drawcall = gfx_indirect_end(base);
  gfx_draw_indirect(drawcall);
}

// void foo() {
//   Gfx_IndirectDrawcall drawcall = drawcall_begin();
//   u32 base_drawcall = drawcall_begin();
//   VK_Mesh mesh = {};
//   u32 entity_id = 0;
//   gfx_push_indirect(mesh, entity_id);
//   VK_Mesh mesh1 = {};
//   entity_id = 1;
//   gfx_push_indirect(mesh1, entity_id);
//   // drawcall = drawcall_end(drawcall);
//   drawcall = drawcall_end(base_drawcall);
//   gfx_draw_indirect(drawcall);
// }

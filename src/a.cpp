
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
  VK_DrawCall info = {};
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
  VK_DrawCall*  = (VK_DrawCall*)st->vk.indirect_draw_buffer.base;
  [w.drawcall_cursor++] = info;
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
  LoopArray (i, entities0) {
    entities0[i] = i;
  }
  u32 entities1[100];
  LoopArray (i, entities1) {
    entities1[i] = i + 100;
  }
  u32 entities2[100];
  LoopArray (i, entities2) {
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


struct ThingId {
  u32 idx;
  u32 gen;
};

struct Thing {
  ThingId parent;
  ThingId first;
  ThingId last;
  ThingId next;
  ThingId prev;
};

Pool<Thing, 100, ThingId> things;

void foo() {
  ThingId id = pool_push(things);
  Thing& thing = pool_get(id);
}

struct TreeNode {
  TreeNode* parent;
  List list;
  Node node;
};

struct A {
  TreeNode tree;
  // A* parent;
  // A* first;
  // A* last;
  // A* next;
  // A* prev;
};

struct Dar {
  Node node;
  u32 a;
  u32 b;
};

struct Entity {
  EntityId parent;
  EntityId first;
  EntityId last;
  EntityId next;
  EntityId prev;
};

void foo() { 
  Scratch scratch;
  List list = {};
  Dar d0 = {.a = 0};
  Dar d1 = {.a = 1};
  Dar d2 = {.a = 2};
  list_push(&list, &d0.node);
  list_push(&list, &d1.node);
  list_push(&list, &d2.node);

  ListFor (i, Dar, list) {
    Info("%i", i->a);
  }
  #define ForEach ()
  // EachElement

  // Slice<u8> data = os_read_all(...);
  // struct Header {
  //   u32 a;
  //   u32 b;
  //   u32 c;
  // };
  // Header* header = (Header*)data.data;
  // u32* magic = (u32*)slice_skip(data, sizeof(Header)).data;



}

// void foo() {
//   struct A {
//     u32 a;
//     u32 b;
//   } a;
//   u32* pb = &a.b;
//   A* pa = ContainerOf(pb, A, b);
// }

struct Texture {
  u32 width;
  u32 height
  u8* data;
};

void foo() {
  
}

struct Writer {
  u8* base;
  u64 cursor;
};

u64 write(Writer* w, Slice<u8> data) {
  MemCopy(w->base + w->cursor, data.data, data.size);
  u64 res = w->cursor;
  w->cursor += data.size;
  return res;
}

void mem_copy_push(void* dst, Slice<Slice<u8>> slices) {
  u64 offset = 0;
  Loop (i, slices.count) {
    u64 size = slice_size(slices[i]);
    MemCopy(Offset(dst, size), slices[i].data, size);
    offset += size;
  }
}

a.h {
  struct A {
    B_T;
  };
  void a_do0();
  void a_do1();
}

a.c {
  void a_do0() {}
  void a_do1() {}
}

b.h {
  struct B {
    A_T;
  };
  void b_do0();
  void b_do1();
}

b.c {
  void b_do0() {}
  void b_do1() {}
}

com.h {
  struct A_T {
  };
  struct B_T {
  };
  #include "a.h"
  #include "b.h"
  struct Com {
  };
  void com();
}

inc.c {
  #include "file0.h"
  #include "file1.h"
  #include "file2.h"
  #include "file0.c"
  #include "file1.c"
  #include "file2.c"
}

struct Node {
  Node* next;
  Node* prev;
};
struct List {
  Node* first;
  Node* last;
  u32 count;
};

void list_push_front(List* list, Node* n) {
  n->next = list->first;
  n->prev = null;
  if (list->first) {
    list->first->prev = n;
  } else {
    list->last = n;
  }
  list->first = n;
  ++list->count;
}
void list_push(List* list, Node* n) {
  n->prev = list->last;
  n->next = null;
  if (list->last) {
    list->last->next = n;
  } else {
    list->first = n;
  }
  list->last = n;
  ++list->count;
}
void list_remove(List* list, Node* n) {
  if (n == null) return;
  if (n->prev) {
    n->prev->next = n->next;
  } else {
    list->first = n->next;
  }
  if (n->next) {
    n->next->prev = n->prev;
  } else {
    list->last = n->prev;
  }
  --list->count;
}
Node* list_pop_front(List* list) {
	Node* n = list->first;
	if (n == null) return null;
  list->first = n->next;
  if (list->first) {
    list->first->prev = null;
  } else {
    list->last = null;
  }
  --list->count;
  return n;
}
Node* list_pop(List* list) {
	Node* n = list->last;
	if (n == null) return null;
  list->last = n->prev;
  if (list->last) {
    list->last->next = null;
  } else {
    list->first = null;
  }
  --list->count;
  return n;
}

void sll_queue_push(List* list, Node* n) {
  n->next = null;
  if (list->last) {
    list->last->next = n;
  } else {
    list->first = n;
  }
  list->last = n;
  ++list->count;
}
Node* sll_queue_pop(List* list) {
  Node* n = list->first;
  if (!n) return null;
  list->first = n->next;
  if (list->first == null) {
    list->last = null;
  }
  --list->count;
  return n;
}

void sll_stack_push(Node*& first, Node* n) {
  n->next = first;
  first = n;
}
void sll_stack_pop(Node*& first, Node* n) {
  first = first->next;
}


struct NodeIdx {
  u32 next;
  u32 prev;
};
struct ListIdx {
  u32 first;
  u32 last;
  u32 count;
};

template<typename T> void list_push_front(T* arr, ListIdx* list, u32 n) {
  NodeIdx& node = arr[n].node;
  node.next = list->first;
  node.prev = 0;
  if (list->first) {
    arr[list->first].node.prev = n;
  } else {
    list->last = n;
  }
  list->first = n;
  ++list->count;
}
template<typename T> void list_push(T* arr, ListIdx* list, u32 n) {
  NodeIdx& node = arr[n].node;
  node.next = list->last;
  node.prev = 0;
  if (list->last) {
    arr[list->last].node.next = n;
  } else {
    list->first = n;
  }
  list->last = n;
  ++list->count;
}
template<typename T> void list_remove(T* arr, ListIdx* list, u32 n) {
  if (n == 0) return;
  NodeIdx& node = arr[n].node;
  if (node.prev) {
    arr[node.prev].node.next = node.next;
  } else {
    list->first = node.next;
  }
  if (node.next) {
    arr[node.next].node.prev = node.prev;
  } else {
    list->last = node.prev;
  }
  --list->count;
}
template<typename T> u32 list_pop_front(T* arr, ListIdx* list) {
	u32 n = list->first;
	if (n == 0) return 0;
  NodeIdx& node = arr[n].node;
  list->first = node.next;
  if (list->first) {
    arr[list->first].node.prev = 0;
  } else {
    list->last = 0;
  }
  --list->count;
  return n;
}
template<typename T> u32 list_pop(T* arr, ListIdx* list) {
	u32 n = list->last;
	if (n == 0) return 0;
  NodeIdx& node = arr[n].node;
  list->last = node.prev;
  if (list->last) {
    arr[list->last].node.next = 0;
  } else {
    list->first = 0;
  }
  --list->count;
  return n;
}

#define ListIdxFor(it, T, arr, list)                      \
  for (T& it = *ContainerOf(&arr[(list).first], T, node); \
       it.node.next != 0;                                 \
       it = *ContainerOf(&arr[it.node.next], T, node))

template<typename T, typename L, typename H> void list_push_front(T* p,  L* list, H n) {
  T* arr = p->data;
  NodeIdx& node = p->data[n].node;
  node.next = list->first;
  node.prev = 0;
  if (list->first) {
    arr[list->first].node.prev = n;
  } else {
    list->last = n;
  }
  list->first = n;
}

struct DrawCall_std430 {
  float4x4 model : [[RowMajor, MatrixStride(16), Offset(0)]];
  float4 color : [[Offset(64)]];
  int type : [[Offset(80)]];
  float3x4 m : [[RowMajor, MatrixStride(16), Offset(96)]];
  uint tex : [[Offset(144)]];
  uint mat : [[Offset(148)]];
  uint cur_resolve_idx : [[Offset(152)]];
  uint flags : [[Offset(156)]];
}
struct StructuredBuffer : [[Block]] {
  DrawCall_std430[] __member0 : [[Offset(0)]] : [[ArrayStride(160)]];
}

struct DrawCall_std430 {
  float4x4 model : [[RowMajor, MatrixStride(16), Offset(0)]];
  float4 color : [[Offset(64)]];
  int type : [[Offset(80)]];
  float4x3 m : [[RowMajor, MatrixStride(16), Offset(96)]];
  uint tex : [[Offset(160)]];
  uint mat : [[Offset(164)]];
  uint cur_resolve_idx : [[Offset(168)]];
  uint flags : [[Offset(172)]];
}
struct StructuredBuffer : [[Block]] {
  DrawCall_std430[] __member0 : [[Offset(0)]] : [[ArrayStride(176)]];
}


DebugEntryPoint(vs_main, Slang Compiler from Khronos slangc,

    command line: -target spirv  -I "/opt/shader-slang-bin/bin" -matrix-layout-column-major -O0 -stage vertex -entry vs_main -g2);

struct DrawCall_std430 {

  float4x4 model : [[RowMajor, MatrixStride(16), Offset(0)]];

  float4 color : [[Offset(64)]];

  int type : [[Offset(80)]];

  float3x4 m : [[RowMajor, MatrixStride(16), Offset(96)]];

  uint tex : [[Offset(144)]];

  uint mat : [[Offset(148)]];

  uint cur_resolve_idx : [[Offset(152)]];

  uint flags : [[Offset(156)]];

}

struct StructuredBuffer : [[Block]] {

  DrawCall_std430[] __member0 : [[Offset(0)]] : [[ArrayStride(160)]];

}

struct DrawCall_std430 {

  float4x4 model : [[RowMajor, MatrixStride(16), Offset(0)]];

  float4 color : [[Offset(64)]];

  int type : [[Offset(80)]];

  float4x3 m : [[RowMajor, MatrixStride(16), Offset(96)]];

  uint tex : [[Offset(160)]];

  uint mat : [[Offset(164)]];

  uint cur_resolve_idx : [[Offset(168)]];

  uint flags : [[Offset(172)]];

}

struct StructuredBuffer : [[Block]] {

  DrawCall_std430[] __member0 : [[Offset(0)]] : [[ArrayStride(176)]];

}





}




DebugEntryPoint(vs_main, Slang Compiler from Khronos slangc,

    command line: -target spirv  -I "/opt/shader-slang-bin/bin" -matrix-layout-row-major -O0 -stage vertex -entry vs_main -g2);

const float2[3] _147 = {{-1.0000, -1.0000}, {-1.0000, 3.0000}, {3.0000, -1.0000}};

const float2[3] _163 = {{0.0000, 0.0000}, {0.0000, 2.0000}, {2.0000, 0.0000}};

struct PushConstants_std430 : [[Block]] {

  uint idx : [[Offset(0)]];

}

struct DrawCall_std430 {

  float4x4 model : [[ColMajor, MatrixStride(16), Offset(0)]];

  float4 color : [[Offset(64)]];

  int type : [[Offset(80)]];

  float3x4 m : [[ColMajor, MatrixStride(16), Offset(96)]];

  uint tex : [[Offset(160)]];

  uint mat : [[Offset(164)]];

  uint cur_resolve_idx : [[Offset(168)]];

  uint flags : [[Offset(172)]];

}

struct StructuredBuffer : [[Block]] {

  DrawCall_std430[] __member0 : [[Offset(0)]] : [[ArrayStride(176)]];

}

with with row major flag, matrices are column major and with column major, matrices are row major?

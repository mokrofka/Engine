
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
  VK_DrawCall* drawcalls = (VK_DrawCall*)st->vk.indirect_draw_buffer.base;
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
	Node* n = list->first;
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

void sll_stack_push(Node* first, Node* n) {
  n->next = first;
  first = n;
}
void sll_stack_pop(Node* first, Node* n) {
  first = first->next;
}

#define ListFor(it, T, list)                       \
  for (T* it = ContainerOf((list).first, T, node); \
       &it->node != null;                          \
       it = ContainerOf(it->node.next, T, node))

struct NodeIdx {
  u32 next;
  u32 prev;
};

struct ListIdx {
  u32 first;
  u32 last;
  u32 count;
};

template<typename T> void list_push_front(T* p, ListIdx* list, u32 n) {
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
template<typename T> void list_push(T* p, ListIdx* list, u32 n) {
  T* arr = p->data;
  NodeIdx& node = p->data[n].node;
  node.next = list->last;
  node.prev = 0;
  if (list->last) {
    arr[list->last].next = n;
  } else {
    list->first = n;
  }
  list->last = n;
}
template<typename T> void list_remove(T* p, ListIdx* list, u32 n) {
  if (n == 0) return;
  T* arr = p->data;
  NodeIdx& node = p->data[n].node;
  if (node.prev) {
    arr[node.prev].next = node.next;
  } else {
    list->first = node.next;
  }
  if (node.next) {
    arr[node.next].prev = node.prev;
  } else {
    list->last = node.prev;
  }
}
template<typename T> u32 list_pop_front(T* p, ListIdx* list) {
	u32 n = list->first;
	if (n == 0) return 0;
  T* arr = p->data;
  NodeIdx& node = p->data[n].node;
  list->first = node.next;
  if (list->first) {
    arr[list->first].prev = 0;
  } else {
    list->last = null;
  }
  return n;
}
template<typename T> u32 list_pop(T* p, ListIdx* list) {
	u32 n = list->first;
	if (n == 0) return 0;
  T* arr = p->data;
  NodeIdx& node = p->data[n].node;
  list->last = node.prev;
  if (list->last) {
    arr[list->last].next = 0;
  } else {
    list->first = 0;
  }
  return n;
}

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

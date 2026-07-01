#include "containers.h"

////////////////////////////////////////////////////////////////////////
// SparseSet

// SparseSetIndex::SparseSetIndex(Allocator alloc_) { *this = {}; alloc = alloc_; }
// void SparseSetIndex::init(Allocator alloc_) { *this = {}; alloc = alloc_; }
// void SparseSetIndex::deinit() { if (sparse) { mem_free(alloc, sparse); mem_free(alloc, dense); } }
// u32* SparseSetIndex::begin() { return dense; }
// u32* SparseSetIndex::end()   { return dense + count; }

// void SparseSetIndex::add(u32 id) {
//   if (count >= cap) {
//     grow();
//   }
//   if (id >= sparse_count) {
//     grow_max_index(id);
//   }
//   sparse[id] = count;
//   dense[count] = id;
//   ++count;
// }

// void SparseSetIndex::remove(u32 id) {
//   u32 idx_removed = sparse[id];
//   u32 idx_last = count - 1;
//   u32 last_entity = dense[idx_last];
//   sparse[last_entity] = idx_removed;
//   dense[idx_removed] = last_entity;
//   --count;
// }

// void SparseSetIndex::grow() {
//   if (dense) {
//     u32 old_cap = cap;
//     cap *= DEFAULT_RESIZE_FACTOR;
//     dense = mem_realloc_array(alloc, dense, old_cap, cap);
//   }
//   else {
//     cap = DEFAULT_CAPACITY;
//     dense = push_array(alloc, u32, cap);
//     sparse_count = cap;
//     sparse = push_array(alloc, u32, sparse_count);
//   }
// }

// void SparseSetIndex::grow_max_index(u32 id) {
//   u32 modifier = CeilIntDiv(id+1, sparse_count);
//   u32 old_sparse_count = sparse_count;
//   sparse_count *= modifier;
//   sparse = mem_realloc_array(alloc, sparse, old_sparse_count, sparse_count);
// }

// ////////////////////////////////////////////////////////////////////////
// // HandlerArray

// DarrayIndexHandler::DarrayIndexHandler(Allocator alloc_) { *this = {}; alloc = alloc_; }
// void DarrayIndexHandler::init(Allocator alloc_) { *this = {}; alloc = alloc_; }
// void DarrayIndexHandler::deinit() { if (sparse) mem_free(alloc, sparse); }
// u32* DarrayIndexHandler::begin() { return dense; }
// u32* DarrayIndexHandler::end()   { return dense + count; }

// u32 DarrayIndexHandler::add() {
//   if (count >= cap) {
//     grow();
//   }
//   u32 id = count++;
//   sparse[id] = id;
//   dense[id] = id;
//   return id;
// }

// void DarrayIndexHandler::remove(u32 id) {
//   DebugDo(Assert(sparse[id] != INVALID_ID));
//   u32 idx_removed = sparse[id];
//   u32 idx_last = count - 1;
//   u32 last_entity = dense[idx_last];
//   sparse[last_entity] = idx_removed;
//   dense[idx_removed] = last_entity;
//   --count;
//   DebugDo(sparse[id] = INVALID_ID);
// }

// void DarrayIndexHandler::grow() {
//   if (sparse) {
//     u32 cap_old = cap;
//     cap *= DEFAULT_RESIZE_FACTOR;
//     SoA_Field fields[] = {
//       SoA_push_field(&sparse, u32),
//       SoA_push_field(&dense, u32),
//     };
//     mem_realloc_soa(alloc, cap_old, cap, slice(fields));
//   }
//   else {
//     cap = DEFAULT_CAPACITY;
//     SoA_Field fields[] = {
//       SoA_push_field(&sparse, u32),
//       SoA_push_field(&dense, u32),
//     };
//     mem_alloc_soa(alloc, cap, slice(fields));
//   }
// }

////////////////////////////////////////////////////////////////////////
// IdPool

IdPool id_pool_make(Allocator alloc) {
  IdPool res = {
    .alloc = alloc,
  };
  return res;
}
u32 id_pool_alloc(IdPool& p) {
#if BUILD_DEBUG
  if (p.count+1 >= p.cap) {
    if (p.ids) {
      u32 old_cap = p.cap;
      p.cap *= DEFAULT_RESIZE_FACTOR;
      p.ids = mem_realloc_array(p.alloc, p.ids, old_cap, p.cap);
      p.generations = mem_realloc_array_zero(p.alloc, p.generations, old_cap, p.cap);
      for (u32 i = old_cap; i < p.cap; ++i) {
        p.ids[i] = i;
      }
    } else {
      p.cap = DEFAULT_CAPACITY;
      p.ids = push_array(p.alloc, u32, p.cap);
      p.generations = push_array_zero(p.alloc, u32, p.cap);
      for (u32 i = 0; i < p.cap; ++i) {
        p.ids[i] = i;
      }
    }
  }
  u32 result = id_make(p.generations[p.count], p.count++);
  return result;
#else
  if (p.count+1 >= p.cap) {
    if (p.ids) {
      u32 old_cap = p.cap;
      p.cap *= DEFAULT_RESIZE_FACTOR;
      p.ids = mem_realloc_array(p.alloc, p.ids, old_cap, p.cap);
      for (u32 i = old_cap; i < p.cap; ++i) {
        p.ids[i] = i;
      }
    } else {
      p.cap = DEFAULT_CAPACITY;
      p.ids = push_array(p.alloc, u32, p.cap);
      for (u32 i = 0; i < p.cap; ++i) {
        p.ids[i] = i;
      }
    }
  }
  return p.ids[p.count++];
#endif
}
void id_pool_free(IdPool& p, u32 h) {
  u32 idx = id_idx(h);
  Assert(generation_bitmask(p.generations[idx]++) == id_generation(h));
  p.ids[--p.count] = idx;
}
void id_pool_destroy(IdPool& p) {
  if (p.ids) { 
    // mem_free(p.alloc, p.ids);
#if BUILD_DEBUG
    // mem_free(p.alloc, p.generations);
#endif
  }
}
void id_pool_clear(IdPool& p) {
  p.count = 0;
}

///////////////////////////////////
// Radix

u32 sort_i32_key_to_u32(i32 x) {
  return x ^ 0x80000000;
}
u32 sort_f32_key_to_u32(f32 sort_key) {
  u32 res = *(u32*)&sort_key;
  if (res & 0x80000000) {
    res = ~res;
  } else {
    res |= 0x80000000;
  }
  return res;
}

void sort_radix(Allocator alloc, Slice<SortEntry> arr) {
  SortEntry* src = arr.data;
  SortEntry* dst = push_array(alloc, SortEntry, arr.size);
  for (u32 shift = 0; shift < 32; shift += 8) {
    u32 counts[256] = {};

    // 1) histogram
    Loop (i, arr.size) {
      u32 byte = (src[i].sort_key >> shift) & 0xFF;
      ++counts[byte];
    }

    // 2) prefix sum (positions)
    u32 sum = 0;
    LoopElement (i, counts) {
      u32 c = counts[i];
      counts[i] = sum;
      sum += c;
    }

    // 3) distribute (stable)
    Loop (i, arr.size) {
      u32 v = src[i].sort_key;
      u32 byte = (v >> shift) & 0xFF;
      dst[counts[byte]++] = src[i];
    }
    Swap(src, dst);
  }
}

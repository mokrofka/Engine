#pragma once
#include "lib.h"

struct FreelistGpuNode {
  u64 offset;
  u64 size;
  FreelistGpuNode* next;
};

struct FreelistGpu {
  u64 total_size;
  u64 max_entries;
  FreelistGpuNode* head;
  FreelistGpuNode* nodes;
};

FreelistGpu gpu_freelist_create(Arena* arena, u64 size);

u64 gpu_freelist_alloc(FreelistGpu& list, u64 size);

void gpu_freelist_free(FreelistGpu& list, u64 size, u64 offset);

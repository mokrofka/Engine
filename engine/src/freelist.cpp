#include "freelist.h"

FreelistGpuNode* get_node(FreelistGpu& list);
void return_node(FreelistGpu& list, FreelistGpuNode* node);
u64 freelist_free_space(FreelistGpu& list);

FreelistGpu gpu_freelist_create(Arena* arena, u64 size) {
    // Enough space to hold state, plus array for all nodes.
    u64 max_entries = size / 128;  //NOTE: This might have a remainder, but that's ok.

    // The block's layout is head* first, then array of available nodes.
    FreelistGpu result;
    result.nodes = push_array(arena, FreelistGpuNode, max_entries);
    result.max_entries = max_entries;
    result.total_size = size;

    result.head = &result.nodes[0];
    result.head->offset = 0;
    result.head->size = size;
    result.head->next = 0;

    // Invalidate the offset and size for all but the first node. The invalid
    // value will be checked for when seeking a new node from the list.
  for (i32 i = 1; i < max_entries; ++i) {
    result.nodes[i].offset = INVALID_ID;
    result.nodes[i].size = INVALID_ID;
  }
  return result;
}

u64 gpu_freelist_alloc(FreelistGpu& list, u64 size) {
  FreelistGpuNode* node = list.head;
  FreelistGpuNode* previous = 0;
  u64 result;
  while (node) {
    if (node->size == size) {
      // Exact match. Just return the node.
      result = node->offset;
      FreelistGpuNode* node_to_return = 0;
      if (previous) {
        previous->next = node->next;
        node_to_return = node;
      } else {
        // This node is the head of the list. Reassign the head
        // and return the previous head node.
        node_to_return = list.head;
        list.head = node->next;
      }
      return_node(list, node_to_return);
      return result;
    } else if (node->size > size) {
      // Node is larger. Deduct the memory from it and move the offset
      // by that amount.
      result = node->offset;
      node->size -= size;
      node->offset += size;
      return result;
    }

    previous = node;
    node = node->next;
  }

  u64 free_space = freelist_free_space(list);
  Warn("freelist_find_block, no block with enough free space found (requested: %lluB, available: %lluB).", size, free_space);
  return -1;
}

void gpu_freelist_free(FreelistGpu& list, u64 size, u64 offset) {
  FreelistGpuNode* node = list.head;
  FreelistGpuNode* previous = 0;
  if (!node) {
    // Check for the case where the entire thing is allocated.
    // In this case a new node is needed at the head.
    FreelistGpuNode* new_node = get_node(list);
    new_node->offset = offset;
    new_node->size = size;
    new_node->next = 0;
    list.head = new_node;
    return;
  } else {
    while (node) {
      if (node->offset == offset) {
        // Can just be appended to this node.
        node->size += size;

        // Check if this then connects the range between this and the next
        // node, and if so, combine them and return the second node..
        if (node->next && node->next->offset == node->offset + node->size) {
          node->size += node->next->size;
          FreelistGpuNode* next = node->next;
          node->next = node->next->next;
          return_node(list, next);
        }
        return;
      } else if (node->offset > offset) {
        // Iterated beyond the space to be freed. Need a new node.
        FreelistGpuNode* new_node = get_node(list);
        new_node->offset = offset;
        new_node->size = size;

        // If there is a previous node, the new node should be inserted between this and it.
        if (previous) {
          previous->next = new_node;
          new_node->next = node;
        } else {
          // Otherwise, the new node becomes the head.
          new_node->next = node;
          list.head = new_node;
        }

        // Double-check next node to see if it can be joined.
        if (new_node->next && new_node->offset + new_node->size == new_node->next->offset) {
          new_node->size += new_node->next->size;
          FreelistGpuNode* rubbish = new_node->next;
          new_node->next = rubbish->next;
          return_node(list, rubbish);
        }

        // Double-check previous node to see if the new_node can be joined to it.
        if (previous && previous->offset + previous->size == new_node->offset) {
          previous->size += new_node->size;
          FreelistGpuNode* rubbish = new_node;
          previous->next = rubbish->next;
          return_node(list, rubbish);
        }

        return;
      }

      previous = node;
      node = node->next;
    }
  }

  Warn("Unable to find block to be freed. Corruption possible?");
  return;
}

// void freelist_resize(FreelistGpu* list, u64* memory_requirement, void* new_memory, u64 new_size, void** out_old_memory) {
//     if (!list || !memory_requirement || ((internal_state*)list->memory)->total_size > new_size) {
//         return false;
//     }

//     // Enough space to hold state, plus array for all nodes.
//     u64 max_entries = (new_size / sizeof(void*));  //NOTE: This might have a remainder, but that's ok.
//     *memory_requirement = sizeof(internal_state) + (sizeof(freelist_node) * max_entries);
//     if (!new_memory) {
//         return true;
//     }

//     // Assign the old memory pointer so it can be freed.
//     *out_old_memory = list->memory;

//     // Copy over the old state to the new.
//     internal_state* old_state = (internal_state*)list->memory;
//     u64 size_diff = new_size - old_state->total_size;

//     // Setup the new memory
//     list->memory = new_memory;

//     // The block's layout is head* first, then array of available nodes.
//     MemZero(list->memory, *memory_requirement);

//     // Setup the new state.
//     internal_state* state = (internal_state*)list->memory;
//     state->nodes = (freelist_node*)((u8*)list->memory + sizeof(internal_state));
//     state->max_entries = max_entries;
//     state->total_size = new_size;

//     // Invalidate the offset and size for all but the first node. The invalid
//     // value will be checked for when seeking a new node from the list.
//     for (u64 i = 1; i < state->max_entries; ++i) {
//         state->nodes[i].offset = INVALID_ID;
//         state->nodes[i].size = INVALID_ID;
//     }

//     state->head = &state->nodes[0];

//     // Copy over the nodes.
//     freelist_node* new_list_node = state->head;
//     freelist_node* old_node = old_state->head;
//     if (!old_node) {
//         // If there is no head, then the entire list is allocated. In this case,
//         // the head should be set to the difference of the space now available, and
//         // at the end of the list.
//         state->head->offset = old_state->total_size;
//         state->head->size = size_diff;
//         state->head->next = 0;
//     } else {
//         // Iterate the old nodes.
//         while (old_node) {
//             // Get a new node, copy the offset/size, and set next to it.
//             freelist_node* new_node = get_node(list);
//             new_node->offset = old_node->offset;
//             new_node->size = old_node->size;
//             new_node->next = 0;
//             new_list_node->next = new_node;
//             // Move to the next entry.
//             new_list_node = new_list_node->next;

//             if (old_node->next) {
//                 // If there is another node, move on.
//                 old_node = old_node->next;
//             } else {
//                 // Reached the end of the list.
//                 // Check if it extends to the end of the block. If so,
//                 // just append to the size. Otherwise, create a new node and
//                 // attach to it.
//                 if (old_node->offset + old_node->size == old_state->total_size) {
//                     new_node->size += size_diff;
//                 } else {
//                     freelist_node* new_node_end = get_node(list);
//                     new_node_end->offset = old_state->total_size;
//                     new_node_end->size = size_diff;
//                     new_node_end->next = 0;
//                     new_node->next = new_node_end;
//                 }
//                 break;
//             }
//         }
//     }

//     return true;
// }

void freelist_clear(FreelistGpu& list) {
  // Invalidate the offset and size for all but the first node. The invalid
  // value will be checked for when seeking a new node from the list.
  for (u64 i = 1; i < list.max_entries; ++i) {
    list.nodes[i].offset = INVALID_ID;
    list.nodes[i].size = INVALID_ID;
  }

  // Reset the head to occupy the entire thing.
  list.head->offset = 0;
  list.head->size = list.total_size;
  list.head->next = 0;
}

u64 freelist_free_space(FreelistGpu& list) {
  u64 running_total = 0;
  FreelistGpuNode* node = list.head;
  while (node) {
    running_total += node->size;
    node = node->next;
  }

  return running_total;
}

FreelistGpuNode* get_node(FreelistGpu& list) {
  for (u64 i = 1; i < list.max_entries; ++i) {
    if (list.nodes[i].offset == INVALID_ID) {
      return &list.nodes[i];
    }
  }

  // Return nothing if no nodes are available.
  return 0;
}

void return_node(FreelistGpu& list, FreelistGpuNode* node) {
  node->offset = INVALID_ID;
  node->size = INVALID_ID;
  node->next = 0;
}

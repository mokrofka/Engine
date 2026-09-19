#include "lib.h"

////////////////////////////////////////////////////////////////////////
// IdPool
DIdPool id_pool_make(Allocator alloc) {
	DIdPool res = {
		.alloc = alloc,
	};
	return res;
}
u32 id_pool_push(DIdPool& p) {
	if(p.count >= p.cap) {
		if(p.ids) {
			u32 old_cap = p.cap;
			p.cap *= DEFAULT_RESIZE_FACTOR;
			p.ids = mem_realloc_array(p.alloc, p.ids, old_cap, p.cap);
			for(u32 i = old_cap; i < p.cap; i++) {
				p.ids[i] = i;
			}
		} else {
			p.cap = DEFAULT_CAPACITY;
			p.ids = push_array(p.alloc, u32, p.cap);
			for(u32 i = 0; i < p.cap; i++) {
				p.ids[i] = i;
			}
		}
	}
	return p.ids[p.count++];
}
void id_pool_remove(DIdPool& p, u32 id) { p.ids[--p.count] = id; }
void id_pool_destroy(DIdPool& p) { if(p.ids) mem_free(p.alloc, p.ids, p.cap * sizeof(u32)); }
void id_pool_clear(DIdPool& p) { Loop(i, p.cap) p.ids[i] = i; p.count = 0; }

///////////////////////////////////
// Radix
u32 sort_i32_key_to_u32(i32 x) { return x ^ 0x80000000; }
u32 sort_f32_key_to_u32(f32 sort_key) {
	u32 res = bit_cast(u32, sort_key);
	if(res & 0x80000000) {
		res = ~res;
	} else {
		res |= 0x80000000;
	}
	return res;
}

void sort_radix(Allocator alloc, Slice<SortEntry> arr) {
	SortEntry* src = arr.data;
	SortEntry* dst = push_array(alloc, SortEntry, arr.size);
	for(u32 shift = 0; shift < 32; shift += 8) {
		u32 counts[256] = {};

		// 1) histogram
		Loop(i, arr.size) {
			u32 byte = (src[i].sort_key >> shift) & 0xFF;
			counts[byte]++;
		}

		// 2) prefix sum (positions)
		u32 sum = 0;
		LoopArray(i, counts) {
			u32 c = counts[i];
			counts[i] = sum;
			sum += c;
		}

		// 3) distribute (stable)
		Loop(i, arr.size) {
			u32 byte = (src[i].sort_key >> shift) & 0xFF;
			dst[counts[byte]++] = src[i];
		}
		Swap(src, dst);
	}
}

#include "com.h"

#include "gfx.cpp"
#include "render.cpp"
#include "meta.cpp"

#include "generated.h"

GlobalVar GlobalState* st;

R_Vertex cube_vertices[] = {
	// Front face (0, 0, 1)
	{.pos = v3(-1, -1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0, 0)},
	{.pos = v3( 1, -1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1, 0)},
	{.pos = v3( 1,  1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1, 1)},
	{.pos = v3( 1,  1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(-1,  1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0, 1)},
	{.pos = v3(-1, -1,  1), /*0.0f, 0.0f, 1.0f,*/ .uv = v2(0, 0)},

	// Back face (0, 0, -1)
	{.pos = v3( 1, -1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0, 0)},
	{.pos = v3(-1, -1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1, 0)},
	{.pos = v3(-1,  1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(-1,  1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(1, 1)},
	{.pos = v3( 1,  1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0, 1)},
	{.pos = v3( 1, -1, -1), /*0.0f, 0.0f, -1.0f,*/ .uv = v2(0, 0)},

	// Left face (-1, 0, 0)
	{.pos = v3(-1, -1, -1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 0)},
	{.pos = v3(-1, -1,  1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 0)},
	{.pos = v3(-1,  1,  1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(-1,  1,  1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(-1,  1, -1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 1)},
	{.pos = v3(-1, -1, -1),  /*-1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 0)},

	// Right face (1, 0, 0)
	{.pos = v3(1, -1,  1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 0)},
	{.pos = v3(1, -1, -1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 0)},
	{.pos = v3(1,  1, -1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(1,  1, -1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(1,  1,  1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 1)},
	{.pos = v3(1, -1,  1),  /*1.0f, 0.0f, 0.0f,*/ .uv = v2(0, 0)},

	// Bottom face (0, -1, 0)
	{.pos = v3(-1, -1, -1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0, 1)},
	{.pos = v3( 1, -1, -1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3( 1, -1,  1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1, 0)},
	{.pos = v3( 1, -1,  1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(1, 0)},
	{.pos = v3(-1, -1,  1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0, 0)},
	{.pos = v3(-1, -1, -1),  /*0.0f, -1.0f, 0.0f,*/ .uv = v2(0, 1)},

	// Top face (0, 1, 0)
	{.pos = v3(-1,  1,  1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0, 0)},
	{.pos = v3( 1,  1,  1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1, 0)},
	{.pos = v3( 1,  1, -1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3( 1,  1, -1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(1, 1)},
	{.pos = v3(-1,  1, -1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0, 1)},
	{.pos = v3(-1,  1,  1), /*0.0f, 1.0f, 0.0f,*/ .uv = v2(0, 0)},
};

R_Vertex triangle_vertices[] = {
	{.pos = v3( 0.0f,   0.5f, 0), .uv = v2(0.5f, 0), .color = u32_from_rgba(v4(1,0,0,1))},
	{.pos = v3(-0.5f,  -0.5f, 0), .uv = v2(0.0f, 1), .color = u32_from_rgba(v4(0,1,0,1))},
	{.pos = v3( 0.5f,  -0.5f, 0), .uv = v2(1.0f, 1), .color = u32_from_rgba(v4(0,0,1,1))},
};

R_Vertex axis_vertices[] = {
	{.pos = v3(),      .color = u32_from_rgba(v4(1,0,0,1))},
	{.pos = v3(1,0,0), .color = u32_from_rgba(v4(1,0,0,1))},
	{.pos = v3(),      .color = u32_from_rgba(v4(0,1,0,1))},
	{.pos = v3(0,1,0), .color = u32_from_rgba(v4(0,1,0,1))},
	{.pos = v3(),      .color = u32_from_rgba(v4(0,0,1,1))},
	{.pos = v3(0,0,1), .color = u32_from_rgba(v4(0,0,1,1))},
};

global_var String meshes_strs[] = {
#define X(name) [Glue(Mesh_, name)] = Stringify(name),
	MESH_LIST
#undef X
};

global_var String textures_strs[] = {
#define X(name) [Glue(Texture_, name)] = Stringify(name),
	TEXTURE_LIST
#undef X
};

global_var String materials_strs[] = {
#define X(name) [Glue(Material_, name)] = Stringify(name),
	MATERIAL_LIST
#undef X
};

global_var String things_enum_strs[] = {
#define X(name) [name] = Stringify(name),
	THING_LIST
#undef X
};

const u32 TEST_SAMPLES = 100;
global_var i32 test_alignments[] = { 8, 16, 32, 64 };

void test_arena_alloc() {
	Arena arena = arena_make();
	Array<u8*, TEST_SAMPLES> arr = {};
	Array<u32, TEST_SAMPLES> sizes = {};
	Array<u32, TEST_SAMPLES> values = {};
	Loop(i, TEST_SAMPLES) {
		u32 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		arr[i] = push_buffer(arena, size, align);
		sizes[i] = size;
		values[i] = rand_u32(0, 255);
		MemSet(arr[i], values[i], size);
	}
	Loop(i, TEST_SAMPLES) {
		u8* buf = arr[i];
		u32 size = sizes[i];
		u32 value = values[i];
		Loop(j, size) {
			AssertAlways(buf[j] == value);
		}
	}
	arena_destroy(arena);
}

void test_arena_list_alloc() {
	Scratch scratch;
	ArenaList arena(scratch);
	Array<u8*, TEST_SAMPLES> arr = {};
	Array<u32, TEST_SAMPLES> sizes = {};
	Array<u32, TEST_SAMPLES> values = {};

	Loop(i, TEST_SAMPLES) {
		u32 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		arr[i] = push_buffer(arena, size, align);
		sizes[i] = size;
		values[i] = rand_u32(0, 255);
		MemSet(arr[i], values[i], size);
	}
	Loop(i, TEST_SAMPLES) {
		u8* buf = arr[i];
		u32 size = sizes[i];
		u32 value = values[i];
		Loop(j, size) {
			AssertAlways(buf[j] == value);
		}
	}
	alloc_arena_list_clear(arena);

	Loop(i, TEST_SAMPLES) {
		u32 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		arr[i] = push_buffer(arena, size, align);
		sizes[i] = size;
		values[i] = rand_u32(0, 255);
		MemSet(arr[i], values[i], size);
	}
	Loop(i, TEST_SAMPLES) {
		u8* buf = arr[i];
		u32 size = sizes[i];
		u32 value = values[i];
		Loop(j, size) {
			AssertAlways(buf[j] == value);
		}
	}
	alloc_arena_list_clear(arena);
}

void test_alloc() {
	Scratch scratch;
	Alloc alloc = alloc_make(scratch);
	defer{alloc_destroy(alloc);};
	struct AllocCtx {
		u8* data;
		u64 size;
	};
	Array<AllocCtx, TEST_SAMPLES> arr = {};

	Loop(i, TEST_SAMPLES) {
		u64 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		array_push(arr, {mem_alloc(alloc, size, align), size});
		MemZero(arr[i].data, size);
	}
	Array<u32, TEST_SAMPLES> indices = {};
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		mem_free(alloc, arr[indices[i]].data, arr[indices[i]].size);
	}

	array_clear(arr);
	Loop(i, TEST_SAMPLES) {
		u64 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		array_push(arr, {mem_alloc(alloc, size, align), size});
		MemZero(arr[i].data, size);
	}
	array_clear(indices);
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		mem_free(alloc, arr[indices[i]].data, arr[indices[i]].size);
	}
}

void test_gpu_seglist_alloc() {
	Scratch scratch;
	GpuAllocSegList alloc = {.cap = MB(1)};
	alloc = gpu_alloc_seglist_make(scratch);
	Array<GpuMemId, TEST_SAMPLES> arr = {};

	Loop(i, TEST_SAMPLES) {
		u64 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		array_push(arr, gpu_alloc_seglist_alloc(alloc, size, align));
	}
	Array<u32, TEST_SAMPLES> indices = {};
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		gpu_alloc_seglist_free(alloc, arr[indices[i]]);
	}

	array_clear(arr);
	Loop(i, TEST_SAMPLES) {
		u64 size = rand_u32(8, KB(1));
		u64 align = ArrayRand(test_alignments);
		array_push(arr, gpu_alloc_seglist_alloc(alloc, size, align));
	}
	array_clear(indices);
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		gpu_alloc_seglist_free(alloc, arr[indices[i]]);
	}
}

void test_object_pool() {
	Scratch scratch;
	struct A {
		u32 a;
		u32 b;
	};
	var pool = pool_make<A, OpaqueId>(scratch);
	Array<A, TEST_SAMPLES> values = {};
	Array<OpaqueId, TEST_SAMPLES> handlers = {};

	Loop(i, TEST_SAMPLES) {
		values[i].a = rand_u32(0, TEST_SAMPLES);
		values[i].b = rand_u32(0, TEST_SAMPLES);
	};
	Loop(i, TEST_SAMPLES) {
		handlers[i] = pool_push(pool, values[i]);
	}
	Loop(i, TEST_SAMPLES) {
		AssertAlways(MemMatchStruct(&values[i], &pool_get(pool, handlers[i])));
	}
	Array<u32, TEST_SAMPLES> indices = {};
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		pool_remove(pool, handlers[i]);
	}

	array_clear(indices);
	Loop(i, TEST_SAMPLES) {
		values[i].a = rand_u32(0, TEST_SAMPLES);
		values[i].b = rand_u32(0, TEST_SAMPLES);
	};
	Loop(i, TEST_SAMPLES) {
		handlers[i] = pool_push(pool, values[i]);
	}
	Loop(i, TEST_SAMPLES) {
		AssertAlways(MemMatchStruct(&values[i], &pool_get(pool, handlers[i])));
	}
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	Loop(i, TEST_SAMPLES) {
		pool_remove(pool, handlers[i]);
	}
}

void test_object_pool_linklist() {
	Scratch scratch;
	struct A {
		u32 a;
		u32 b;
	};
	PoolIterative<A, TEST_SAMPLES+1, OpaqueId> pool = {};
	Array<A, TEST_SAMPLES> values = {};
	Array<OpaqueId, TEST_SAMPLES> handlers = {};

	Loop(i, TEST_SAMPLES) {
		values[i].a = rand_u32(0, TEST_SAMPLES);
		values[i].b = rand_u32(0, TEST_SAMPLES);
	};
	Loop(i, TEST_SAMPLES) {
		handlers[i] = pool_push(pool, values[i]);
	}
	Loop(i, TEST_SAMPLES) {
		AssertAlways(MemMatchStruct(&values[i], &pool_get(pool, handlers[i])));
	}
	Array<u32, TEST_SAMPLES> indices = {};
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));

	u32 i = 0;
	LoopIter (it, pool_begin(pool)) {
		A elem = *it;
		AssertAlways(elem.a == values[i].a && elem.a == values[i].a);
		i++;
	}
	Loop(i, TEST_SAMPLES) {
		pool_remove(pool, handlers[i]);
	}

	array_clear(indices);
	Loop(i, TEST_SAMPLES) {
		values[i].a = rand_u32(0, TEST_SAMPLES);
		values[i].b = rand_u32(0, TEST_SAMPLES);
	};
	Loop(i, TEST_SAMPLES) {
		handlers[i] = pool_push(pool, values[i]);
	}
	Loop(i, TEST_SAMPLES) {
		AssertAlways(MemMatchStruct(&values[i], &pool_get(pool, handlers[i])));
	}
	Loop(i, TEST_SAMPLES) array_push(indices, (u32)i);
	rand_shuffle(slice(indices));
	i = 0;
	
	LoopIter (it, pool_begin(pool)) {
		A elem = *it;
		AssertAlways(elem.a == values[i].a && elem.a == values[i].a);
		i++;
	}
	Loop(i, TEST_SAMPLES) {
		pool_remove(pool, handlers[i]);
	}
}

// void test_handle_darray() {
// 	Scratch scratch;
// 	struct A {
// 		u32 a;
// 		u32 b;
// 	};
// 	var arr = array_handler_make<A, OpaqueId>(scratch);
// 	Array<A, TEST_SAMPLES> values = {};
// 	Array<OpaqueId, TEST_SAMPLES> handlers = {};

// 	Loop(i, TEST_SAMPLES) {
// 		values[i].a = rand_u32_rng(0, TEST_SAMPLES);
// 		values[i].b = rand_u32_rng(0, TEST_SAMPLES);
// 	};
// 	Loop(i, TEST_SAMPLES) {
// 		handlers[i] = array_handler_push(arr, values[i]);
// 	}
// 	Loop(i, TEST_SAMPLES) {
// 		AssertAlways(MemMatchStruct(&values[i], &array_handler_get(arr, handlers[i])));
// 	}
// 	Array<u32, TEST_SAMPLES> indices = {};
// 	Loop(i, TEST_SAMPLES) array_push(indices, i);
// 	rand_shuffle(slice(indices));
// 	Loop(i, TEST_SAMPLES) {
// 		array_handler_remove(arr, handlers[indices[i]]);
// 	}

// 	array_clear(indices);
// 	Loop(i, TEST_SAMPLES) {
// 		values[i].a = rand_u32_rng(0, TEST_SAMPLES);
// 		values[i].b = rand_u32_rng(0, TEST_SAMPLES);
// 	};
// 	Loop(i, TEST_SAMPLES) {
// 		handlers[i] = array_handler_push(arr, values[i]);
// 	}
// 	Loop(i, TEST_SAMPLES) {
// 		AssertAlways(MemMatchStruct(&values[i], &array_handler_get(arr, handlers[i])));
// 	}
// 	Loop(i, TEST_SAMPLES) array_push(indices, i);
// 	rand_shuffle(slice(indices));
// 	Loop(i, TEST_SAMPLES) {
// 		array_handler_remove(arr, handlers[indices[i]]);
// 	}
// }

void test_id_pool() {
	Scratch scratch;
	{
		DIdPool id_pool = id_pool_make(scratch);
		Array<u32, TEST_SAMPLES> arr = {};
		Loop(i, TEST_SAMPLES) {
			u32 id = id_pool_push(id_pool);
			array_push(arr, id);
		}
		rand_shuffle(slice(arr));
		Loop(i, arr.count) {
			id_pool_remove(id_pool, arr[i]);
		}
		Array<u32, TEST_SAMPLES> new_arr = {};
		Loop(i, arr.count) {
			u32 id = id_pool_push(id_pool);
			array_push(new_arr, id);
		}
		Loop(i, arr.count) {
			b32 exists = false;
			Loop(j, arr.count) {
				if(arr[i] == new_arr[j]) {
					AssertAlways(exists == false);
					exists = true;
				}
			}
			AssertAlways(exists);
		}
		Loop(i, arr.count) {
			id_pool_remove(id_pool, new_arr[i]);
		}
	}

	{
		IdPool<TEST_SAMPLES> static_id_pool = {};
		id_pool_init(static_id_pool);
		Array<u32, TEST_SAMPLES> arr = {};
		Loop(i, TEST_SAMPLES) {
			u32 id = id_pool_push(static_id_pool);
			array_push(arr, id);
		}
		rand_shuffle(slice(arr));
		Loop(i, arr.count) {
			id_pool_remove(static_id_pool, arr[i]);
		}
		Array<u32, TEST_SAMPLES> new_arr = {};
		Loop(i, arr.count) {
			u32 id = id_pool_push(static_id_pool);
			array_push(new_arr, id);
		}
		Loop(i, arr.count) {
			b32 exists = false;
			Loop(j, arr.count) {
				if(arr[i] == new_arr[j]) {
					AssertAlways(exists == false);
					exists = true;
				}
			}
			AssertAlways(exists);
		}
		Loop(i, arr.count) {
			id_pool_remove(static_id_pool, new_arr[i]);
		}
	}
}

void test_profiler_bar() {
	ProfFunc;
	{
		ProfBlock("block in bar");
		os_sleep_ms(1);
	}
	os_sleep_ms(2);
}

void test_profiler_der() {
	ProfFunc;
	os_sleep_ms(10);
}

void test_profiler_die(i32 i) {
	ProfFunc;
	os_sleep_ms(1);
	if(--i) {
		test_profiler_die(i);
	}
}

// intern void test_profile_print_time_elapsed(u64 total_tsc_elapsed, ProfileAnchor anchor) {
//   Scratch scratch;
//   String label_c = push_str_copy(scratch, anchor.label);
//   f64 percent = 100.0 * ((f64)anchor.tsc_elapsed_exclusive / (f64)total_tsc_elapsed);
//   print("  %s[%u64]: %u64 (%.2f%%)", label_c.str, anchor.hit_count, anchor.tsc_elapsed_exclusive, percent);
//   if(anchor.tsc_elapsed_inclusive != anchor.tsc_elapsed_exclusive) {
//     f64 percent_with_children = 100.0 * ((f64)anchor.tsc_elapsed_inclusive / (f64)total_tsc_elapsed);
//     print(", %.2f%% w/children", percent_with_children);
//   }
//   print(")\n");
// }

// intern void test_profiler_print() {
	// u64 cpu_freq = cpu_frequency();
	// u64 total_cpu_elapsed = g_st->profiler.prev_tsc_elapsed;
	// if(cpu_freq) {
	//   print("\nTotal time: %0.4fms (CPU freq %lu)\n", 1000.0 * (f64)total_cpu_elapsed / (f64)cpu_freq, cpu_freq);
	// }
	// Slice<ProfileAnchor> anchors = profiler_get_current_frame_anchors();
	// Loop(anchor_idx, anchors.count) {
	//   ProfileAnchor anchor = anchors[anchor_idx];
	//   if(anchor.tsc_elapsed_inclusive) {
	//     test_profile_print_time_elapsed(total_cpu_elapsed, anchor);
	//   }
	// }
// }

void test_profiler() {
	// profiler_begin();
	test_profiler_bar();
	// test_profiler_bar();
	// test_profiler_der();
	// test_profiler_die(10);
}

void test_sort() {
	Scratch scratch;
	{
		Info("insert");
		i32 arr[] = {0, -4, -3, 7, 3};
		sort_insert(slice(arr), [](var a, var b) { return a < b; });
		LoopArray (i, arr) {
			print("%i ", arr[i]);
		}
		print("\n");
	}
	{
		Info("quick");
		i32 arr[] = {0, -4, -3, 7, 3};
		sort_quick(slice(arr), [](var a, var b) { return a < b;});
		LoopArray (i, arr) {
			print("%i ", arr[i]);
		}
		print("\n");
	}
	{
		Scratch scratch;
		Info("merge");
		i32 arr[] = {0, -4, -3, 7, 3};
		sort_merge(scratch, slice(arr), [](var a, var b) { return a < b;});
		LoopArray (i, arr) {
			print("%i ", arr[i]);
		}
		print("\n");
	}
	{
		Scratch scratch;
		Info("radix");
		i32 arr[] = {0, -4, -3, 7, 3};
		Slice<SortEntry> entries = push_slice(scratch, SortEntry, ArrayCount(arr));
		LoopArray (i, arr) {
			entries[i] = {sort_i32_key_to_u32(arr[i]), (u32)i};
		}
		sort_radix(entries);
		LoopArray (i, arr) {
			print("%i ", arr[entries[i].idx]);
		}
		print("\n");
	}

	u32 counts[] = {32, 64, 128, 512, KB(1), KB(10), KB(100), MB(1)};
	Slice<u32> arrs[ArrayCount(counts)];
	LoopArray (i, counts) {
		arrs[i] = push_slice(scratch, u32, counts[i]);
		Loop(j, counts[i]) {
			arrs[i][j] = rand_i32();
		}
	}
	print("\n/////////////////\n");
	Info("\nQuick test:");
	Loop(i, 2) {
		LoopArray (i, counts) {
			Info("Count %i", counts[i]);
			rand_shuffle(arrs[i]);
			{
				u64 s = cpu_now();
				sort_quick(arrs[i], [](var a, var b) {return a < b;});
				Info("%fms", tsc_to_ms(cpu_now()-s));
			}
		}
	}
	Info("\nmerge test:");
	Loop(i, 2) {
		LoopArray (i, counts) {
			Scratch scratch;
			Info("Count %i", counts[i]);
			rand_shuffle(arrs[i]);
			{
				u64 s = cpu_now();
				sort_merge(scratch, arrs[i], [](var a, var b) {return a < b;});
				Info("%fms", tsc_to_ms(cpu_now()-s));
			}
		}
	}
	Info("\nradix test:");
	Loop(i, 2) {
		LoopArray (i, counts) {
			Scratch scratch;
			Info("Count %i", counts[i]);
			rand_shuffle(arrs[i]);
			Slice<SortEntry> entries = push_slice(scratch, SortEntry, counts[i]);
			Loop(j, counts[i]) {
				entries[j] = {arrs[i][j], (u32)j};
			}
			{
				u64 s = cpu_now();
				Scratch scratch;
				sort_radix(entries);
				Info("%fms", tsc_to_ms(cpu_now()-s));
			}
		}
	}
}

void test_js() {
	Scratch scratch;
	String d = R"(
	{
		"name": "cube",
		"visible": true,
		"pos": [1.0, 2.0, 3.0],
		"material": {
			"color": [1.0, 0.0, 0.0],
			"roughness": 0.5
		}
	}
	)";


	// {
	// 	struct {
	// 		String name;
	// 		b32 visible;
	// 		v3 pos;
	// 		struct {
	// 			v3 color;
	// 			f32 roughness;
	// 		} material;
	// 	} res = {};
	// 	JsParser p = js_parse_make(scratch, d);
	// 	JsObj root = js_parse(&p).obj;
	// 	Loop(i, root.fields.count) {
	// 		var [k, v] = root.fields[i];
	// 		if(str_match(k, "name")) {
	// 			res.name = push_str_copy(scratch, v->str);
	// 		}
	// 		else if(str_match(k, "visible")) {
	// 			res.visible = v->boolean;
	// 		}
	// 		else if(str_match(k, "pos")) {
	// 			Loop(i, v->array.count) {
	// 				res.pos.v[i] = v->array[i]->number;
	// 			}
	// 		}
	// 		else if(str_match(k, "material")) {
	// 			JsObj material = v->obj;
	// 			Loop(i, material.fields.count) {
	// 				var [k, v] = material.fields[i];
	// 				if(str_match(k, "color")) {
	// 					Loop(i, v->array.count) {
	// 						res.material.color.v[i] = v->array[i]->number;
	// 					}
	// 				} else if(str_match(k, "roughness")) {
	// 					res.material.roughness = v->number;
	// 				}
	// 			}
	// 		}
	// 	}
	// }


	{
		JsParser p = js_parse_make(scratch, d);
		JsObj root = js_parse(p).obj;
		String name = js_str(root, "name").val();
		Info("%s", name);
		b32 visible = js_bool(root, "visible").val();
		Info("%u", visible);
		Slice pos = js_array(root, "pos").val();
		Loop(i, pos.count) {
			Info("%f", pos[i]->number);
		}
		{
			JsObj material = js_obj(root, "material").val();
			Slice color = js_array(material, "color").val();
			Loop(i, color.count) {
				Info("%f", color[i]->number);
			}
			f64 roughness = js_number(material, "roughness").val();
			Info("%f", roughness);
		}
	}
}

void test_co1(Coroutine* co, f32 dt) {
	co_begin(co);

	Info("child");
	co_wait(co, 1, dt);
	co_yield(co);
	Info("child waits");
	co_wait(co, 1, dt);

	co_end(co);
}

void test_co(Coroutine* co, f32 dt) {
	f32 wait_time = 1;
	var& a = co_var(co, u32);
	co_begin(co);

	Info("start");
	co_wait(co, wait_time, dt);
	co_yield(co);

	Info("waited 0, %i", a++);
	co_wait(co, wait_time, dt);
	co_call(co, test_co1(co, dt));
	Info("waited 1, %i", a);
	co_wait(co, wait_time, dt);
	Info("waited 2, %i", a);

	co_end(co);
}

void test() {
	ProfFunc;
	// test_sort();
	// os_exit(0);
	test_alloc();
	test_arena_alloc();
	test_arena_list_alloc();
	test_gpu_seglist_alloc();
	test_object_pool();
	test_object_pool_linklist();
	// test_handle_darray();
	test_id_pool();
}

f64 tsc_to_ms(u64 tsc) { return (f64)tsc/cpu_frequency*1000; }
b32 time_on_interval(f64 time, f32 delta, f32 interval, f32 offset) {
	u32 last = (time - offset - delta) / interval;
	u32 next = (time - offset) / interval;
	return last < next;
}
u32 time_on_interval_steps(f64 time, f32 delta, f32 interval, f32 offset) {
	u32 last = (time - offset - delta) / interval;
	u32 next = (time - offset) / interval;
	return next - last;
}
f64 time_next_interval(f64 time, f32 interval, f32 offset) {
	u32 next = (time - offset) / interval + 1;
	return offset + next * interval;
}
f64 time_prev_interval(f64 time, f32 interval, f32 offset) {
	u32 prev = (time - offset) / interval;
	return offset + prev * interval;
}
b32 time_on_time(f64 time, f64 timestamp, f64 dt)                  { return time >= timestamp && (time - dt) < timestamp; }
b32 time_on_between_interval(f64 time, f32 interval, f32 offset)   { return mod(time - offset, interval*2) >= interval; }
f32 time_percent(f64 time, f64 start, f64 duration)                { return (time - start) / duration; }
f32 time_lerp_delta(f32 current, f32 target, f32 rate, f32 delta)  { return target + (current - target) * exp(-rate * delta); }
b32 time_on_frame_interval(u32 frame, u32 n, u32 offset = 0)       { return (frame + offset) % n == 0; }
f64 time_saw_wave(f64 time, f32 interval, f32 offset) {
	f64 t = (time - offset) / interval;
	return t - floor(t);
}
f32 time_sine_wave(f64 time, f32 period) {
	return sin(time / period * 2.0f * PI);
}
f32 time_smooth_wave(f64 time, f32 period) {
	f32 p = time_saw_wave(time, period, 0);
	return 0.5f - 0.5f * cos(p * 2.0f * PI);
}
f32 time_triangle_wave(f64 time, f32 period) {
	f32 p = time_saw_wave(time, period, 0);
	return (1.0f - Abs(2.0f * p - 1.0f));
}
b32 time_pulse_wave(f64 time, f32 period, f32 duration, f32 offset) {
	return mod(time - offset, period) < duration;
}
u32 time_frame(f64 time, f32 frame_duration, u32 frame_count) {
	return u32(time / frame_duration) % frame_count;
}

b32 time_elapsed(f64 time, f32 start, f32 duration) { 
	return time > start && start + duration > time;
}
b32 time_between(f64 time, f32 start, f32 end) {
	return rng1_contains({start, end}, time);
}

b32 time_on_interval(f32 interval, f32 offset)         { return time_on_interval(time_now, time_dt, interval, offset); }
b32 time_on_between_interval(f32 interval, f32 offset) { return time_on_between_interval(time_now, interval, offset); }
f64 time_since(f64 timestamp) { return time_now - timestamp; }
f64 time_until(f64 timestamp) { return timestamp - time_now; }

void imgui_draw_rect(ImDrawList* draw, Rng2 rect, Color col, f32 rounding, ImDrawFlags flags, f32 thickness) {
	draw->AddRect(rect.min, rect.max, col.v, rounding, thickness, flags);
}
void imgui_draw_rect_filled(ImDrawList* draw, Rng2 rect, Color col, f32 rounding, ImDrawFlags flags) {
	draw->AddRectFilled(rect.min, rect.max, col.v);
}
void imgui_draw_line(ImDrawList* draw, v2 p0, v2 p1, Color col, f32 thickness) {
	draw->AddLine(p0, p1, col.v);
}
void imgui_draw_text(ImDrawList* draw, v2 pos, Color col, String fmt, ...) {
	Scratch scratch;
	VaList args;
	va_start(args, fmt);
	String formateted = push_strfv(scratch, fmt, args);
	va_end(args);
	draw->AddText(pos, col.v, (char*)formateted.str, (char*)(formateted.str + formateted.size));
}
void imgui_draw_text(ImDrawList* draw, f32 font_size, v2 pos, Color col, String fmt, ...) {
	Scratch scratch;
	VaList args;
	va_start(args, fmt);
	String formateted = push_strfv(scratch, fmt, args);
	va_end(args);
	draw->AddText(null, font_size, pos, col.v, (char*)formateted.str, (char*)(formateted.str + formateted.size));
}
void imgui_text(String fmt, ...) {
	Scratch scratch;
	VaList args;
	va_start(args, fmt);
	String formateted = push_strfv(scratch, fmt, args);
	va_end(args);
	ImGui::TextUnformatted((char*)formateted.str, (char*)(formateted.str + formateted.size));
}
v2 imgui_calc_text_size(String str) {
	return ImGui::CalcTextSize((char*)str.str, (char*)str.str+str.size);
}

void debug_window_apply_state(DebugWindow& win) {
	if(win.toggle_fullscreen) {
		if(win.fullscreen) {
			win.fullscreen = false;
			ImGui::SetNextWindowPos(win.pos);
			ImGui::SetNextWindowSize(win.size);
			win.flags = NoFlags;
		} else {
			win.fullscreen = true;
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
			win.flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
		}
		win.toggle_fullscreen = false;
	}
}

void debug_window_track_state(DebugWindow& win) {
	if(!win.fullscreen) {
		win.pos = ImGui::GetWindowPos();
		win.size = ImGui::GetWindowSize();
	}
}

void debug_window_toggle_fullscreen(DebugWindow& win) {
	win.toggle_fullscreen = 1;
}

void dev_init() {
	var& g = *st;
	g.prof_win = {
		.win.open = false,
		.root_cam = {.zoom = 1, .zoom2 = v2(1)},
		.frames_cam = {.zoom = 1, .zoom2 = v2(1)},
		.launch_cam = {.zoom = 1, .zoom2 = v2(1)},
		.mem_cam = {.zoom = 1, .zoom2 = v2(1)},
	};
	g.imgui_demo_open = false;
	imgui_init();
	g.prof_win.colors = {
		.work = ColorGreyDark,
		.sleep = ColorGreenUi,
		.job = ColorOrangeUi,
		.async = ColorBlueUi,
		.current_frame = ColorGrey3,
		.frame_ok = ColorGreen,
		.frame_warn = ColorYellow,
		.frame_bad = ColorRed,
		.mem_used = ColorGreenUi,
		.mem_committed = ColorBlueUi,
		.mem_cap = ColorRedUi,
	};
}

void dev_update() {
	Scratch scratch;
	var& g = *st;
	if(g.imgui_demo_open) ImGui::ShowDemoWindow();
	if(os_key_is_pressed(Key_F1)) g.prof_win.win.open = !g.prof_win.win.open;
	if(os_key_is_pressed(Key_F2)) g.imgui_demo_open = !g.imgui_demo_open;
	if(os_key_is_pressed(Key_F3)) g.game_win.open = !g.game_win.open;

	///////////////////////////////////
	// Game
	{
		var& win = st->game_win;
		if(win.open) {
			debug_window_apply_state(win);
			ImGuiWindow("Game") {
				if(ImGui::IsWindowHovered() && os_key_is_pressed(Key_V)) {
					debug_window_toggle_fullscreen(win);
				}
				if(ImGui::Button("save state")) {
					save_game_state();
				}
				if(ImGui::Button("load state")) {
					load_game_state();
				}
				ImGui::DragFloat3("cam pos", g.cam.pos.v);
				ImGui::DragFloat("cam vel fric", &g.cam.vel_friction, 0.1, 0, 100);
			}
		}
	}

	///////////////////////////////////
	// Profiler
	{
		struct  {
			f32 tabs_height = 30;
			f32 top_bar_height = 48;
			f32 thread_name_text_size = 20;
			f32 time_bar_text_size = 15;
			f32 bar_height = 30;
			f32 thread_height_off = 200;
			f32 bar_race_height = 10;
		}l;

		ProfBlock("Profiler");
		var& prof = profiler_st;
		var& prof_win = st->prof_win;

		// Avg, min, max
		u64 tsc_elapsed_sum = 0;
		u64 tsc_elapsed_max = 0;
		u64 tsc_elapsed_min = U64_MAX;
		for(var frame : prof.frames_times) {
			u64 elapsed = frame.tsc_end - frame.tsc_start;
			tsc_elapsed_sum += elapsed;
			tsc_elapsed_max = Max(tsc_elapsed_max, elapsed);
			tsc_elapsed_min = Min(tsc_elapsed_min, elapsed);
		}
		f32 frame_avg_time = tsc_to_ms(tsc_elapsed_sum / ProfRecordHistoryNum);
		f32 frame_max_time = tsc_to_ms(tsc_elapsed_max);
		f32 frame_min_time = tsc_to_ms(tsc_elapsed_min);

		// Prev frame
		ProfFrame prev_frame = prof_get_prev_frame();
		u64 prev_frame_tsc_start = prev_frame.frame_time.tsc_start;
		u64 prev_frame_tsc_end = prev_frame.frame_time.tsc_end;
		u64 prev_frame_tsc_elapsed = prev_frame_tsc_end - prev_frame_tsc_start;
		ProfColors colors = prof_win.colors;
		if(os_key_is_pressed(Key_H)) {
			ImGui::SetNextWindowFocus(); 
		}
		if(prof_win.win.open) {
			debug_window_apply_state(prof_win.win);
			ImGuiWindow("Profiler", prof_win.win.flags) {
				debug_window_track_state(prof_win.win);
				if(os_key_is_pressed(Key_1)) prof_win.active_tab = ProfileTabActive_Root;
				if(os_key_is_pressed(Key_2)) prof_win.active_tab = ProfileTabActive_Frames;
				if(os_key_is_pressed(Key_3)) prof_win.active_tab = ProfileTabActive_Time;
				if(os_key_is_pressed(Key_4)) prof_win.active_tab = ProfileTabActive_LaunchTime;
				if(os_key_is_pressed(Key_5)) prof_win.active_tab = ProfileTabActive_Memory;
				if(os_key_is_pressed(Key_P)) prof.paused = !prof.paused;
				if(ImGui::IsWindowHovered() && os_key_is_pressed(Key_V)) {
					debug_window_toggle_fullscreen(prof_win.win);
				}
				var draw = ImGui::GetWindowDrawList();
				Rng2 win_rect = rng2_make(ImGui::GetCursorScreenPos(), ImGui::GetContentRegionAvail());
				ImGuiPushClipRect2(win_rect);
				ImGuiBeginTabBar("MyTabBar") {
					rng2_cut_top(&win_rect, l.tabs_height);
					var top_bar_rect = rng2_cut_top(&win_rect, l.top_bar_height);
					var info_rect = win_rect;
					ImGui::SetCursorScreenPos(top_bar_rect.min);
					local_persist f32 fps;
					if(time_on_interval(0.1)) fps = 1000 / tsc_to_ms(prev_frame_tsc_elapsed);
					imgui_text("%.1ffps %.1fms CPU %.1fGhz, Recording: %s", fps, tsc_to_ms(prev_frame_tsc_elapsed), (f64)cpu_frequency / Billion(1), prof.paused ? S("off") : S("on"));
					imgui_text("avg %.1fms, max %.1f, min %.1f", frame_avg_time, frame_max_time, frame_min_time);

					///////////////////////////////////
					// Tab mouse click
					var tab_mouse_click_handle = [&](ProfTabActive tab) {
						Rng2 tab_rect = Rng2(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
						if(os_mouse_is_button_pressed(MouseButton_Left)) {
							if(rng2_contains(tab_rect, os_mouse_pos())) switch(tab) {
								case ProfileTabActive_Root: prof_win.active_tab = ProfileTabActive_Root; break;
								case ProfileTabActive_Frames: prof_win.active_tab = ProfileTabActive_Frames; break;
								case ProfileTabActive_Time: prof_win.active_tab = ProfileTabActive_Time; break;
								case ProfileTabActive_LaunchTime: prof_win.active_tab = ProfileTabActive_LaunchTime; break;
								case ProfileTabActive_Memory: prof_win.active_tab = ProfileTabActive_Memory; break;
							}
						}
					};

					var draw_thread_names = [&](Rng2 rect, Camera2 cam) {
						f32 thread_height = 200;
						f32 thread_height_offset = 0;
						f32 text_height_off = -30;
						String str = push_strf(scratch, "Main thread");
						v2 text_pos = {0, text_height_off};
						text_pos = {rect.min.x, world_to_screen2(cam, text_pos, rect.min).y};
						imgui_draw_text(draw, l.thread_name_text_size, text_pos, ColorWhite, str);
						Loop(i, Thread_NumWorkers) {
							thread_height_offset += thread_height;
							String str = push_strf(scratch, "Worker %i", i);
							v2 text_pos = {0, thread_height_offset + text_height_off};
							text_pos = {rect.min.x, world_to_screen2(cam, text_pos, rect.min).y};
							imgui_draw_text(draw, l.thread_name_text_size, text_pos, ColorWhite, str);
						}
					};

					var draw_frame_graph = [&](Rng2 info_rect, Slice<Slice<ProfAnchor>> slices, ProfFrameTime time, f32 width_off, Camera2 cam, b32 wrap = false) {
						ProfFunc;
						Scratch scratch;
						Loop(i, slices.count) {
							var anchors = slices[i];
							u64 tsc_elapsed = time.tsc_end - time.tsc_start;
							for(var anchor : anchors) {

								// Handle async anchors
								if(wrap) {
									if(anchor.tsc_end == 0) {
										anchor.tsc_elapsed_excl = time.tsc_end - anchor.tsc_start;
										anchor.tsc_end = time.tsc_end;
									}
									if(anchor.tsc_start < time.tsc_start) {
										anchor.tsc_start = time.tsc_start;
									}
								} else if(anchor.tsc_end == 0) continue;
								f64 width = remap_f64(anchor.tsc_end-anchor.tsc_start, tsc_elapsed, rng2_width(info_rect));
								f64 start_off = remap_f64(anchor.tsc_start, time.tsc_start, time.tsc_end, 0, rng2_width(info_rect));
								Rng2 item_rect = rng2_make(v2(start_off, anchor.depth*l.bar_height), v2(width, l.bar_height));

								// Offset
								item_rect = rng2_shift(item_rect, v2(width_off, l.thread_height_off * i));

								// To Screen
								item_rect.min = world_to_screen2(cam, item_rect.min, info_rect.min);
								item_rect.max = world_to_screen2(cam, item_rect.max, info_rect.min);

								///////////////////////////////////
								// Draw
								Color color = {};
								String str = {};
								switch(anchor.type) {
									case ProfType_Default: {
										color = colors.work;
										str = "work";
									}break;
									case ProfType_Sleep: {
										color = colors.sleep;
										str = "sleep";
									}break;
									case ProfType_Worker: {
										color = colors.job;
										str = "job";
									}break;
									case ProfType_Async: {
										color = colors.async;
										str = "async";
									}break;
								}
								imgui_draw_rect_filled(draw, item_rect, color);
								imgui_draw_rect(draw, item_rect, ColorGreyLight);
								if(rng2_contains(item_rect, os_mouse_pos())) ImGuiBeginToolTip() {
									imgui_text("Label: %s", anchor.label);
									imgui_text("Percent: %f%%", rng2_dim(item_rect).x / rng2_dim(info_rect).x * 100);
									imgui_text("Time: %fms", tsc_to_ms(anchor.tsc_end - anchor.tsc_start));
									imgui_text("Time exclusive: %fms", tsc_to_ms(anchor.tsc_elapsed_excl));
									imgui_text("Type: %s", str);
								}

								// Text
								str = push_strf(scratch, "%s %.3f", anchor.label, tsc_to_ms(anchor.tsc_end - anchor.tsc_start));
								v2 text_size = imgui_calc_text_size(str);
								if(rng2_dim(item_rect).x < 30.1 || cam.zoom2.y < 0.3) {
									continue;
								}
								v2 text_pos = {};
								if(text_size.x > rng2_dim(item_rect).x) {
									text_pos.x = item_rect.min.x;
									text_pos.y = rng1_align_center(rng2_rng_y(item_rect), text_size.y);
								} else {
									text_pos = rng2_align_dim_at_center(item_rect, text_size).min;
								}
								ImGuiPushClipRect2(item_rect);
								imgui_draw_text(draw, l.time_bar_text_size, text_pos, ColorWhite, str);
							}
						}
					};

					///////////////////////////////////
					// Root
					var active_tab = prof_win.active_tab;
					ImGuiBeginTabItem("root", active_tab == ProfileTabActive_Root ? ImGuiTabItemFlags_SetSelected : 0) {
						if(ImGui::IsWindowHovered()) {
							camera_zoom_and_pan(prof_win.root_cam, info_rect.min);
						}
						draw_thread_names(info_rect, prof_win.root_cam);
						Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
						u32 idx = (current_frame-1) % ArrayCount(prof.frames_times);
						local_persist i32 frequency = 1;
						ImGui::SetCursorScreenPos(rng2_subrng_x01(top_bar_rect, {0.5,1}).min);
						ImGui::PushItemWidth((50));
						ImGui::DragInt("show freq", &frequency, 0.1f, 1, ProfRecordHistoryNum); ImGui::SameLine();
						ImGui::DragInt("fps max", (i32*)&g.fps, 1.0f, 10, 2000); ImGui::SameLine();
						ImGui::SliderFloat("scale", &st->r.scale, 0.01, 1);
						ImGui::PopItemWidth();
						idx /= ProfRecordHistoryNum/frequency;

						// Little bars
						Rng2 race_bar_rect = rng2_cut_top(&info_rect, l.bar_race_height);
						Loop(i, ArrayCount(prof.frames_times)) {
							ProfFrameTime frame_time = prof.frames_times[i];
							f32 ms = tsc_to_ms(frame_time.tsc_end - frame_time.tsc_start);
							f32 width = rng2_width(race_bar_rect) / ArrayCount(prof.frames_times);
							v2 max = race_bar_rect.min + v2(width * i + width, l.bar_race_height);
							v2 s = {width, ms};
							var rect = rng2_make(max - s, s);
							if(rng2_contains(rect, os_mouse_pos())) {
								if(os_mouse_is_button_pressed(MouseButton_Left)) {
									prof_win.frames_cam.pos = v2(rng2_width(race_bar_rect) * i, 0);
									prof_win.frames_cam.zoom2 = v2(1);
									prof_win.frames_cam.zoom = 1;
								}
							}
							Color color = {};
							if(i == current_frame % ArrayCount(prof.frames_times)) {
								color = colors.current_frame;
							} else {
								color = colors.frame_ok;
								if(rng1_contains({17, 21}, ms)) {
									color = colors.frame_warn;
								} else if(ms >= 21) {
									color = colors.frame_bad;
								}
							}
							imgui_draw_rect_filled(draw, rect, color);
							Color c = ColorGrey0;
							c.a = 0.3f * 255;
							imgui_draw_rect(draw, rect, c);
						}

						LoopArray(i, slices) slices[i] = slice(prof.prof_threads[i].recorded_anchors[idx]);
						draw_frame_graph(info_rect, slice(slices), prof.frames_times[idx], 0, prof_win.root_cam, true);

						// Gpu flame graph
						{
							local_persist Gfx_ProfileFrame pf;
							if(time_on_interval(0.5)) pf = st->gfx.cur_prof_frame;
							var& zones = pf.zones;
							f64 elapsed = pf.end_ms-pf.start_ms;
							f64 time_start = pf.start_ms;
							f64 time_end = pf.end_ms;
							f32 gpu_offset_height = -100;

							Rng2 item_rect = rng2_make(v2(0), v2(rng2_width(info_rect), l.bar_height));
							item_rect = rng2_shift(item_rect, v2(0, gpu_offset_height - l.bar_height));
							item_rect.min = world_to_screen2(prof_win.root_cam, item_rect.min, info_rect.min);
							item_rect.max = world_to_screen2(prof_win.root_cam, item_rect.max, info_rect.min);
							imgui_draw_rect_filled(draw, item_rect, colors.work);
							imgui_draw_rect(draw, item_rect, ColorGreyLight);

							{
								String str = push_strf(scratch, "gpu frame %.3f", elapsed);
								v2 text_size = imgui_calc_text_size(str);
								v2 text_pos = rng2_align_dim_at_center(item_rect, text_size).min;
								ImGuiPushClipRect2(item_rect);
								imgui_draw_text(draw, l.time_bar_text_size, text_pos, ColorWhite, str);
							}

							for(var zone : zones) {
								f64 width = remap_f64(zone.end - zone.start, elapsed, rng2_width(info_rect));
								f64 start_off = remap_f64(zone.start, time_start, time_end, 0, rng2_width(info_rect));
								Rng2 item_rect = rng2_make(v2(start_off, zone.depth*l.bar_height), v2(width, l.bar_height));

								item_rect = rng2_shift(item_rect, v2(0, gpu_offset_height));

								item_rect.min = world_to_screen2(prof_win.root_cam, item_rect.min, info_rect.min);
								item_rect.max = world_to_screen2(prof_win.root_cam, item_rect.max, info_rect.min);

								imgui_draw_rect_filled(draw, item_rect, colors.work);
								imgui_draw_rect(draw, item_rect, ColorGreyLight);
								
								if(rng2_contains(item_rect, os_mouse_pos())) ImGuiBeginToolTip() {
									imgui_text("vert count: %u, frag count: %u", zone.vert_invocations, zone.frag_invocations);
								}

								// Text
								String str = push_strf(scratch, "%s %.3f", zone.name, zone.end - zone.start);
								v2 text_size = imgui_calc_text_size(str);
								if(rng2_dim(item_rect).x < 30.1 || prof_win.root_cam.zoom2.y < 0.3) {
									continue;
								}
								v2 text_pos = {};
								if(text_size.x > rng2_dim(item_rect).x) {
									text_pos.x = item_rect.min.x;
									text_pos.y = rng1_align_center(rng2_rng_y(item_rect), text_size.y);
								} else {
									text_pos = rng2_align_dim_at_center(item_rect, text_size).min;
								}
								ImGuiPushClipRect2(item_rect);
								imgui_draw_text(draw, l.time_bar_text_size, text_pos, ColorWhite, str);
							}
						}
					}tab_mouse_click_handle(ProfileTabActive_Root);
					ImGuiBeginTabItem("frames", active_tab == ProfileTabActive_Frames ? ImGuiTabItemFlags_SetSelected : 0) {
						if(ImGui::IsWindowHovered()) {
							camera_zoom_and_pan(prof_win.frames_cam, info_rect.min);
						}
						// Little bars
						Rng2 race_bar_rect = rng2_cut_top(&info_rect, l.bar_race_height);
						Loop(i, ArrayCount(prof.frames_times)) {
							ProfFrameTime frame_time = prof.frames_times[i];
							f32 ms = tsc_to_ms(frame_time.tsc_end - frame_time.tsc_start);
							f32 width = rng2_width(race_bar_rect) / ArrayCount(prof.frames_times);
							v2 max = race_bar_rect.min + v2(width * i + width, l.bar_race_height);
							v2 s = {width, ms};
							var rect = rng2_make(max - s, s);
							if(rng2_contains(rect, os_mouse_pos())) {
								if(os_mouse_is_button_pressed(MouseButton_Left)) {
									prof_win.frames_cam.pos = v2(rng2_width(info_rect) * i, 0);
									prof_win.frames_cam.zoom2 = v2(1);
									prof_win.frames_cam.zoom = 1;
								}
							}
							Color color = {};
							if(i == current_frame % ArrayCount(prof.frames_times)) {
								color = colors.current_frame;
							} else {
								color = colors.frame_ok;
								if(rng1_contains({17, 21}, ms)) {
									color = colors.frame_warn;
								} else if(ms >= 21) {
									color = colors.frame_bad;
								}
							}
							imgui_draw_rect_filled(draw, rect, color);
							var c = ColorGrey0;
							c.a = 0.3f * 255;
							imgui_draw_rect(draw, rect, c.a);
						}
						draw_thread_names(info_rect, prof_win.frames_cam);
						// Draw graph per thread
						LoopArray(j, prof.frames_times) {
							Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
							LoopArray(i, prof.prof_threads) slices[i] = slice(prof.prof_threads[i].recorded_anchors[j]);
							draw_frame_graph(info_rect, slice(slices), prof.frames_times[j], j * rng2_width(info_rect), prof_win.frames_cam);
						}
					}tab_mouse_click_handle(ProfileTabActive_Frames);

					///////////////////////////////////
					// Time
					ImGuiBeginTabItem("time", active_tab == ProfileTabActive_Time ? ImGuiTabItemFlags_SetSelected : 0) {
						var sort_entries = push_slice(scratch, SortEntry64, prev_frame.anchors.count);
						Loop(i, sort_entries.count) {
							var& entry = sort_entries[i];
							entry.idx = i;
							entry.sort_key = prev_frame.anchors[i].tsc_elapsed_excl;
						}
						sort_radix64_msd(sort_entries);
						Loop(i, prev_frame.anchors.count) {
							ProfAnchor anchor = prev_frame.anchors[sort_entries[i].idx];
							f32 width = remap_f64(anchor.tsc_elapsed_excl, prev_frame_tsc_elapsed, rng2_width(info_rect));
							f32 height = 30;
							var rect = rng2_make(info_rect.min+v2(0,height*i), v2(width,height));
							imgui_draw_rect_filled(draw, rect, ColorGreyDark);
							imgui_draw_rect(draw, rect, ColorGreyLight);
							v2 text_size = imgui_calc_text_size(anchor.label);
							v2 pos = {
								rect.min.x,
								rng1_align_center(rng2_rng_y(rect), text_size.y),
							};
							imgui_draw_text(draw, pos, ColorWhite, anchor.label);
							String ms_str = push_strf(scratch, "%.3fms", tsc_to_ms(anchor.tsc_elapsed_excl));
							pos = {
								rng2_width(info_rect) * 0.92f,
								info_rect.min.y + height * i,
							},
							imgui_draw_text(draw, pos, ColorWhite, ms_str);
						}
					}tab_mouse_click_handle(ProfileTabActive_Time);

					///////////////////////////////////
					// Launch
					ImGuiBeginTabItem("launch", active_tab == ProfileTabActive_LaunchTime ? ImGuiTabItemFlags_SetSelected : 0) {
						if(ImGui::IsWindowHovered()) {
							camera_zoom_and_pan(prof_win.launch_cam, info_rect.min);
						}
						draw_thread_names(info_rect, prof_win.launch_cam);
						Slice<ProfAnchor> slices[ArrayCount(prof.prof_threads)] = {};
						LoopArray(i, prof.prof_threads) slices[i] = slice(prof.prof_threads[i].launch_anchors);
						draw_frame_graph(info_rect, slice(slices), prof.launch_time, 0, prof_win.launch_cam, true);
					}tab_mouse_click_handle(ProfileTabActive_LaunchTime);

					///////////////////////////////////
					// Memory
					ImGuiBeginTabItem("memory", active_tab == ProfileTabActive_Memory ? ImGuiTabItemFlags_SetSelected : 0) {
						if(ImGui::IsWindowHovered()) {
							camera_zoom_and_pan(prof_win.mem_cam, info_rect.min, true);
						}
						var mem_info_top = rng2_cut_top(&info_rect, 20);
						AllocatorInfoList infos = mem_track_info();
						var sort_entries = push_slice(scratch, SortEntry64, infos.count);
						u64 mem_usage = 0;
						u32 idx = 0;
						LoopNode(it, infos.first) {
							var& entry = sort_entries[idx++];
							entry.p = it;
							entry.sort_key = it->pos;
							mem_usage += it->cap;
						}
						sort_radix64_msd(sort_entries);
						u64 max_pos = ((AllocatorInfo*)sort_entries[0].p)->pos;
						f32 mem_levels[] = {KB(1), KB(10), KB(100), MB(1), MB(10), MB(100), GB(1)};
						DArray<AllocatorInfo*> mem_arrays[ArrayCount(mem_levels)] = {};
						for(var& arr : mem_arrays) arr = array_make<AllocatorInfo*>(scratch);
						u32 max_mem_level = 0;
						LoopArray(i, mem_levels) {
							if(max_pos < mem_levels[i]) { 
								max_mem_level = i;
								break;
							}
						}
						Loop(i, sort_entries.count) {
							var info = (AllocatorInfo*)sort_entries[i].p;
							u32 max_level = 0;
							LoopArray(j, mem_levels) {
								if(info->pos < mem_levels[j]) {
									max_level = j;
									break;
								}
							}
							array_push(mem_arrays[max_level], info);
						}
						f32 height_off = 0;
						LoopReverse(mem_idx, max_mem_level+1) {
							if(mem_arrays[mem_idx].count) {
								v2 rect_size = {50, 25};
								// var pos = rng1_align_center(rng2_rng_x(info_rect), rect_size.x);
								var pos = rng1_align_center(Rng1(0, rng2_width(info_rect)), rect_size.x);
								// var rect = rng2_make(v2(pos, info_rect.min.y) + v2(0,height_off), rect_size);
								var rect = rng2_make(v2(pos, 0) + v2(0,height_off), rect_size);
								rect = world_to_screen(prof_win.mem_cam, rect, info_rect.min);

								// rect = world_to_screen(prof_win.mem_cam, rect, info_rect.min);

								imgui_draw_rect_filled(draw, rect, ColorBlueUi);
								imgui_draw_rect(draw, rect, ColorGreyLight);
								MemFormatSize mem_fmt = mem_format_size(mem_levels[mem_idx]);
								String str = push_strf(scratch, "%.0f%s", mem_fmt.size, mem_fmt.format);
								v2 text_size = imgui_calc_text_size(str);
								Rng2 text_rect = rng2_align_dim_at_center(rect, text_size);
								imgui_draw_text(draw, text_rect.min, ColorWhite, str);
								height_off += 40;
							}
							Loop(parent_idx, mem_arrays[mem_idx].count) {
								var info = mem_arrays[mem_idx][parent_idx];
								f32 excl_width = remap(info->pos - info->children_size, mem_levels[mem_idx], rng2_width(info_rect));
								f32 incl_width = remap(info->pos, mem_levels[mem_idx], rng2_width(info_rect));
								f32 cap_width = remap(info->cap, mem_levels[mem_idx], rng2_width(info_rect));
								f32 height = 30;
								// var r = rng2_make(info_rect.min + v2(0, height_off), {rng2_width(info_rect), height});
								var r = rng2_make(v2(0, height_off), {rng2_width(info_rect), height});
								var excl_rect = rng2_cut_left(&r, excl_width);
								var incl_rect = rng2_cut_left(&r, incl_width-excl_width);
								var cap_rect =  rng2_cut_left(&r, cap_width-incl_width);
								excl_rect = world_to_screen(prof_win.mem_cam, excl_rect, info_rect.min);
								incl_rect = world_to_screen(prof_win.mem_cam, incl_rect, info_rect.min);
								cap_rect = world_to_screen(prof_win.mem_cam, cap_rect, info_rect.min);
								height_off += 30;
								imgui_draw_rect_filled(draw, excl_rect, ColorGreenUi);
								imgui_draw_rect(draw, excl_rect, ColorGreyLight);
								imgui_draw_rect_filled(draw, incl_rect, ColorBlueUi);
								imgui_draw_rect(draw, incl_rect, ColorGreyLight);
								imgui_draw_rect_filled(draw, cap_rect, ColorRedUi);
								imgui_draw_rect(draw, cap_rect, ColorGreyLight);
								MemFormatSize incl_mem = mem_format_size(info->pos);
								MemFormatSize excl_mem = mem_format_size(info->pos - info->children_size);
								MemFormatSize cmt_mem = mem_format_size(info->cap);
								if(rng2_contains(rng2_union(excl_rect, cap_rect), os_mouse_pos())) ImGuiBeginToolTip() {
									imgui_text(push_strf(scratch, "inclusive: %.2f%s", incl_mem.size, incl_mem.format));
									imgui_text(push_strf(scratch, "exclusive: %.2f%s", excl_mem.size, excl_mem.format));
									imgui_text(push_strf(scratch, "file: %s, line: %i", String(info->file), info->line));
								}
								String name_str = push_strf(scratch, "%s", String(info->name));
								String mem_str = push_strf(scratch, "%.2f%s pos, %.2f%s cap", incl_mem.size, incl_mem.format, cmt_mem.size, cmt_mem.format);
								imgui_draw_text(draw, excl_rect.min, ColorWhite, name_str);
								imgui_draw_text(draw, excl_rect.min + v2(200, 0), ColorWhite, mem_str);
								var sort_entries = push_slice(scratch, SortEntry64, info->child_count);
								u32 idx = 0;
								LoopNode(it, info->first) {
									var& entry = sort_entries[idx++];
									entry.p = it;
									entry.sort_key = it->pos;
								}
								sort_radix64_msd(sort_entries);
								Loop(child_idx, sort_entries.count) {
									var info = (AllocatorInfo*)sort_entries[child_idx].p;
									f32 c_width = remap(info->pos, mem_levels[mem_idx], rng2_width(info_rect));
									f32 c_cap_width = remap(info->cap, mem_levels[mem_idx], rng2_width(info_rect));
									f32 height = 30;
									var r = rng2_make(v2(0, height_off), {rng2_width(info_rect), height});
									var rect = rng2_cut_left(&r, c_width);
									var cap_rect =  rng2_cut_left(&r, c_cap_width - c_width);
									r = world_to_screen(prof_win.mem_cam, r, info_rect.min);
									rect = world_to_screen(prof_win.mem_cam, rect, info_rect.min);
									cap_rect = world_to_screen(prof_win.mem_cam, cap_rect, info_rect.min);
									height_off += 30;
									imgui_draw_rect_filled(draw, rect, ColorGreyDark);
									imgui_draw_rect(draw, rect, ColorGreyLight);
									imgui_draw_rect_filled(draw, cap_rect, ColorRedUi);
									imgui_draw_rect(draw, cap_rect, ColorGreyLight);
									MemFormatSize mem = mem_format_size(info->pos);
									MemFormatSize cap_mem = mem_format_size(info->cap);
									if(rng2_contains(rng2_union(rect, cap_rect), os_mouse_pos())) ImGuiBeginToolTip() {
										imgui_text(push_strf(scratch, "pos: %.2f%s", mem.size, mem.format));
										imgui_text(push_strf(scratch, "file: %s, line: %i", String(info->file), info->line));
									}
									String name_str = push_strf(scratch, "%s", String(info->name));
									String mem_str = push_strf(scratch, "%.2f%s pos, %.2f%s cap", mem.size, mem.format, cap_mem.size, cap_mem.format);
									imgui_draw_text(draw, rect.min, ColorWhite, name_str);
									imgui_draw_text(draw, rect.min + v2(200, 0), ColorWhite, mem_str);
								}
								if(sort_entries.count) height_off += 10;
								height_off+=2;
							}
						}

						// overall usage
						ImGui::SetCursorScreenPos(mem_info_top.min);
						MemFormatSize mem_fmt = mem_format_size(mem_usage);
						MemFormatSize os_commited_fmt = mem_format_size(os_commited_size());
						MemFormatSize os_address_reserved_fmt = mem_format_size(os_reserved_size());
						imgui_text("os commited: %.2f%s, address reserved %.2f%s, mem usage: %.2f%s",
							os_commited_fmt.size, os_commited_fmt.format,
							os_address_reserved_fmt.size, os_address_reserved_fmt.format,
							mem_fmt.size, mem_fmt.format);

						// Gpu memory
						{
							var& gfx = st->gfx;
							u64 cpu_cap = st->gfx.cpu_mem.cap;
							u64 gpu_cap = st->gfx.gpu_mem.cap;
							u64 images_size = st->gfx.images_mem_usage;
							u64 target_images_size = st->gfx.target_images_mem_usage;
							u64 max = Max(Max3(cpu_cap, gpu_cap, images_size), target_images_size);
							struct Info {
								u64 pos;
								u64 cap;
								String name;
							} infos[] = {
								{gfx.cpu_buf.pos, gfx.cpu_buf.cap, "cpu"},
								{gfx.gpu_buf.pos, gfx.gpu_buf.cap, "gpu"},
								{gfx.stage_buffer.pos, gfx.stage_buffer.size, "stage"},
								{gfx.images_mem_usage, gfx.images_mem_usage, "images"},
								{gfx.target_images_mem_usage, gfx.target_images_mem_usage, "targets images"},
							};

							f32 height_off = -150;
							LoopArray(i, infos) {
								f32 height = 30;
								f32 pos_width = remap(infos[i].pos, max, rng2_width(info_rect));
								f32 cap_width = remap(infos[i].cap, max, rng2_width(info_rect));
	
								var r = rng2_make(v2(), v2(rng2_width(info_rect), height));
								var pos_r = rng2_cut_left(&r, pos_width);
								var cap_r = rng2_cut_left(&r, cap_width - pos_width);
	
								pos_r = rng2_shift(pos_r, v2(0, height_off));
								cap_r = rng2_shift(cap_r, v2(0, height_off));
								height_off += height;
	
								pos_r = world_to_screen(prof_win.mem_cam, pos_r, info_rect.min);
								cap_r = world_to_screen(prof_win.mem_cam, cap_r, info_rect.min);
	
								imgui_draw_rect_filled(draw, pos_r, ColorGreenUi);
								imgui_draw_rect(draw, pos_r, ColorGreyLight);
								imgui_draw_rect_filled(draw, cap_r, ColorRedUi);
								imgui_draw_rect(draw, cap_r, ColorGreyLight);
	
								var pos_fmt = mem_format_size(infos[i].pos);
								var cap_fmt = mem_format_size(infos[i].cap);
								imgui_draw_text(draw, pos_r.min, ColorWhite, push_strf(scratch, "%s pos: %.2f%s cap: %.2f%s", infos[i].name, pos_fmt.size, pos_fmt.format, cap_fmt.size, cap_fmt.format));
							}
						}
					}tab_mouse_click_handle(ProfileTabActive_Memory);
				}
			}
		}
	}
}

R_MeshDesc load_obj(Allocator arena, String name) {
	Scratch scratch(arena);
	var positions = array_make<v3>(scratch);
	var normals = array_make<v3>(scratch);
	var uvs = array_make<v2>(scratch);
	var indexes = array_make<v3u>(scratch);

	// String str = R"(
	//   v  -4.4   14 4.1
	//   v   1.4  -14 4.1
	//   vt -4.4   14
	//   vt  1.4  -14
	//   vn -4.4   14 4.1
	//   vn  1.4  -14 4.1
	//   f 10/4/1 1/2/3 11/22/33
	//   f 20/2/2 91/42/13 141/22/33
	// )";

	struct WordLexer {
		String str;
		u32 cursor;
	};
	var word_lexer_next = [](WordLexer& l) {
		while(l.cursor < l.str.size && char_is_ws(l.str.str[l.cursor])) {
			l.cursor++;
		}
		u32 word_base = l.cursor;
		while(l.cursor < l.str.size && !char_is_ws(l.str.str[l.cursor])) {
			l.cursor++;
		}
		String res = {l.str.str + word_base, l.cursor - word_base};
		return res;
	};
	WordLexer l = {os_file_path_read_all_str(scratch, name)};
	for(String word = {}; (word = word_lexer_next(l)).size;) {
		// Info("%s", word);
		if(word.str[0] == 'v' && word.str[1] == ' ') {
			v3 pos = {
			 f32_from_str(word_lexer_next(l)),
			 f32_from_str(word_lexer_next(l)),
			 f32_from_str(word_lexer_next(l)),
			};
			array_push(positions, pos);
			// Info("%f %f %f", pos.x, pos.y, pos.z);
		} else if(word.str[0] == 'v' && word.str[1] == 'n') {
			v3 norm = {
				f32_from_str(word_lexer_next(l)),
				f32_from_str(word_lexer_next(l)),
				f32_from_str(word_lexer_next(l)),
			};
			array_push(normals, norm);
			// Info("%f %f %f", norm.x, norm.y, norm.z);
		} else if(word.str[0] == 'v' && word.str[1] == 't') {
			v2 uv = {
				f32_from_str(word_lexer_next(l)),
				f32_from_str(word_lexer_next(l)),
			};
			array_push(uvs, uv);
			// Info("%f %f", uv.x, uv.y);
		} else if(word.str[0] == 'f' && word.str[1] == ' ') {
			Loop(i, 3) {
				v3u raw = {};
				String word = word_lexer_next(l);
				u32 cursor = 0;
				String r = {};
				Loop(i, 3) {
					u32 num_base = cursor;
					while(cursor < word.size && char_is_digit(word.str[cursor])) {
						cursor++;
					}
					r = {word.str+num_base, cursor - num_base};
					raw.v[i] = u64_from_str(r) - 1;
					cursor++;
				}
				// Info("%u %u %u", raw.x, raw.y, raw.z);
				v3u v = {raw.x, raw.z, raw.y};
				array_push(indexes, v);
			}
		}
	}

	var vertices = array_make<R_Vertex>(arena);
	var final_indices = array_make<u16>(arena);
	var map = map_make<u16>(scratch);
	Loop(i, indexes.count) {
		v3u idx = indexes[i];
		u64 h = hash_bytes(&idx, sizeof(v3u));
		var[value, ok] = map_get(map, h);
		if(ok) {
			array_push(final_indices, value);
		} else {
			R_Vertex v = {
			 positions[idx.x],
			 normals[idx.y],
			 uvs[idx.z],
			};
			u16 new_index = vertices.count;
			array_push(vertices, v);
			array_push(final_indices, new_index);
			map_set(map, h, new_index);
		} 
	}
	R_MeshDesc mesh = {
		.vertices = slice(vertices),
		.indices = slice(final_indices),
	};
	return mesh;
}

#define Gltf_i8  5120
#define Gltf_u8  5121
#define Gltf_i16 5122
#define Gltf_u16 5123
#define Gltf_u32 5125
#define Gltf_f32 5126

GltfInfo load_gltf(Allocator arena, String name, b32 is_glb) {
	Scratch scratch(arena);

	String path = push_strf(scratch, "%s/%s", st->models_dir, name);

	String json = {};
	Slice<u8> cursor = {};
	if(is_glb) {
		struct FileHeader {
			u32 magic;
			u32 version;
			u32 size;
		};
		struct ChunkHeader {
			u32 chunk_length;
			u32 chunk_type;
		};
		cursor = os_file_path_read_all(scratch, path);

		FileHeader* header = (FileHeader*)cursor.data;
		NoOp(header);
		Assert(str_match(String((u8*)&header->magic, 4), "glTF"));
		cursor = slice_skip(cursor, sizeof(FileHeader));

		ChunkHeader* json_chunk = (ChunkHeader*)cursor.data;
		Assert(str_match(String((u8*)&json_chunk->chunk_type, 4), "JSON"));
		cursor = slice_skip(cursor, sizeof(ChunkHeader));
		json = str_make(slice_prefix(cursor, json_chunk->chunk_length));
		cursor = slice_skip(cursor, json_chunk->chunk_length);

		ChunkHeader* bin_chunk = (ChunkHeader*)cursor.data;
		NoOp(bin_chunk);
		Assert(str_match(String((u8*)&bin_chunk->chunk_type, 3), "BIN"));
		cursor = slice_skip(cursor, sizeof(ChunkHeader));
	} else {
		json = os_file_path_read_all_str(scratch, path);
	}

	String file_path = str_chop_last_dot(path);
	String file_path_bin = push_strf(scratch, "%s.bin", file_path);
	Slice data = is_glb ? cursor : os_file_path_read_all(scratch, file_path_bin);

	u32 vert_count = 0;
	u32 indices_count = 0;
	Slice<v3> pos = {};
	Slice<v3> norm = {};
	Slice<v2> texcoord = {};
	Slice<u16> indices_u16 = {};
	Slice<v4u8> joints = {};
	Slice<v4> weights = {};
	Slice<m4x4> inv_matrices = {};
	Slice<Bone> bones = {};
	Slice<Channel> channels = {};
	Slice<Sampler> samplers = {};

	JsParser p = js_parse_make(scratch, json);
	JsObj root = js_parse(p).obj;
	Slice nodes = js_array(root, "nodes").val();
	Slice accessors = js_array(root, "accessors").val();
	Slice buffer_views = js_array(root, "bufferViews").val();

	///////////////////////////////////
	// Mesh
	{
		Slice meshes = js_array(root, "meshes").val();
		JsObj first_mesh = meshes[0]->obj;
		Slice primitives = js_array(first_mesh, "primitives").val();
		JsObj first_primitive = primitives[0]->obj;
		JsObj attributes = js_obj(first_primitive, "attributes").val();
		{
			u32 pos_attribute = js_number(attributes, "POSITION").val();
			JsObj pos_accessor = accessors[pos_attribute]->obj;
			u32 buffer_view = js_number(pos_accessor, "bufferView").val();
			vert_count = js_number(pos_accessor, "count").val();
			JsObj pos_buffer_view = buffer_views[buffer_view]->obj;
			u64 size = js_number(pos_buffer_view, "byteLength").val();
			u64 off = js_number(pos_buffer_view, "byteOffset").val();
			pos = slice_reinterpret<v3>(slice_n(data, off, size));
		}
		{
			u32 norm_attribute = js_number(attributes, "NORMAL").val();
			JsObj norm_accessor = accessors[norm_attribute]->obj;
			u32 buffer_view = js_number(norm_accessor, "bufferView").val();
			JsObj norm_buffer_view = buffer_views[buffer_view]->obj;
			u64 size = js_number(norm_buffer_view, "byteLength").val();
			u64 off = js_number(norm_buffer_view, "byteOffset").val();
			norm = slice_reinterpret<v3>(slice_n(data, off, size));
		}
		{
			u32 texcoord_attribute = js_number(attributes, "TEXCOORD_0").val();
			JsObj texcoord_accessor = accessors[texcoord_attribute]->obj;
			u32 buffer_view = js_number(texcoord_accessor, "bufferView").val();
			JsObj texcoord_buffer_view = buffer_views[buffer_view]->obj;
			u64 size = js_number(texcoord_buffer_view, "byteLength").val();
			u64 off = js_number(texcoord_buffer_view, "byteOffset").val();
			texcoord = slice_reinterpret<v2>(slice_n(data, off, size));
		}
		{
			u32 indices_attribute = js_number(first_primitive, "indices").val();
			JsObj indices_accessor = accessors[indices_attribute]->obj;
			u32 buffer_view = js_number(indices_accessor, "bufferView").val();
			indices_count = js_number(indices_accessor, "count").val();
			JsObj indices_buffer_view = buffer_views[buffer_view]->obj;
			u64 size = js_number(indices_buffer_view, "byteLength").val();
			u64 off = js_number(indices_buffer_view, "byteOffset").val();
			indices_u16 = slice_reinterpret<u16>(slice_n(data, off, size));
		}
		{
			var[joints_attribute, ok] = js_number(attributes, "JOINTS_0");
			if(ok) {
				JsObj joints_accessor = accessors[joints_attribute]->obj;
				u32 buffer_view = js_number(joints_accessor, "bufferView").val();
				JsObj joints_buffer_view = buffer_views[buffer_view]->obj;
				u64 size = js_number(joints_buffer_view, "byteLength").val();
				u64 offset = js_number(joints_buffer_view, "byteOffset").val();
				joints = slice_reinterpret<v4u8>(slice_n(data, offset, size));
			}
		}
		{
			var[weights_attribute, ok] = js_number(attributes, "WEIGHTS_0");
			if(ok) {
				JsObj weights_accessor = accessors[weights_attribute]->obj;
				u32 buffer_view = js_number(weights_accessor, "bufferView").val();
				JsObj weights_buffer_view = buffer_views[buffer_view]->obj;
				u64 size = js_number(weights_buffer_view, "byteLength").val();
				u64 off = js_number(weights_buffer_view, "byteOffset").val();
				weights = slice_reinterpret<v4>(slice_n(data, off, size));
			}
		}
	}

	///////////////////////////////////
	// Skin
	{
		var[skins, ok] = js_array(root, "skins");
		if(ok) {
			JsObj first_skin = skins[0]->obj;
			u32 inv_matrices_attribute = js_number(first_skin, "inverseBindMatrices").val();
			JsObj inv_matrices_accessor = accessors[inv_matrices_attribute]->obj;
			u32 buffer_view = js_number(inv_matrices_accessor, "bufferView").val();
			JsObj inv_matrices_buffer_view = buffer_views[buffer_view]->obj;
			u64 size = js_number(inv_matrices_buffer_view, "byteLength").val();
			u64 off = js_number(inv_matrices_buffer_view, "byteOffset").val();
			var matrices = slice_reinterpret<m4x4>(slice_n(data, off, size));
			inv_matrices = push_slice(arena, m4x4, matrices.count);
			Slice joints_indices = js_array(first_skin, "joints").val();
			bones = push_slice(arena, Bone, joints_indices.count);
			u32 bone_count = 0;
			u32 node_to_joint[255] = {};
			u32 node_to_bone[255] = {};
			Loop(joint_idx, joints_indices.count) {
				u32 node_idx = joints_indices[joint_idx]->number;
				node_to_joint[node_idx] = joint_idx;
			}
			var dfs = [&](var dfs, u32 node_idx, u32 parent_idx) -> void {
				JsObj node = nodes[node_idx]->obj;
				u32 bone_idx = bone_count++;
				node_to_bone[node_idx] = bone_idx;
				inv_matrices[bone_idx] = matrices[node_to_joint[node_idx]];
				Transform trans = {};
				trans.pos = js_v3(node, "translation").or_else(v3());
				trans.rot = js_v4(node, "rotation").or_else(quat_identity());
				trans.scale = js_v3(node, "scale").or_else(v3(1));
				bones[bone_idx].local = trans;
				bones[bone_idx].parent = parent_idx;
				Slice children = js_array(node, "children").or_else({});
				Loop(i, children.count) {
					dfs(dfs, children[i]->number, bone_idx);
				}
			};
			dfs(dfs, joints_indices[0]->number, 0);

			// Animation
			var[animations, ok] = js_array(root, "animations");
			if(ok) {
				var first_animation = animations[0]->obj;
				var js_channels = js_array(first_animation, "channels").val();
				channels = push_slice(arena, Channel, js_channels.count);
				Loop(i, js_channels.count) {
					var channel = js_channels[i]->obj;
					u32 sampler = js_number(channel, "sampler").val();
					var target = js_obj(channel, "target").val();
					u32 node = js_number(target, "node").val();
					String path = js_str(target, "path").val();
					channels[i].bone = node_to_bone[node];
					channels[i].sampler = sampler;
					AnimPath anim_path = {};
					if(str_match(path, "translation")) {
						anim_path = AnimPath_Pos;
					} else if(str_match(path, "rotation")) {
						anim_path = AnimPath_Rot;
					} else if(str_match(path, "scale")) {
						anim_path = AnimPath_Scale;
					} else InvalidPath;
					channels[i].path = anim_path;
				}
				var js_samplers = js_array(first_animation, "samplers").val();
				samplers = push_slice(arena, Sampler, js_samplers.count);
				Loop(i, js_samplers.count) {
					var sampler = js_samplers[i]->obj;
					u32 input = js_number(sampler, "input").val();
					JsObj input_accessor = accessors[input]->obj;
					u32 input_buffer_view = js_number(input_accessor, "bufferView").val();
					JsObj js_input_buffer_view = buffer_views[input_buffer_view]->obj;
					u64 input_size = js_number(js_input_buffer_view, "byteLength").val();
					u64 input_off = js_number(js_input_buffer_view, "byteOffset").val();
					var timestamps = slice_reinterpret<f32>(slice_n(data, input_off, input_size));
					samplers[i].times = slice_clone(arena, timestamps).data;
					samplers[i].count = timestamps.count;

					u32 output = js_number(sampler, "output").val();
					JsObj output_accessor = accessors[output]->obj;
					u32 output_buffer_view = js_number(output_accessor, "bufferView").val();
					String output_type = js_str(output_accessor, "type").val();
					JsObj js_output_buffer_view = buffer_views[output_buffer_view]->obj;
					u64 output_size = js_number(js_output_buffer_view, "byteLength").val();
					u64 output_off = js_number(js_output_buffer_view, "byteOffset").val();
					if(str_match(output_type, "VEC3")) {
						var values = slice_reinterpret<v3>(slice_n(data, output_off, output_size));
						samplers[i].values = slice_clone(arena, values).data;
					} else if(str_match(output_type, "VEC4")) {
						var values = slice_reinterpret<v4>(slice_n(data, output_off, output_size));
						samplers[i].values = slice_clone(arena, values).data;
					} else InvalidPath;
				}
			}
		}
	}

	Slice vertices = push_slice(arena, R_Vertex, vert_count);
	Slice indices = push_slice(arena, u16, indices_count);
	Loop(i, indices.count) {
		indices[i] = indices_u16[i];
	}
	Loop(i, vertices.count) {
		vertices[i] = {
			.pos = pos[i],
			.norm = norm[i],
			.uv = texcoord[i],
		};
	}
	if(joints.count) {
		Loop(i, vertices.count) {
			// vertices[i].joints = joints[i];
			vertices[i].joints.v[0] = joints[i].v[0];
			vertices[i].joints.v[1] = joints[i].v[1];
			vertices[i].joints.v[2] = joints[i].v[2];
			vertices[i].joints.v[3] = joints[i].v[3];
			// vertices[i].weights = weights[i];
		}
	}
	GltfInfo res = {
		.mesh = {
			.vertices = vertices,
			.indices = indices,
		},
		.bones = bones,
		.inv_matrices = inv_matrices,
		.animation = {
			.channels = channels,
			.samplers = samplers,
		},
	};
	res.animation.duration = animation_duration(res.animation);
	return res;
}

v3 sample_v3(Sampler s, f32 time) {
	u32 found = -1;
	LoopOff(i, 1, s.count) {
		if(time < s.times[i]) {
			found = i-1;
			break;
		}
	}
	if(found == -1)
		return ((v3*)s.values)[s.count - 1];
	f32 t0 = s.times[found];
	f32 t1 = s.times[found + 1];
	f32 t = unlerp(t0, time, t1);
	v3 a = ((v3*)s.values)[found];
	v3 b = ((v3*)s.values)[found + 1];
	return v3_lerp(a, t, b);
}
v4 sample_quat(Sampler s, f32 time) {
	u32 found = -1;
	LoopOff(i, 1, s.count) {
		if(time < s.times[i]) {
			found = i-1;
			break;
		}
	}
	if(found == -1)
		return ((v4*)s.values)[s.count - 1];
	f32 t0 = s.times[found];
	f32 t1 = s.times[found + 1];
	f32 t = unlerp(t0, time, t1);
	v4 a = ((v4*)s.values)[found];
	v4 b = ((v4*)s.values)[found + 1];
	return quat_slerp(a, t, b);
}

void update_animation(Animation anim, Pose pose, f32 time) {
	for(var channel : anim.channels) {
		Sampler sampler = anim.samplers[channel.sampler];
		Transform& bone = pose.bones[channel.bone].local;
		switch(channel.path) {
			case AnimPath_Pos: {
				bone.pos = sample_v3(sampler, time);
			} break;
			case AnimPath_Rot: {
				bone.rot = sample_quat(sampler, time);
			} break;
			case AnimPath_Scale: {
				bone.scale = sample_v3(sampler, time);
			} break;
		}
	}
}

f32 animation_duration(Animation anim) {
	f32 duration = 0;
	for(var sampler : anim.samplers) {
		f32 last_time = sampler.times[sampler.count - 1];
		duration = Max(duration, last_time);
	}
	return duration;
}

JsParser js_parse_make(Allocator arena, String str) {
	JsParser res = {
		.arena = arena,
		.str = str,
	};
	return res;
}
b32 js_is_end(JsParser& p) {
	return p.cursor >= p.str.size;
}
u8 js_peek(JsParser& p) {
	if(js_is_end(p)) return 0;
	return p.str.str[p.cursor];
}
void js_skip_ws(JsParser& p) {
	while(char_is_ws(js_peek(p))) {
		p.cursor++;
	}
}
String js_parse_str(JsParser& p) {
	Assert(js_peek(p) == '\"');
	p.cursor++;
	u32 start = p.cursor;
	while(js_peek(p) != '\"' && !js_is_end(p)) {
		p.cursor++;
	}
	Assert(js_peek(p) == '\"'); 
	String res = str_substr(p.str, Rng1u(start, p.cursor));
	p.cursor++;
	return res;
}
f64 js_parse_number(JsParser& p) {
	u32 start = p.cursor;
	if(js_peek(p) == '-')
		p.cursor++;
	while(char_is_digit(js_peek(p)))
		p.cursor++;
	if(js_peek(p) == '.') {
		p.cursor++;
		while(char_is_digit(js_peek(p)))
			p.cursor++;
	}
	if(js_peek(p) == 'e' || js_peek(p) == 'E') {
		p.cursor++;
		if(js_peek(p) == '+' || js_peek(p) == '-')
			p.cursor++;
		while(char_is_digit(js_peek(p)))
			p.cursor++;
	}
	String str = str_substr(p.str, Rng1u(start, p.cursor));
	f64 res = f64_from_str(str);
	return res;
}
JsVal js_parse(JsParser& p) {
	js_skip_ws(p);
	switch(js_peek(p)) {
		default:   return {.type = JsType_Number, .number = js_parse_number(p)};
		case '\"': return {.type = JsType_Str, .str = js_parse_str(p)};
		case 't': p.cursor += 4; return {.type = JsType_Bool, .boolean = true};
		case 'f': p.cursor += 5; return {.type = JsType_Bool, .boolean = false};
		case 'n': p.cursor += 4; return {.type = JsType_Null};
		case '[': {
			p.cursor++;
			var arr = array_make<JsVal*>(p.arena);
			For {
				js_skip_ws(p);
				if(js_peek(p) == ']') {
					p.cursor++;
					break;
				}
				JsVal* val = push_struct(p.arena, JsVal);
				*val = js_parse(p);
				// switch(val->type) {
				//   case JsType_Null: Info("null"); break;
				//   case JsType_Bool: Info("bool: %u", val->boolean); break;
				//   case JsType_Number: Info("num: %f", val->number); break;
				//   case JsType_Str: Info("str: %s", val->str); break;
				//   case JsType_Array: Info("array"); break;
				//   case JsType_Obj:  Info("obj"); break;
				// }
				array_push(arr, val);
				js_skip_ws(p);
				if(js_peek(p) == ',') {
					p.cursor++;
				}
			}
			JsVal res = {
				.type = JsType_Array,
				.array = slice(arr),
			};
			return res;
		}break;
		case '{': {
			p.cursor++;
			var fields = array_make<JsField>(p.arena);
			For {
				js_skip_ws(p);
				if(js_peek(p) == '}') {
					p.cursor++;
					break;
				}
				String key = js_parse_str(p);
				js_skip_ws(p);
				Assert(js_peek(p) == ':');
				p.cursor++;
				JsVal* val = push_struct(p.arena, JsVal);
				*val = js_parse(p);
				// switch(val->type) {
				//   case JsType_Null: Info("null"); break;
				//   case JsType_Bool: Info("bool: %u", val->boolean); break;
				//   case JsType_Number: Info("num: %f", val->number); break;
				//   case JsType_Str: Info("str: %s", val->str); break;
				//   case JsType_Array: Info("array"); break;
				//   case JsType_Obj:  Info("obj"); break;
				// }
				array_push(fields, {.key = key, .val = val});
				js_skip_ws(p);
				if(js_peek(p) == ',') {
					p.cursor++;
				}
			}
			JsVal res = {
				.type = JsType_Obj,
				.obj = slice(fields),
			};
			return res;
		}break;
	}
}

Result<JsVal> js_val(JsVal val, String key) {
	for(var& field : val.obj.fields) {
		if(str_match(field.key, key)) {
			return {*field.val, true};
		}
	}
	return {};
}
Result<b32> js_bool(JsObj obj, String key) {
	var[v, ok] = js_val({.obj = obj}, key);
	return {v.boolean, ok};
}
Result<f64> js_number(JsObj obj, String key) {
	var[v, ok] = js_val({.obj = obj}, key);
	return {v.number, ok};
}
Result<String> js_str(JsObj obj, String key) {
	var[v, ok] = js_val({.obj = obj}, key);
	return {v.str, ok};
}
Result<Slice<JsVal*>> js_array(JsObj obj, String key) {
	var[v, ok] = js_val({.obj = obj}, key);
	return {v.array, ok};
}
Result<JsObj> js_obj(JsObj obj, String key) {
	var[v, ok] = js_val({.obj = obj}, key);
	return {v.obj, ok};
}
Result<v3> js_v3(JsObj obj, String key) {
	var[v, ok] = js_val({.obj = obj}, key);
	if(ok) {
		v3 res = {};
		Loop(i, 3) {
			res.v[i] = v.array[i]->number;
		}
		return {res, true};
	}
	return {};
}
Result<v4> js_v4(JsObj obj, String key) {
	var[v, ok] = js_val({.obj = obj}, key);
	if(ok) {
		v4 res = {};
		Loop(i, 4) {
			res.v[i] = v.array[i]->number;
		}
		return {res, true};
	}
	return {};
}

f32 parse_f32(Parser& p) {
	b32 negative = false;
	if(tok_match(p, TokenType_Minus)) {
		negative = true;
	}
	Token tok = tok_expect(p, TokenType_Number);
	f32 v = f32_from_str(tok.str);
	return negative ? -v : v;
}
f32 parse_u32(Parser& p) {
	b32 negative = false;
	if(tok_match(p, TokenType_Minus)) {
		negative = true;
	}
	Token tok = tok_expect(p, TokenType_Number);
	i32 v = u32_from_str(tok.str);
	return negative ? -v : v;
}
f32 parse_i32(Parser& p) {
	b32 negative = false;
	if(tok_match(p, TokenType_Minus)) {
		negative = true;
	}
	Token tok = tok_expect(p, TokenType_Number);
	i32 v = i32_from_str(tok.str);
	return negative ? -v : v;
}
v3 parse_v3(Parser& p) {
	return v3(parse_f32(p), parse_f32(p), parse_f32(p));
}
v4 parse_v4(Parser& p) {
	return v4(parse_f32(p), parse_f32(p), parse_f32(p), parse_f32(p));
}
String write_v3(Allocator alloc, v3 v) { return push_strf(alloc, "%f %f %f", v.x, v.y, v.z); }
String write_v4(Allocator alloc, v4 v) { return push_strf(alloc, "%f %f %f %f", v.x, v.y, v.z, v.w); }

v2 world_to_screen(Camera2 c, v2 p, v2 screen_center) {
	return screen_center + (p - c.pos) * c.zoom;
}
Rng2 world_to_screen(Camera2 c, Rng2 rect, v2 screen_center) {
	Rng2 res = {
		screen_center + (rect.min - c.pos) * c.zoom,
		screen_center + (rect.max - c.pos) * c.zoom,
	};
	return res;
}
v2 world_to_screen2(Camera2 c, v2 p, v2 screen_center) {
	v2 res = {
		screen_center.x + (p.x - c.pos.x) * c.zoom2.x,
		screen_center.y + (p.y - c.pos.y) * c.zoom2.y,
	};
	return res;
}
v2 screen_to_world(Camera2 c, v2 p, v2 screen_center) {
	return c.pos + (p - screen_center) / c.zoom;
}
v2 screen_to_world2(Camera2 c, v2 p, v2 screen_center) {
	v2 res = {
		c.pos.x + (p.x - screen_center.x) / c.zoom2.x,
		c.pos.y + (p.y - screen_center.y) / c.zoom2.y,
	};
	return res;
}
void camera_zoom_at(Camera2& c, v2 mouse, v2 screen_center, f32 factor) {
	v2 before = screen_to_world(c, mouse, screen_center);
	c.zoom *= factor;
	v2 after = screen_to_world(c, mouse, screen_center);
	c.pos += before - after;
}
void camera_zoom_at2(Camera2& c, v2 mouse, v2 screen_center, f32 factor) {
	v2 before = screen_to_world2(c, mouse, screen_center);
	c.zoom *= factor;
	v2 after = screen_to_world2(c, mouse, screen_center);
	c.pos += before - after;
}
void camera_zoom_at_pow_clamp(Camera2& c, v2 mouse, v2 screen_center, f32 factor) {
	v2 before = screen_to_world2(c, mouse, screen_center);
	c.zoom *= factor;
	c.zoom = ClampBot(c.zoom, 0.02);
	c.zoom2.y = Clamp(0.01, c.zoom, 3);
	f32 t = pow(c.zoom + 1, 2);
	f32 ratio = t * 0.3;
	c.zoom2.x = c.zoom2.y * ratio;
	if(c.zoom2.y > 1) {
		f32 inv = 1.0f / c.zoom;
		f32 target = inv / (1.0f + inv);
		c.zoom2.y = lerp(1.0f, 0.3f, target);
	}
	v2 after = screen_to_world2(c, mouse, screen_center);
	c.pos += before - after;
}
void camera_pan(Camera2& camera, v2 screen_delta) {
	camera.pos -= screen_delta / camera.zoom;
}
void camera_pan2(Camera2& camera, v2 screen_delta) {
	camera.pos.x -= screen_delta.x / camera.zoom2.x;
	camera.pos.y -= screen_delta.y / camera.zoom2.y;
}
void camera_zoom_and_pan(Camera2& c, v2 screen_center, b32 linear) {
	f32 wheel = os_mouse_wheel();
	if(wheel) {
		if(os_key_is_down(Key_LControl)) {
			if(!linear) {
				camera_zoom_at_pow_clamp(c, os_mouse_pos(), screen_center, wheel > 0 ? 1.3f : 1 / 1.3f);
			} else {
				camera_zoom_at(c, os_mouse_pos(), screen_center, wheel > 0 ? 1.3f : 1 / 1.3f);
			}
		} else {
			f32 wheel_h = os_mouse_wheel_horizontal();
			camera_pan2(c, v2(wheel_h * 100, wheel * 100));
		}
	} else {
		f32 wheel_h = os_mouse_wheel_horizontal();
		camera_pan2(c, v2(wheel_h * 100, wheel * 100));
	}
}

void watch_add(String watch_name, WatchOp op) {
	var& g = *st;
	FileProperties props = os_file_path_properties(watch_name);
	WatchFile file_watch = {
		.path = watch_name,
		.modified = props.modified,
		.op = op,
	};
	array_push(g.watches, file_watch);
}

void watch_directory_add(String watch_name, WatchOp op, OS_WatchFlags flags) {
	var& g = *st;
	String dir_path = push_strf(st->arena, "%s", watch_name);
	OS_Watch watch = os_watch_open(flags);
	os_watch_attach(watch, dir_path);
	WatchDirectory dir_watch = {
		.path = dir_path,
		.watch = watch,
		.op = op,
	};
	array_push(g.watch_directories, dir_watch);
}

void watch_update() {
	var& g = *st;
	Scratch scratch;
	Loop(i, g.watches.count) {
		WatchFile& x = g.watches[i];
		FileProperties props = os_file_path_properties(x.path);
		if(props.modified > x.modified) {
			switch(x.op) {
				case WatchOp_NotifyHotreload: {
					st->should_hotreload = true;
				}break;
				InvalidDefaultCase;
			}
			x.modified = props.modified;
		}
	}
	for(var dir : g.watch_directories) {
		Slice strs = os_watch_check(scratch, dir.watch);
		Loop(i, strs.count) {
			String name = strs[i];
			switch(dir.op) {
				case WatchOp_RecompileShader: {
					GlobalState& g = *st;
					Scratch scratch;
					String shader_filepath = push_strf(scratch, "%s/%s", g.shader_dir, name);
					String shader_compiled_filepath = push_strf(scratch, "%s/%s%s", g.shader_compiled_dir, str_chop_last_dot(name), String(".spv"));
					StringList list = {};
					str_list_push(scratch, list, "slangc");
					str_list_push(scratch, list, shader_filepath);
					str_list_push(scratch, list, "-target");
					str_list_push(scratch, list, "spirv");
					str_list_push(scratch, list, "-g");
					str_list_push(scratch, list, "-o");
					str_list_push(scratch, list, shader_compiled_filepath);
					os_process_make(list);
				}break;
				case WatchOp_ShaderReload: {
					GlobalState& g = *st;
					String shader_name = str_chop_last_dot(name);
					String shader_name_slang = push_strf(scratch, "%s.slang", shader_name);
					String shader_filepath = push_strf(scratch, "%s/%s", g.shader_dir, shader_name_slang);
					String shader_compiled_filepath = push_strf(scratch, "%s/%s", g.shader_compiled_dir, name);
					os_file_path_copy_mtime(shader_filepath, shader_compiled_filepath);
					r_reload_shader(shader_name);
				}break;
				InvalidDefaultCase;
			}
		}
	}
}

ThingDesc default_thing_desc() {
	ThingDesc res = {
		.scale = v3(1),
		.rot = quat_identity(),
		.color = ColorWhite,
		.mesh_id = get_mesh(Mesh_Cube),
		.mat_id = get_material(Material_Orange),
	};
	return res;
}

R_MaterialProps default_material_props() {
	R_MaterialProps props = {
		.ambient = v3(1),
		.diffuse = v3(1),
		.specular = v3(1),
		.shininess = 1,
	};
	return props;
}

Thing& get_thing(ThingId id) { return pool_get(st->things, id); }
R_MeshId get_mesh(MeshEnum mesh_enum) { return st->meshes_ids[mesh_enum]; }
R_TextureId get_texture(TextureEnum tex_enum) { return st->textures_ids[tex_enum]; }
R_MaterialId get_material(MaterialEnum id) { return st->materials_ids[id]; }

void push_child_thing(ThingId parent, ThingId id) {
	var& g = *st;
	var& t = get_thing(parent);
	hdll_push_back(g.things.pool.data, t.first, t.last, id);
}

String dumb_struct(Allocator arena, Slice<MemberDefinition> members, void* ptr, EntityFlags flags) {
	Scratch scratch(arena);
	var string = dstr_make(arena);
	for(var member : members) {
		u8* member_ptr = Offset(ptr, member.offset);
		switch(member.type) {
			default:break;
			case MetaType_u32: {
				dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(u32*)member_ptr));
			}break;
			case MetaType_i32: {
				dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(i32*)member_ptr));
			}break;
			case MetaType_b32: {
				dstr_push(string, push_strf(scratch, "%s %u\n", member.name, *(b32*)member_ptr));
			}break;
			case MetaType_f32: {
				dstr_push(string, push_strf(scratch, "%s %f\n", member.name, *(f32*)member_ptr));
			}break;
			case MetaType_v2: {
				v2 v = *(v2*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s %f %f\n", member.name, v.x, v.y));
			}break;
			case MetaType_v3: {
				v3 v = *(v3*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s %f %f %f\n", member.name, v.x, v.y, v.z));
			}break;
			case MetaType_v4: {
				v4 v = *(v4*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s %f %f %f %f\n", member.name, v.x, v.y, v.z, v.w));
			}break;
			case MetaType_Rng3: {
				Rng3 v = *(Rng3*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s %f %f %f %f %f %f\n", member.name, v.min.x,v.min.y,v.min.z, v.max.x,v.max.y,v.max.z));
			}break;
			case MetaType_String: {
				String v = *(String*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, v));
			}break;
			case MetaType_R_MeshId: {
				R_MeshId v = *(R_MeshId*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->mesh_id_to_str[v.idx]));
			}break;
			case MetaType_R_MaterialId: {
				R_MaterialId v = *(R_MaterialId*)member_ptr;
				dstr_push(string, push_strf(scratch, "%s \"%s\"\n", member.name, st->material_id_to_str[v.idx]));
			}break;
		}
	}
	return string;
}

void dumb_struct_load(Slice<MemberDefinition> members, void* ptr, Parser* parser) {
	Parser& p = *parser;
	while(!tok_match(p, TokenType_CloseBrace)) {
		MemberDefinition member = {};
		Token ident_token = tok_advance(p);
		if(ident_token.type == TokenType_Identifier) {
			Loop(i, members.count) {
				if(str_match(members[i].name, ident_token.str)) {
					member = members[i];
					break;
				}
			}
		}
		u8* mem = Offset(ptr, member.offset);
		switch(member.type) {
			default:break;
			case MetaType_u32: {
				Token tok = tok_advance(p);
				*(u32*)mem = u32_from_str(tok.str);
			}break;
			case MetaType_i32: {
				*(u32*)mem = parse_i32(p);
			}break;
			case MetaType_b32: {
				Token tok = tok_advance(p);
				*(u32*)mem = u32_from_str(tok.str);
			}break;
			case MetaType_f32: {
				*(f32*)mem = parse_f32(p);
			}break;
			case MetaType_v2: {
				*(v2*)mem = v2(parse_f32(p), parse_f32(p));
			}break;
			case MetaType_v3: {
				*(v3*)mem = v3(parse_f32(p), parse_f32(p), parse_f32(p));
			}break;
			case MetaType_v4: {
				*(v4*)mem = v4(parse_f32(p), parse_f32(p), parse_f32(p), parse_f32(p));
			}break;
			case MetaType_Rng3: {
				*(Rng3*)mem = Rng3(v3(parse_f32(p), parse_f32(p), parse_f32(p)), v3(parse_f32(p), parse_f32(p), parse_f32(p)));
			}break;
			// case MetaType_R_MeshId: {
			// 	Token tok = tok_expect(p, TokenType_String);
			// 	var [mesh, ok] = map_get(st->str_to_mesh, hash(tok.str));
			// 	Assert(ok);
			// 	*(R_MeshId*)mem = mesh;
			// }break;
			// case MetaType_MaterialId: {
			// 	Token tok = tok_expect(p, TokenType_String);
			// 	var [material, ok] = map_get(st->str_to_material, hash(tok.str));
			// 	Assert(ok);
			// 	*(R_MaterialId*)mem = material;
			// }break;
			// case MetaType_String: {
			// 	Token tok = tok_expect(p, TokenType_String);
			// 	*(String*)mem = push_str_copy(st->arena, tok.str);
			// }break;
			// case MetaType_EntityFlags: {
			// 	*(EntityFlags*)mem = parse_u32(p);
			// }break;
		}
	}
}

struct Observation {
	ThingId observed;
	ThingId observer;
	u32 next_observed, prev_observed;
	u32 next_observer, prev_observer;
};

Poolu32<Observation, KB(1)> observations;
Map<u32, KB(1)> first_observed;
Map<u32, KB(1)> first_observer;

Slice<ThingId> find_observed(Allocator arena, ThingId observer) {
	var res = array_make<ThingId>(arena);
	u64 key = observer.idx;
	var[first_idx, ok] = map_get(first_observed, key);
	if(!ok) return {};
	u32 idx = first_idx;
	do {
		Observation o = observations.data[idx];
		array_push(res, o.observed);
		idx = o.next_observed;
	}while(idx != first_idx);
	return slice(res);
}

Slice<ThingId> find_observers(Allocator arena, ThingId observed) {
	var res = array_make<ThingId>(arena);
	u64 key = observed.idx;
	var[first_idx, ok] = map_get(first_observer, key);
	if(!ok) return {};
	u32 idx = first_idx;
	do {
		Observation o = observations.data[idx];
		array_push(res, o.observer);
		idx = o.next_observer;
	}while(idx != first_idx);
	return slice(res);
}

u32 add_observation(ThingId observer, ThingId observed) {
	u32 idx = pool_push(observations, {.observed = observed, .observer = observer});
	{
		u64 key = observer.idx;
		var[first, ok] = map_get(first_observed, key);
		if(ok) {
			DLL_list_push(observations.data, first, idx, next_observed, prev_observed);
		} else {
			DLL_list_init(observations.data, idx, next_observed, prev_observed);
			map_set(first_observed, key, idx);
		}
	}
	{
		u64 key = observed.idx;
		var[first, ok] = map_get(first_observer, key);
		if(ok) {
			DLL_list_push(observations.data, first, idx, next_observer, prev_observer);
		} else {
			DLL_list_init(observations.data, idx, next_observer, prev_observer);
			map_set(first_observer, key, idx);
		}
	}
	return idx;
}

void remove_observation(u32 idx) {
	var o = observations.data[idx];
	{
		DLL_list_remove(observations.data, idx, next_observed, prev_observed);
		u64 key = o.observer.idx;
		var[first, ok] = map_get(first_observed, key);
		if(first == idx) {
			if(o.next_observed != idx) {
				map_set(first_observed, key, o.next_observed);
			} else {
				map_remove(first_observed, key);
			}
		}
	}
	{
		DLL_list_remove(observations.data, idx, next_observer, prev_observer);
		u64 key = o.observed.idx;
		var[first, ok] = map_get(first_observer, key);
		if(first == idx) {
			if(o.next_observer != idx) {
				map_set(first_observer, key, o.next_observer);
			} else {
				map_remove(first_observer, key);
			}
		}
	}
	pool_remove(observations, idx);
}

void print_observed(ThingId id) {
	Scratch scratch;
	var s = find_observed(scratch, id);
	print("Thing %i is observed by: ", id.idx);
	for(var idx : s) {
		print("%i ", idx);
	}
	println("");
}

void print_observers(ThingId id) {
	Scratch scratch;
	var s = find_observers(scratch, id);
	print("Thing %i is observing: ", id.idx);
	for(var idx : s) {
		print("%i ", idx);
	}
	println("");
}

void check_observer_observed() {
	Scratch scratch;

	add_observation({2}, {1}); /* Enemy A observes Player      -> idx 0 */
	uint32_t enemyB_watches_player =
		add_observation({3}, {1}); /* Enemy B observes Player      -> idx 1 */
	add_observation({4}, {1});	 /* Enemy C observes Player      -> idx 2 */
	add_observation({2}, {5});	 /* Enemy A also observes QuestGiver -> idx 3 */

	Info("-- before removal --");
	print_observers({1}); /* expect: 2 3 4 (some order) */
	print_observed({2});	 /* expect: 1 5 */
	print_observed({3});	 /* expect: 1 */
	print_observers({5}); /* expect: 2 */

	/* Enemy B stops observing Player */
	remove_observation(enemyB_watches_player);

	Info("\n-- after Enemy B stops observing Player --");
	print_observers({1}); /* expect: 2 4 only, no 3 */
	print_observed({3});	 /* expect: (nothing) - B's chain is now empty */
}

void init() {
	Scratch scratch;
	var& g = *st;

	g.asset_dir = push_strf(g.arena, "%s/%s", os_cur_directory(), String("../assets"));
	g.shader_dir = push_str_cat(g.arena, g.asset_dir, "/shaders");
	g.shader_compiled_dir = push_str_cat(g.arena, g.shader_dir, "/compiled");
	g.models_dir = push_str_cat(g.arena, g.asset_dir, "/models");
	g.textures_dir = push_str_cat(g.arena, g.asset_dir, "/textures");

	for(var s : os_args()) {
		if(str_match(s, "compile_shaders")) {
			u64 s = os_now_ns();
			r_shaders_compile(scratch);
			r_shaders_compile_join();
			Info("took: %fms", f64(os_now_ns()-s) / Million(1));
			os_exit(0);
		}
	}

	r_shaders_compile(scratch);

	g.gpa = alloc_make(g.arena);
	g.frame_arena = arena_make();

	cpu_find_frequency();
	prof_init(g.arena);
	prof_launch_begin();
	{
		ProfBlock("init");
		os_gfx_init();
		thread_pool_init();
		test();
		r_init();
		dev_init();
		init_game();
		watch_directory_add(g.shader_dir, WatchOp_RecompileShader);
		watch_directory_add(g.shader_compiled_dir, WatchOp_ShaderReload);
		ui_init();
	}
	prof_launch_end();
}

shared_function void update(HotReloadData* data) {
	Scratch scratch;
if(data->ctx == null) {
		Arena arena = arena_make(.name = "common arena");
		data->ctx = st = push_struct_zero(arena, GlobalState);
		st->arena = arena;
		{
			u64 start = cpu_now();
			init();
			Debug("init time: %fms", tsc_to_ms(cpu_now() - start));
		}
#if HOTRELOAD_BUILD
		watch_add(data->lib_path, WatchOp_NotifyHotreload);
#endif
	}
	if(!st) {
		st = (GlobalState*)data->ctx;
		st->should_hotreload = false;
	}

	var& g = *st;
	g.fps = 60;
	u64 prev_ns = os_now_ns();
	while(!os_window_should_close()) {
		if(st->should_hotreload) {
			goto hotreload;
		}
		u64 target_fps = Billion(1) / g.fps;
		prof_begin_frame();
		{
			ProfBlock("frame");
			os_pump_messages();
			u64 now_ns = os_now_ns();
			time_dt = f64(now_ns - prev_ns) / Billion(1);
			time_now += time_dt;
			prev_ns = now_ns;
			r_begin();
			ui_begin_frame();
			update_game();
			ui_end_frame();
			r_end();
			watch_update();

			u64 frame_duration = os_now_ns() - now_ns;
			if(frame_duration < target_fps) {
				u64 sleep_time = target_fps - frame_duration;
				ProfBlock("main sleep", ProfType_Sleep);
				os_sleep_ms(sleep_time / Million(1));
			}
		}
		prof_end_frame();
		current_frame++;
		arena_clear(st->frame_arena);
		mem_track_end();
	}

	// vk_shutdown();
	// os_gfx_shutdown();
	os_exit(0);

	hotreload:
	thread_wait_remanings();
}

R_MeshDesc generate_sphere(Allocator arena) {
	u32 lat_steps = 10;
	u32 lon_steps = 10;
	u32 vert_count = lat_steps*lon_steps;
	u32 index_count = (lat_steps - 1) * lon_steps * 6;
	var vertices = push_slice(arena, R_Vertex, vert_count);
	var indices = push_slice(arena, u16, index_count);
	f32 lat_step_angle = PI / lat_steps;
	f32 lon_step_angle = 2*PI / lon_steps;
	for(u32 i = 0; i < lat_steps; i++) {
		f32 lat_angle = -PI/2 + i*lat_step_angle;
		for(u32 j = 0; j < lon_steps; j++) {
			f32 lon_angle = j * lon_step_angle;
			R_Vertex vert = {
				.pos.x = cos(lat_angle) * cos(lon_angle),
				.pos.y = sin(lat_angle),
				.pos.z = cos(lat_angle) * sin(lon_angle),
				.uv.x = (f32)j / (lon_steps - 1),  // 0 → 1 across longitude
				.uv.y = (f32)i / (lat_steps - 1),  // 0 → 1 from bottom to top
			};
			vertices[i*lon_steps + j] = vert;
		}
	}
	var idx = [&](u32 i, u32 j) {
		return i * lon_steps + j;
	};
	u32 k = 0;
	for(u32 i = 0; i < lat_steps - 1; i++) {
		for(u32 j = 0; j < lon_steps; j++) {
			u32 next_j = (j + 1) % lon_steps; // wrap around
			u32 v0 = idx(i, j);
			u32 v1 = idx(i, next_j);
			u32 v2 = idx(i + 1, j);
			u32 v3 = idx(i + 1, next_j);
			indices[k++] = v0;
			indices[k++] = v2;
			indices[k++] = v1;
			indices[k++] = v1;
			indices[k++] = v2;
			indices[k++] = v3;
		}
	}
	// u32 north_pole_index = vert_count - 1; // last vertex
	// for(u32 j = 0; j < lon_steps; ++j) {
	//   u32 next_j = (j + 1) % lon_steps;
	//   indices[k++] = idx(lat_steps - 2, j); // last row before pole
	//   indices[k++] = north_pole_index;      // pole
	//   indices[k++] = idx(lat_steps - 2, next_j);
	// }
	R_MeshDesc mesh = {
		.vertices = vertices,
		.indices = indices,
	};
	return mesh;
}

R_MeshDesc generate_grid(Allocator arena, u32 size, f32 step) {
	var vertices = push_slice(arena, R_Vertex, size*4);
	v3 pos_offset = v3(-(i32)size/2, 0, -(i32)size/2);
	for(i32 i = 0; i < size; i++) {
		vertices[i*2].pos = pos_offset + v3(0, 0, i*step);
		vertices[i*2+1].pos = pos_offset + v3(size*step, 0, i*step);
	}
	var vertical_vertices = slice_skip(vertices, size*2);
	for(i32 i = 0; i < size; i++) {
		vertical_vertices[i*2].pos = pos_offset + v3(i*step, 0, 0);
		vertical_vertices[i*2+1].pos = pos_offset + v3(i*step, 0, size*step);
	}
	R_MeshDesc mesh = {
		.vertices = vertices,
	};
	return mesh;
}

// b32 ray_intersect_AABB(Ray ray, AABB aabb) {
//   v3 tMin = v3_hadamard_div(aabb.min - ray.origin, ray.dir);
//   v3 tMax = v3_hadamard_div(aabb.max - ray.origin, ray.dir);
//   v3 t1 = v3_less(tMin, tMax);
//   v3 t2 = v3_greater(tMin, tMax);
//   f32 tNear = Max3(t1.x, t1.y, t1.z);
//   f32 tFar = Min3(t2.x, t2.y, t2.z);
//   if(tNear > tFar) {
//     return false;
//   }
//   return true;
// };

ThingId make_thing(ThingDesc desc) {
	var& g = *st;
	Thing e = {
		.pos = desc.pos,
		.rot = desc.rot,
		.scale = desc.scale,
		.mesh = desc.mesh_id,
		.mat = desc.mat_id,
		.aabb = desc.aabb,
		.color = desc.color,
	};
	ThingId id = pool_push(g.things, e);
	// sparse_set_push(g.active_entities, id.idx);
	g.entities_count++;
	return id;
}

void destroy_thing(ThingId id) {
	var& g = *st;
	pool_remove(g.things, id);
	// sparse_set_remove(g.active_entities, id.idx);
	g.entities_count--;
}

PoolIterativeIter<Thing, MaxEntities, ThingId> things_begin() {
	var& g = *st;
	return {&g.things};
}

HNodeIter<Thing, ThingId> thing_node_begin(ThingId first) {
	var& g = *st;
	return {g.things.pool.data, first};
}

void select_obj() {
	var& g = *st;
	v3 dir = ray_from_screen(os_mouse_pos(), os_window_size(), g.cam.pos, st->view, st->projection).dir;
	var desc = default_thing_desc();
	desc.pos = g.cam.pos;
	desc.scale = v3(0.3);
	desc.vel = dir * 4;
	make_thing(desc);
	// e.pos() = st->cam.pos + v3_norm(m4x4_forward(st->cam.view));
}

void save_game_state() {
	Scratch scratch;
	var& g = *st;
	Dstring data = dstr_make(scratch);

	{
		var& cam = g.cam;
		dstr_push(data, "Camera {\n");
		dstr_pushf(data, "pos %s\n", write_v3(scratch, cam.pos));
		dstr_pushf(data, "dir %f %f %f\n", write_v3(scratch, cam.dir));
		dstr_pushf(data, "yaw %f\n", cam.yaw);
		dstr_pushf(data, "pitch %f\n", cam.pitch);
		dstr_pushf(data, "fov %f\n", cam.fov);
		dstr_pushf(data, "accel %f\n", cam.accel);
		dstr_pushf(data, "vel %s\n", write_v3(scratch, cam.vel));
		dstr_pushf(data, "vel_friction %f\n", cam.vel_friction);
		dstr_push(data, "}\n");
	}

	{
		dstr_push(data, "Things {\n");
		u32 idx = 0;
		LoopIter(i, things_begin()) {
			Thing& e = *i;
			e.serialized_idx = idx;
			dstr_push(data, "{\n");
			dstr_pushf(data, "pos %s\n", write_v3(scratch, e.pos));
			dstr_pushf(data, "rot %s\n", write_v4(scratch, e.rot));
			dstr_pushf(data, "scale %s\n", write_v3(scratch, e.scale));
			dstr_pushf(data, "mesh \"%s\"\n", g.mesh_id_to_str[e.mesh.idx]);
			dstr_pushf(data, "mat \"%s\"\n", g.material_id_to_str[e.mat.idx]);
			dstr_push(data, "}\n");
			idx++;
		}
		dstr_push(data, "}\n");
		dstr_push(data, "ThingEnum {\n");
		LoopEnumNonZero(i, ThingEnum) {
			// var id = g.thing_enums[i];
			// if(pool_is_valid_handle(g.things.pool, id)) {
			// 	var& t = get_thing(id);
			// 	dstr_pushf(data, "%s %u\n", things_enum_strs[i], t.serialized_idx);
			// }
		}
		dstr_push(data, "}\n");
	}

	// {
	// 	dstr_push(data, "Camera {\n");
	// 	dstr_push(data, dumb_struct(scratch, slice(members_of_Camera), &g.cam));
	// 	dstr_push(data, "}\n");
	// }

	// // Things
	// dstr_push(data, "Things {\n");
	// {
	// 	Thing e = get_thing(g.cube1);
	// 	dstr_push(data, "cube1 {\n");
	// 	dstr_push(data, dumb_struct(scratch, slice(members_of_Thing), &e));
	// 	dstr_push(data, "}\n");
	// }
	// dstr_push(data, "}\n");

	// {
	// 	LoopIter(it, things_begin()) {
	// 		Thing& e = *it;
	// 		dstr_push(data, "Thing {\n");
	// 		dstr_push(data, dumb_struct(scratch, slice(members_of_Thing), &e, e.flags));
	// 		dstr_push(data, "}\n");
	// 	}
	// }
	os_file_path_write_all(push_strf(scratch, "%s/saved", os_cur_directory(), String("saved")), dstr_slice(data));
}

void load_game_state() {
	var& g = *st;
	Scratch scratch;
	LoopIter(it, things_begin()) {
		ThingId e_id = it.id();
		destroy_thing(e_id);
	}
	pool_clear(g.things.pool);
	String str = os_file_path_read_all_str(scratch, push_strf(scratch, "%s/saved", os_cur_directory(), String("saved")));
	Slice tokens = tokens_from_str(scratch, str);
	Parser p = parser_make(tokens);
	while(p.cur < p.tokens.count) {
		Token tok = tok_advance(p);
		if(tok.type == TokenType_Identifier) {
			if(str_match(tok.str, "Camera")) {
				var& cam = g.cam;
				tok_expect(p, TokenType_OpenBrace);
				tok_expect(p, TokenType_Identifier); cam.pos = parse_v3(p);
				tok_expect(p, TokenType_Identifier); cam.dir = parse_v3(p);
				tok_expect(p, TokenType_Identifier); cam.yaw = parse_f32(p);
				tok_expect(p, TokenType_Identifier); cam.pitch = parse_f32(p);
				tok_expect(p, TokenType_Identifier); cam.fov = parse_f32(p);
				tok_expect(p, TokenType_Identifier); cam.accel = parse_f32(p);
				tok_expect(p, TokenType_Identifier); cam.vel = parse_v3(p);
				tok_expect(p, TokenType_Identifier); cam.vel_friction = parse_f32(p);
				tok_expect(p, TokenType_CloseBrace);
			} else if(str_match(tok.str, "Things")) {
				tok_expect(p, TokenType_OpenBrace);
				while(tok_match(p, TokenType_OpenBrace)) {
					ThingDesc desc = {};
					tok_expect(p, TokenType_Identifier); desc.pos = parse_v3(p);
					tok_expect(p, TokenType_Identifier); desc.rot = parse_v4(p);
					tok_expect(p, TokenType_Identifier); desc.scale = parse_v3(p);
					tok_expect(p, TokenType_Identifier); desc.mesh_id = map_get(g.str_to_mesh_id, hash(tok_advance(p).str)).value;
					tok_expect(p, TokenType_Identifier); desc.mat_id = map_get(g.str_to_material_id, hash(tok_advance(p).str)).value;
					make_thing(desc);
					tok_expect(p, TokenType_CloseBrace);
				}
				tok_expect(p, TokenType_CloseBrace);
			} else if(str_match(tok.str, "ThingEnum")) {
				tok_expect(p, TokenType_OpenBrace);
				while(!tok_match(p, TokenType_CloseBrace)) {
					// var t = tok_expect(p, TokenType_Identifier);
					// u32 thing_idx = parse_u32(p) + 1;
					// u32 enum_idx = map_get(g.str_to_thing_enum, hash(t.str)).value;
					// g.thing_enums[enum_idx] = pool_get_handle(g.things.pool, thing_idx);
				}
			}
		}
	}

	// while(p.cur < p.tokens.count) {
	// 	Token tok = tok_advance(p);
	// 	if(tok.type == TokenType_Identifier) {
	// 		if(str_match(tok.str, "Camera")) {
	// 			dumb_struct_load(slice(members_of_Camera), &g.cam, &p);
	// 		} else if(str_match(tok.str, "Entity")) {
	// 			ThingId id = make_thing({});
	// 			Thing& e = get_thing(id);
	// 			dumb_struct_load(slice(members_of_Thing), &e, &p);
	// 			if(flag_has(e.flags, EntityFlag_Referenced)) {
	// 				if(str_match("monkey", e.name)) {
	// 					g.monkey0 = id;
	// 				} else if(str_match("axis_attached_to_cam", e.name)) {
	// 					g.axis_attached_to_cam_id = id;
	// 				} else if(str_match("rotating_cube", e.name)) {
	// 					g.cube0 = id;
	// 				} 
	// 			}
	// 		} else if(str_match(tok.str, "e")) {
	// 			ThingId e_id = make_thing({});
	// 			Thing& e = get_thing(e_id);
	// 			dumb_struct_load(slice(members_of_Thing), &e, &p);
	// 			g.cube1 = e_id;
	// 		}
	// 	}
	// }
}

void init_game() {
	ProfFunc;
	Scratch scratch;
	var& g = *st;
	g.arena = arena_make(.name = "game arena");
	g.gpa = alloc_make(g.arena);

	///////////////////////////////////
	// Assets
	{
		g.font = r_make_font({.name = "arial.ttf", .font_height = 32});
		r_set_cubemap(r_make_texture({.name = "night_cubemap", .is_cube = true}));

		// Known assets
		var mesh_set = [](MeshEnum mesh_enum, R_MeshId id) { 
			GlobalState& g = *st;
			g.meshes_ids[mesh_enum] = id;
			String str = push_str_copy(g.arena, meshes_strs[mesh_enum]);
			map_set(g.str_to_mesh_id, hash(str), id);
			g.mesh_id_to_str[id.idx] = str;
		};
		mesh_set(Mesh_Triangle, r_make_mesh({.vertices = slice(triangle_vertices)}));
		mesh_set(Mesh_Grid, r_make_mesh(generate_grid(scratch, 100, 1)));
		mesh_set(Mesh_Axis, r_make_mesh({.vertices = slice(axis_vertices)}));
		mesh_set(Mesh_Sphere, r_make_mesh(generate_sphere(scratch)));

		// Mehs
		var load_mesh = [&](MeshEnum enum_name, String name) {
			R_MeshId id = r_make_mesh({.name = name});
			g.meshes_ids[enum_name] = id;
			String str = push_str_copy(g.arena, name);
			map_set(g.str_to_mesh_id, hash(str), id);
			g.mesh_id_to_str[id.idx] = str;
		};
		load_mesh(Mesh_Cube, "cube_ok_uv.glb");
		load_mesh(Mesh_MonkeyGlb, "monkey.glb");
		load_mesh(Mesh_CubeGlft, "cube.gltf");
		load_mesh(Mesh_Barrack, "castle.gltf");
		// load_mesh(Mesh_T, "t.gltf");
		{
			var info = load_gltf(scratch, "t.gltf", false);
			mesh_set(Mesh_T, r_make_mesh(info.mesh));
			array_push_elems(g.r.bones, info.bones);
			array_push_elems(g.r.inv_matrices, info.inv_matrices);
		}

		// Texture
		var load_texture = [&](TextureEnum enum_name, String name) {
			R_TextureId id = r_make_texture({.name = name});
			g.textures_ids[enum_name] = id;
			String str = push_str_copy(g.arena, name);
			map_set(g.str_to_texture_id, hash(str), id);
			g.texture_id_to_str[id.idx] = str;
		};
		load_texture(Texture_Orange, "orange_lines_512.png");
		load_texture(Texture_Container, "container.jpg");
		load_texture(Texture_Barrack, "castle_diffuse.png");
		load_texture(Texture_Bricks, "bricks.png");
		load_texture(Texture_Dummy, "dummy.png");

		// Imgui dummy
		g.imgui_dummy = imgui_add_texture(g.textures_ids[Texture_Dummy]);

		// Materials
		var default_state = r_make_pipeline_state({
			.depth = {
				.compare = Gfx_CompareOp_Less,
				.write_enabled = true,
			}
		});
		var default_state_line = r_make_pipeline_state({
			.primitive_type = Gfx_PrimitiveType_Line,
			.depth = {
				.compare = Gfx_CompareOp_Less,
				.write_enabled = true,
			}
		});
		var load_mat = [&](MaterialEnum enum_name, R_Material desc) {
			R_MaterialId id = r_make_material(desc);
			g.materials_ids[enum_name] = id;
			String str = push_str_copy(g.arena, materials_strs[enum_name]);
			map_set(g.str_to_material_id, hash(str), id);
			g.material_id_to_str[id.idx] = str;
		};
		load_mat(Material_Dummy, {
			.type = ShaderType::Texture,
			.batch = default_state,
			.props = default_material_props(),
			.base_color = get_texture(Texture_Dummy),
		});
		load_mat(Material_Orange, {
			.type = ShaderType::Texture,
			.batch = default_state,
			.props = default_material_props(),
			.base_color = get_texture(Texture_Orange),
		});
		load_mat(Material_Container, {
			.type = ShaderType::Texture,
			.batch = default_state,
			.props = default_material_props(),
			.base_color = get_texture(Texture_Container),
		});
		load_mat(Material_Axis, {
			.type = ShaderType::VertColor,
			.batch = default_state_line,
		});
		load_mat(Material_Line, {
			.type = ShaderType::E_Color,
			.batch = default_state_line,
		});
		load_mat(Material_Barrack, {
			.type = ShaderType::Texture,
			.batch = default_state,
			.props = default_material_props(),
			.base_color = get_texture(Texture_Barrack),
		});
	}

	///////////////////////////////////
	// Camera
	{
		Camera& cam = g.cam;
		cam = {
			.pos = v3(0,0,5),
			.yaw = 180,
			.fov = 45,
			.vel_friction = 7.7f,
			.accel = 300,
		};
		cam.dir = {
			cosd(cam.yaw) * cosd(cam.pitch),
			sind(cam.pitch),
			sind(cam.yaw) * cosd(cam.pitch)
		};
		g.view = m4x4_look_at(cam.pos, cam.dir, v3_up());
	}
}

void update_game() {
	ProfFunc;
	Scratch scratch;
	var& g = *st;
	dev_update();

	// Test jobs
	#if 0
	{
		ProfBlock("push jobs");
		Loop(i, 2) {
			thread_push({.fn = [](void* ctx) {os_sleep_ms(rand_u32()%6);}, .priority = TaskPriority_Low});
		}
		WaitGroup id0 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
		WaitGroup id1 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
		WaitGroup id2 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
		WaitGroup id3 = thread_push({.fn = [](void* ctx) { os_sleep_ms(2); }});
		TaskDesc tasks[] = {
			{.fn = [](void* ctx) { os_sleep_ms(2); }},
			{.fn = [](void* ctx) { os_sleep_ms(2); }},
			{.fn = [](void* ctx) { os_sleep_ms(2); }},
			{.fn = [](void* ctx) { os_sleep_ms(2); }},
		};
		WaitGroup batch = thread_push_batch(slice(tasks));
		thread_wg_wait(id0);
		thread_wg_wait(id1);
		thread_wg_wait(id2);
		thread_wg_wait(id3);
		thread_wg_wait(batch);
	}
	#endif

	// return;

	///////////////////////////////////
	// Hotkeys
	if(os_key_is_down(Key_Escape)) {
		os_window_close();
	}
	if(os_key_is_pressed(Key_U)) {
		if(g.fps_camera) {
			os_cursor_unlock();
		} else {
			os_cursor_lock();
		}
		g.fps_camera = !g.fps_camera;
	}

	///////////////////////////////////
	// Camera
	{
		Camera& cam = g.cam;
		v2 win_size = v2_of_v2u(os_window_size());
		m4x4& projection = st->projection;
		m4x4& view = st->view;
		projection = m4x4_perspective(deg2rad(cam.fov), win_size.x / win_size.y, 0.1f, 1000.0f);

		f32 rotation_speed = 180.0f * time_dt;
		if(os_key_is_down(Key_A)) {
			cam.yaw += rotation_speed;
		}
		if(os_key_is_down(Key_D)) {
			cam.yaw -= rotation_speed;
		}
		if(os_key_is_down(Key_R)) {
			cam.pitch += rotation_speed;
		}
		if(os_key_is_down(Key_F)) {
			cam.pitch -= rotation_speed;
		}
		if(g.fps_camera) {
			f32 rot_speed = 10;
			cam.pitch -= os_mouse_dt().y * time_dt * rot_speed;
			cam.yaw -= os_mouse_dt().x * time_dt * rot_speed;
		}
		f32 speed = 1;
		v3 mov = {};
		if(os_key_is_down(Key_W)) {
			mov += m4x4_forward(view);
		}
		if(os_key_is_down(Key_S)) {
			mov += m4x4_backward(view);
		}
		if(os_key_is_down(Key_Q)) {
			mov += m4x4_left(view);
		}
		if(os_key_is_down(Key_E)) {
			mov += m4x4_right(view);
		}
		if(os_key_is_down(Key_Space)) {
			mov.y += 1.0f;
		}
		if(os_key_is_down(Key_X)) {
			mov.y -= 1.0f;
		}
		if(os_key_is_down(Key_Shift)) {
			speed *= 10;
		}
		if(os_key_is_down(Key_LAlt)) {
			speed *= 0.1;
		}
		mov *= cam.accel * speed;
		f32 dt = time_dt;

		cam.vel += mov * dt;
		cam.pos += cam.vel * dt;
		cam.vel -= cam.vel * cam.vel_friction * dt;
		v4 yaw = quat_axis_angle(v3_up(), deg2rad(cam.yaw));
		v4 pitch = quat_axis_angle(v3_right(), deg2rad(-cam.pitch));
		v4 quat = quat_mul(yaw, pitch);
		cam.dir = quat_forward(quat);
		view = m4x4_look_at(cam.pos, cam.pos + cam.dir, quat_up(quat));
	}

	// XYZ Axis at center
	{
		f32 line_len = 5;
		v3 pos = v3(0, 0.1, 0);
		r_draw_line(pos, line_len * v3_right(), ColorRed);
		r_draw_line(pos, line_len * v3_up(), ColorGreen);
		r_draw_line(pos, line_len * v3_forward(), ColorBlue);
	}

	// Axis aligned at edge of screen
	{
		m4x4& view = st->view;
		v3 forward = m4x4_forward(view);
		v3 right   = m4x4_right(view);
		v3 up      = m4x4_up(view);
		f32 dist = 1.0f;
		f32 xoff = 0.3f;
		f32 yoff = 0.3f;
		v3 pos = g.cam.pos + forward*dist + right*xoff + up*yoff;
		r_draw_mesh_trs(get_mesh(Mesh_Axis), get_material(Material_Axis), pos, quat_identity(), v3(0.1));
	}
	r_draw_mesh(get_mesh(Mesh_Barrack), get_material(Material_Barrack), v3(0,0,-40));
	r_draw_grid(v3(0), 40, 2, u32_from_rgba(v4(0.8)));
	r_draw_mesh(get_mesh(Mesh_T), get_material(Material_Container), v3(), DrawCallFlag_Skeleton);
	// r_draw_mesh(get_mesh(Mesh_T), get_material(Material_Container), v3());

	r_draw_rect(rng2_make(v2(100), v2(100)), RED);
}

#define FONT_SIZE 24

String ui_display_string(String full) {
	u64 idx = str_find_needle(full, S("##"));
	if(idx < full.size) return {full.str, idx};
	return full;
}

String ui_hash_string(String full) {
	u64 idx = str_find_needle(full, S("###"));
	if(idx < full.size) return {full.str+idx, full.size-idx};
	return full;
}

u64 ui_key_from_string(String str, u64 seed) {
	String h = ui_hash_string(str);
	if(h.size == 0) return 0;
	return hash(h, seed);
}

f32 ui_text_measure(String text, R_FontId font, f32 font_height) {
	var f = pool_get(st->r.fonts, font);
	f32 width = 0;
	Loop(i, text.size) {
		u32 advance = f.glyphs[text.str[i]-32].xadvance;
		width += advance;
	}
	f32 scale = font_height / f.font_height;
	return width * scale;
}

UI_Size ui_size_px(f32 v) 	{ return {UI_SizeType_Pixels, v}; }
UI_Size ui_size_text() 				{ return {UI_SizeType_TextContent, 0}; }
UI_Size ui_size_pct(f32 v) { return {UI_SizeType_PercentOfParent, v}; }
UI_Size ui_size_children() { return {UI_SizeType_ChildrenSum, 0}; }
UI_Size ui_size_null()  			{ return {UI_SizeType_Null, 0}; }

void ui_push_parent(UI_Box* box) {
	var& g = st->ui;
	array_push(g.parent_stack, box);
}

void ui_pop_parent() {
	var& g = st->ui;
	array_pop(g.parent_stack);
}

UI_Box* ui_top_parent() {
	var& g = st->ui;
	return g.parent_stack.count > 0 ? array_back(g.parent_stack) : null;
}

void ui_push_font(R_FontId id) {
	var& g = st->ui;
	g.style.font = id;
}

void ui_push_bg_color(v4 color) {
	var& g = st->ui;
	array_push(g.bg_color_stack, color);
}

void ui_pop_bg_color() {
	var& g = st->ui;
	array_pop(g.bg_color_stack);
}

v4 ui_current_bg_color() {
	var& g = st->ui;
	return g.bg_color_stack.count > 0 ? array_back(g.bg_color_stack) : g.style.bg_color;
}

UI_Box* ui_box_make(UI_BoxFlags flags, UI_Size size_x, UI_Size size_y, String string) {
	var& g = st->ui;
	UI_Box* parent = ui_top_parent();
	u64 seed = parent ? parent->key : 0;
	u64 key = ui_key_from_string(string, seed);
	UI_Box* box = ui_box_from_key(key);
	box->first = box->last = box->next = box->prev = null;
	box->parent = parent;
	if(parent) dll_push_back(parent->first, parent->last, box);
	box->flags = flags;
	box->string = string;
	box->semantic_size[UI_Axis2_X] = size_x;
	box->semantic_size[UI_Axis2_Y] = size_y;
	box->child_layout_axis = UI_Axis2_X;
	box->background_color = ui_current_bg_color();
	box->text_color = g.style.text_color;
	box->border_color = g.style.border_color;
	box->last_frame_touched = current_frame;
	return box;
}

UI_Box* ui_box_from_key(u64 key) {
	var& g = st->ui;
	if(key == 0) {
		UI_Box* b = push_struct(g.frame_arena, UI_Box);
		*b = {};
		return b;
	}
	var[b, ok] = map_get(g.box_map, key);
	if(!ok) {
		b = pool_push(g.boxes, {.key = key});
		map_set(g.box_map, key, b);
		u64 d = b - g.boxes.data;
		if(d < UI_KEY_TABLE_SIZE) {
			sparse_set_push(g.active_boxes, d);
		}
	}
	Assert(b > g.boxes.data && b <= g.boxes.data+g.boxes.max_idx);
	return b;
}

void ui_prune_stale_boxes() {
	var& g = st->ui;
	Loop(i, g.active_boxes.count) {
		u32 idx = g.active_boxes.dense[i];
		UI_Box* b = &g.boxes.data[idx];
		if(current_frame - b->last_frame_touched > UI_STALE_FRAMES) {
			sparse_set_remove(g.active_boxes, idx);
			pool_remove(g.boxes, b);
			map_remove(g.box_map, b->key);
		}
	}
}

void ui_init() {
	var& g = st->ui;
	g.frame_arena = arena_make();
	g.style.padding = 8.0f;
	g.style.gap = 4.0f;
	g.style.line_height = 18.0f;
	g.style.text_pad = 6.0f;
	g.style.bg_color = v4(0.16f, 0.16f, 0.18f, 1.0f);
	g.style.text_color = v4(0.92f, 0.92f, 0.92f, 1.0f);
	g.style.border_color = v4(0.30f, 0.30f, 0.33f, 1.0f);
	g.style.hot_color = v4(0.26f, 0.26f, 0.30f, 1.0f);
	g.style.active_color = v4(0.35f, 0.45f, 0.75f, 1.0f);
	g.style.accent_color = v4(0.30f, 0.55f, 0.90f, 1.0f);
}

void ui_begin_frame() {
	var& g = st->ui;
	g.prev_input = g.input;
	g.input = {
		.dt = time_dt,
		.mouse_pos = os_mouse_pos(),
		.mouse_down[0] = os_mouse_is_button_down(MouseButton_Left),
		.mouse_down[1] = os_mouse_is_button_down(MouseButton_Right),
		.mouse_down[2] = os_mouse_is_button_down(MouseButton_Middle),
	};
	arena_clear(g.frame_arena);
	array_clear(g.draw_cmds);
	g.root = ui_box_make(0, ui_size_px(os_window_width()), ui_size_px(os_window_height()), "###root");
	g.root->child_layout_axis = UI_Axis2_Y;
	ui_push_parent(g.root);
}

void ui_end_frame() {
	var& g = st->ui;
	ui_pop_parent();
	ui_layout_standalone(g.root);
	ui_layout_upward(g.root);
	ui_layout_downward(g.root);
	g.root->rect = Rng2(v2(0, 0), v2(g.root->computed_size[UI_Axis2_X], g.root->computed_size[UI_Axis2_Y]));
	ui_layout_positions(g.root);
	ui_build_draw_cmds(g.root);
	ui_prune_stale_boxes();
}

void ui_layout_standalone(UI_Box* box) {
	var& g = st->ui;
	LoopEnum(axis, UI_Axis2) {
		UI_Size sz = box->semantic_size[axis];
		if(sz.type == UI_SizeType_Pixels) {
			box->computed_size[axis] = sz.value;
		} else if(sz.type == UI_SizeType_TextContent) {
			if(axis == UI_Axis2_X)
				box->computed_size[axis] = ui_text_measure(ui_display_string(box->string), g.style.font, FONT_SIZE) + 2.0f * g.style.text_pad;
			else
				box->computed_size[axis] = g.style.line_height + 2.0f * g.style.text_pad;
		}
	}
	LoopNode(it, box->first) {
		ui_layout_standalone(it);
	}
}

void ui_layout_upward(UI_Box* box) {
	LoopEnum(axis, UI_Axis2) {
		UI_Size sz = box->semantic_size[axis];
		if(sz.type == UI_SizeType_PercentOfParent) {
			f32 parent_size = box->parent ? box->parent->computed_size[axis] : box->computed_size[axis];
			box->computed_size[axis] = parent_size * sz.value;
		}
	}
	LoopNode(it, box->first) {
		ui_layout_upward(it);
	}
}

void ui_layout_downward(UI_Box* box) {
	var& g = st->ui;
	LoopNode(it, box->first) {
		ui_layout_downward(it);
	}
	LoopEnum(axis, UI_Axis2) {
		UI_Size sz = box->semantic_size[axis];
		if(sz.type == UI_SizeType_ChildrenSum) {
			f32 sum = 0, mx = 0;
			u32 n = 0;
			LoopNode(it, box->first) {
				if(it->semantic_size[axis].type == UI_SizeType_PercentOfParent) continue;
				sum += it->computed_size[axis];
				mx = Max(mx, it->computed_size[axis]);
				n++;
			}
			if(axis == box->child_layout_axis) {
				f32 gaps = n > 1 ? (f32)(n - 1) * g.style.gap : 0;
				box->computed_size[axis] = sum + gaps + 2.0f * g.style.padding;
			} else {
				box->computed_size[axis] = mx + 2.0f * g.style.padding;
			}
		}
	}
}

void ui_layout_positions(UI_Box* box) {
	var& g = st->ui;
	f32 pad = g.style.padding;
	f32 gap = g.style.gap;
	f32 cursor = pad;
	LoopNode(it, box->first) {
		f32 main = cursor;
		f32 cross = pad;
		if(box->child_layout_axis == UI_Axis2_X) {
			it->rect.x0 = box->rect.x0 + main;
			it->rect.y0 = box->rect.y0 + cross;
		} else {
			it->rect.x0 = box->rect.x0 + cross;
			it->rect.y0 = box->rect.y0 + main;
		}
		it->rect.x1 = it->rect.x0 + it->computed_size[UI_Axis2_X];
		it->rect.y1 = it->rect.y0 + it->computed_size[UI_Axis2_Y];
		cursor += it->computed_size[box->child_layout_axis] + gap;
		ui_layout_positions(it);
	}
}

void ui_build_draw_cmds(UI_Box* box) {
	var& g = st->ui;
	if(box->flags & UI_BoxFlag_DrawBackground) {
		v4 color = box->background_color;
		if((box->flags & UI_BoxFlag_ActiveAnimation) && box->active_t > 0.001f)
			color = v3_to_v4(v3_lerp(color.xyz, box->active_t, g.style.active_color.xyz), 1);
		else if((box->flags & UI_BoxFlag_HotAnimation) && box->hot_t > 0.001f)
			color = v3_to_v4(v3_lerp(color.xyz, box->hot_t, g.style.hot_color.xyz), 1);
		array_push(g.draw_cmds, {UI_DrawCmdType_Rect, box->rect, color, true});
		r_draw_rect(box->rect, u32_from_rgba(color), 0,0,0);
	}
	if(box->flags & UI_BoxFlag_DrawBorder) {
		array_push(g.draw_cmds, {UI_DrawCmdType_Rect, box->rect, box->border_color});
		r_draw_rect(box->rect, u32_from_rgba(box->border_color), 0,4,0);
	}
	if(box->flags & UI_BoxFlag_DrawText) {
		array_push(g.draw_cmds, {UI_DrawCmdType_Text, box->rect, box->text_color, false, ui_display_string(box->string)});
		r_draw_text_ext(g.style.font, v2(box->rect.x0, box->rect.y1), box->string, u32_from_rgba(box->text_color), FONT_SIZE);
	}
	LoopNode(it, box->first)
		ui_build_draw_cmds(it);
}

UI_Signal ui_signal_from_box(UI_Box* box) {
	var& g = st->ui;
	UI_Signal sig = {};
	sig.box = box;

	if(!(box->flags & UI_BoxFlag_Clickable)) return sig;

	/* Hit test against LAST FRAME's rect -- this frame's layout for
	 * `box` hasn't run yet. This is the one-frame lag mentioned up top. */
	b32 hovering = rng2_contains(box->rect, g.input.mouse_pos);
	b32 mouse_down_now = g.input.mouse_down[0];
	b32 mouse_down_prev = g.prev_input.mouse_down[0];
	b32 mouse_pressed_edge = mouse_down_now && !mouse_down_prev;
	b32 mouse_released_edge = !mouse_down_now && mouse_down_prev;

	sig.hovering = hovering;

	if(hovering)
		g.hot_key = box->key;
	else if(g.hot_key == box->key)
		g.hot_key = 0;

	if(hovering && mouse_pressed_edge) {
		g.active_key = box->key;
		sig.pressed = true;
	}

	b32 is_active = (g.active_key == box->key);
	if(is_active && mouse_down_now) {
		sig.dragging = true;
		sig.drag_delta = g.input.mouse_pos - g.prev_input.mouse_pos;
	}
	if(is_active && mouse_released_edge) {
		sig.released = true;
		sig.clicked = hovering; /* only counts as a click if released back over the box */
		g.active_key = 0;
	}

	f32 hot_target = (g.hot_key == box->key) ? 1.0f : 0.0f;
	f32 active_target = (g.active_key == box->key) ? 1.0f : 0.0f;
	box->hot_t = exp_decay(box->hot_t, hot_target, 10.0f, g.input.dt);
	box->active_t = exp_decay(box->active_t, active_target, 15.0f, g.input.dt);

	return sig;
}

UI_Signal ui_label(String string) {
	UI_Box* box = ui_box_make(UI_BoxFlag_DrawText, ui_size_text(), ui_size_text(), string);
	return ui_signal_from_box(box); /* not clickable -- returns an all-false signal */
}

UI_Signal ui_button(String string) {
	UI_Box* box = ui_box_make(
		UI_BoxFlag_Clickable | UI_BoxFlag_DrawBorder | UI_BoxFlag_DrawBackground |
		UI_BoxFlag_DrawText | UI_BoxFlag_HotAnimation | UI_BoxFlag_ActiveAnimation,
		ui_size_text(), ui_size_text(), string);
	return ui_signal_from_box(box);
}

b32 ui_checkbox(String string, b32* value) {
	var& g = st->ui;
	UI_Box* box = ui_box_make(
		UI_BoxFlag_Clickable | UI_BoxFlag_DrawBorder | UI_BoxFlag_DrawBackground |
		UI_BoxFlag_HotAnimation | UI_BoxFlag_ActiveAnimation,
		ui_size_px(20), ui_size_px(20), string);
	UI_Signal sig = ui_signal_from_box(box);
	if(sig.clicked) *value = !*value;
	if(*value) box->background_color = g.style.accent_color;
	return *value;
}

f32 ui_slider(String string, f32* value, f32 min, f32 max) {
	var& g = st->ui;
	UI_Box* track = ui_box_make(
		UI_BoxFlag_Clickable | UI_BoxFlag_DrawBorder | UI_BoxFlag_DrawBackground |
		UI_BoxFlag_HotAnimation | UI_BoxFlag_ActiveAnimation,
		ui_size_pct(1.0f), ui_size_px(20), string);
	UI_Signal sig = ui_signal_from_box(track);
	if(sig.dragging || sig.pressed) {
		f32 w = rng2_width(track->rect);
		f32 t = w > 0.0f ? (g.input.mouse_pos.x - track->rect.x0) / w : 0.0f;
		t = Clamp01(t);
		*value = min + t * (max - min);
	}
	return *value;
}

void ui_spacer(UI_Size size_along_parent_axis) {
	UI_Box* parent = ui_top_parent();
	UI_Axis2 axis = parent ? parent->child_layout_axis : UI_Axis2_X;
	UI_Size sizes[2] = {ui_size_px(0), ui_size_px(0)};
	sizes[axis] = size_along_parent_axis;
	ui_box_make(0, sizes[UI_Axis2_X], sizes[UI_Axis2_Y], {});
}

UI_Box* ui_panel_begin_sized(String string, UI_Axis2 child_layout_axis, UI_Size size_x, UI_Size size_y) {
	UI_Box* box = ui_box_make(
		UI_BoxFlag_DrawBackground | UI_BoxFlag_DrawBorder,
		size_x, size_y, string);
	box->child_layout_axis = child_layout_axis;
	ui_push_parent(box);
	return box;
}

UI_Box* ui_panel_begin(String string, UI_Axis2 child_layout_axis) {
	return ui_panel_begin_sized(string, child_layout_axis, ui_size_children(), ui_size_children());
}
void ui_panel_end() { ui_pop_parent(); }



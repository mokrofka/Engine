#pragma once
#include "base.h"
#include "mem.h"

////////////////////////////////////////////////////////////////////////
// Array
template<typename T, i32 N> struct Array {
	u32 count;
	static constexpr i32 cap = N;
	T data[N];
	T& operator[](u32 idx) {
		Assert(idx < cap);
		return data[idx];
	}
	T* begin() { return data; }
	T* end()   { return data + count; }
};

template<typename T, i32 N> NO_DEBUG Slice<T> slice(Array<T, N>& arr) {
	return {arr.data, arr.count};
}
template<typename T, i32 N> u32 array_push_empty(Array<T, N>& arr) {
	Assert(arr.count < arr.cap);
	return arr.count++;
}
template<typename T, i32 N> u32 array_push(Array<T, N>& arr, T a) {
	Assert(arr.count < arr.cap);
	arr.data[arr.count] = a;
	return arr.count++;
}
template<typename T, i32 N> void array_push_elems(Array<T, N>& arr, Slice<T> elems) {
	Assert(arr.count + elems.count <= arr.cap);
	MemCopyArray(arr.data + arr.count, elems.data, elems.count);
	arr.count += elems.count;
}
template<typename T, i32 N> void array_swap_remove(Array<T, N>& arr, u32 idx) {
	Assert(idx < arr.count);
	arr.data[idx] = arr.data[--arr.count];
}
template<typename T, i32 N> void array_clear(Array<T, N>& arr) {
	arr.count = 0;
}
template<typename T, i32 N> T array_pop(Array<T, N>& arr) {
	return arr.data[--arr.count];
}
template<typename T, i32 N> T array_back(Array<T, N>& arr) {
	return arr.data[arr.count-1];
}

///////////////////////////////////
// Darray
template <typename T> struct DArray {
	u32 count;
	u32 cap;
	Allocator alloc;
	T* data;
	T& operator[](u32 idx) {
		Assert(idx < cap);
		return data[idx];
	}
	T* begin() { return data; }
	T* end()   { return data + count; }
};

template<typename T> Slice<T> NO_DEBUG slice(DArray<T>& arr) { return {arr.data, arr.count}; }
template<typename T> DArray<T> array_make(Allocator alloc) {
	DArray<T> res = {
		.alloc = alloc,
	};
	return res;
}
template<typename T> void array_destroy(DArray<T>& arr) {
	if(arr.data) { mem_free(arr.alloc, arr.data); } 
}
template<typename T> void array_grow(DArray<T>& arr, u32 elem_count) {
	if(arr.data) {
		u32 old_cap = arr.cap;
		arr.cap = Max(arr.cap * DEFAULT_RESIZE_FACTOR, arr.count + elem_count);
		arr.data = mem_realloc_array(arr.alloc, arr.data, old_cap, arr.cap);
	} else {
		arr.cap = Max(DEFAULT_CAPACITY, elem_count);
		arr.data = push_array(arr.alloc, T, arr.cap);
	}
}
template<typename T> void array_reserve(DArray<T>& arr, u32 min_cap) {
	if(arr.cap >= min_cap) return;
	u32 old_cap = arr.cap;
	u32 new_cap = Max(old_cap * DEFAULT_RESIZE_FACTOR, min_cap);
	if(arr.data) {
		arr.data = mem_realloc_array(arr.alloc, arr.data, old_cap, new_cap);
	} else {
		arr.data = push_array(arr.alloc, T, new_cap);
	}
	arr.cap = new_cap;
}
template<typename T> T array_clone(DArray<T>& arr, Allocator alloc) {
	DArray<T> result = {
		.count = arr.count,
		.cap = arr.cap,
		.alloc = alloc,
		.data = push_array(alloc, T, arr.cap),
	};
	MemCopyArray(result.data, arr.data, arr.count);
	return result;
}
template<typename T> u32 array_push_empty(DArray<T>& arr) {
	if(arr.count >= arr.cap) {
		array_grow(arr, 0);
	}
	return arr.count++;
}
template<typename T> u32 array_push(DArray<T>& arr, T a) {
	if(arr.count >= arr.cap) {
		array_grow(arr, 0);
	}
	arr.data[arr.count] = a;
	return arr.count++;
}
template<typename T> void array_push_elems(DArray<T>& arr, Slice<T> elems) {
	if(arr.count + elems.count >= arr.cap) {
		array_grow(arr, elems.count);
	}
	MemCopyArray(arr.data + arr.count, elems.data, elems.count);
	arr.count += elems.count;
}
template<typename T> void array_swap_remove(DArray<T>& arr, u32 idx) {
	Assert(idx < arr.count);
	arr.data[idx] = arr.data[--arr.count];
}
template<typename T> void array_clear(DArray<T>& arr) {
	arr.count = 0; 
}
template<typename T> T array_pop(DArray<T>& arr) {
	return arr.data[--arr.count];
}
template<typename T> T array_back(DArray<T>& arr) {
	return arr.data[arr.count-1];
}

// ////////////////////////////////////////////////////////////////////////
// // ArrayHandler
// template<typename T, i32 N, typename Handle> struct ArrayHandler {
// 	static constexpr i32 cap = N;
// 	u32 count;
// 	u32 sparse[N];
// 	u32 dense[N];
// 	T data[N];
// 	u32 generations[N];
// };

// template<typename T, i32 N, typename Handle> T& array_handler_get(ArrayHandler<T, N, Handle>& a, Handle h) {
// 	Assert(h.idx < a.count);
// 	Assert(a.generations[h.idx]== id_generation(h.gen));
// 	u32 idx = a.sparse[h.idx];
// 	return a.data[idx];
// }
// template<typename T, i32 N, typename Handle> Handle array_handler_push(ArrayHandler<T, N, Handle>& a, T elem) {
// 	Assert(a.count < a.cap);
// 	u32 idx = a.count++;
// 	a.sparse[idx] = idx;
// 	a.dense[idx] = idx;
// 	a.data[idx] = elem;
// 	Handle res = {idx, a.generations[idx]};
// 	return res;
// }
// template<typename T, i32 N, typename Handle> void array_handler_remove(ArrayHandler<T, N, Handle>& a, Handle h) {
// 	Assert(h.idx < a.count);
// 	Assert(a.generations[h.idx]++ == h.gen);
// 	u32 idx_removed = a.sparse[h.idx];
// 	u32 idx_last = a.count - 1;
// 	a.data[idx_removed] = a.data[idx_last];
// 	u32 last_entity = a.dense[idx_last];
// 	a.sparse[last_entity] = idx_removed;
// 	a.dense[idx_removed] = last_entity;
// 	--a.count;
// }
// template<typename T, i32 N, typename Handle> void array_handler_clear(ArrayHandler<T, N, Handle>& arr) {
// 	arr.count = 0;
// }

// ///////////////////////////////////
// // DarrayHandler
// template <typename T, typename Handle> struct DarrayHandler {
// 	u32 count;
// 	u32 cap;
// 	Allocator alloc;
// 	u32* sparse;
// 	u32* dense;
// 	T* data;
// 	u32* generations;
// };

// template<typename T, typename Handle> DarrayHandler<T, Handle> array_handler_make(Allocator alloc) {
// 	DarrayHandler<T, Handle> res = {
// 		.alloc = alloc,
// 	};
// 	return res;
// }
// template<typename T, typename Handle> T& array_handler_get(DarrayHandler<T, Handle>& a, Handle h) {
// 	Assert(h.idx < a.cap);
// 	Assert(a.generations[h.idx] == h.gen);
// 	u32 idx = a.sparse[h.idx];
// 	return a.data[idx];
// }
// template<typename T, typename Handle> void array_handler_grow(DarrayHandler<T, Handle>& a) {
// 	if(a.data) {
// 		u32 cap_old = a.cap;
// 		a.cap *= DEFAULT_RESIZE_FACTOR;
// 		SoA_Field fields[] = {
// 			SoA_push_field(a.sparse),
// 			SoA_push_field(a.dense),
// 			SoA_push_field(a.data),
// 			SoA_push_field(a.generations),
// 		};
// 		mem_realloc_soa(a.alloc, cap_old, a.cap, slice(fields));
// 		MemZeroArray(a.generations+cap_old, a.cap-cap_old);
// 	} else {
// 		a.cap = DEFAULT_CAPACITY;
// 		SoA_Field fields[] = {
// 			SoA_push_field(a.sparse),
// 			SoA_push_field(a.dense),
// 			SoA_push_field(a.data),
// 			SoA_push_field(a.generations),
// 		};
// 		mem_alloc_soa(a.alloc, a.cap, slice(fields));
// 		MemZeroArray(a.generations, a.cap);
// 	}
// }
// template<typename T, typename Handle> Handle array_handler_push(DarrayHandler<T, Handle>& a, T elem) {
// 	if(a.count >= a.cap) {
// 		array_handler_grow(a);
// 	}
// 	u32 idx = a.count++;
// 	a.sparse[idx] = idx;
// 	a.dense[idx] = idx;
// 	a.data[idx] = elem;
// 	Handle res = {idx, a.generations[idx]};
// 	return res;
// }
// template<typename T, typename Handle> void array_handler_remove(DarrayHandler<T, Handle>& a, Handle h) {
// 	Assert(a.generations[h.idx]++ == h.gen);
// 	u32 idx_removed = a.sparse[h.idx];
// 	u32 idx_last = a.count - 1;
// 	a.data[idx_removed] = a.data[idx_last];
// 	u32 last_entity = a.dense[idx_last];
// 	a.sparse[last_entity] = idx_removed;
// 	a.dense[idx_removed] = last_entity;
// 	--a.count;
// }
// template<typename T, typename Handle> void array_handler_clear(DarrayHandler<T, Handle>& arr) {
// 	arr.count = 0;
// }

////////////////////////////////////////////////////////////////////////
// SparseSet
template<i32 N> struct SparseSet {
	u32 count;
	u32 sparse[N];
	u32 dense[N];
};

template<i32 N> void sparse_set_push(SparseSet<N>& a, u32 id) {
	Assert(a.count < N);
	a.sparse[id] = a.count;
	a.dense[a.count] = id;
	a.count++;
}
template<i32 N> void sparse_set_remove(SparseSet<N>& a, u32 id) {
	u32 idx_removed = a.sparse[id];
	u32 idx_last = a.count - 1;
	u32 last_entity = a.dense[idx_last];
	a.sparse[last_entity] = idx_removed;
	a.dense[idx_removed] = last_entity;
	--a.count;
}

template<typename T> struct NodeIter {
	T* nodes;
	u32 idx;
	operator b32()    { return idx != 0; }
	void operator++() { idx = nodes[idx].next; }
	T& operator*()    { return nodes[idx]; }
};
template<typename T> NodeIter<T> node_iter_begin(T* nodes, u32 first) { return {nodes, first}; }
template<typename T, typename Handle> struct HNodeIter {
	T* nodes;
	Handle id;
	operator b32()    { return id.idx != 0; }
	void operator++() { id = nodes[id.idx].next; }
	T& operator*()    { return nodes[id.idx]; }
};
template<typename T, typename Handle> HNodeIter<T,Handle> node_iter_begin(T* nodes, Handle first) { return {nodes, first}; }

////////////////////////////////////////////////////////////////////////
// PoolPtr
template<typename T, i32 N> struct PoolPtr {
	static_assert(sizeof(T) >= 4);
	u32 head;
	static constexpr i32 cap = N;
	u32 max_idx;
	T data[N];
};

template<typename T, i32 N> T* pool_push_empty(PoolPtr<T, N>& p) {
	u32 idx = p.head;
	if(idx > 0) {
		p.head = *(u32*)&p.data[idx];
	} else {
		idx = ++p.max_idx;
		Assert(idx < p.cap);
	}
	return &p.data[idx];
}
template<typename T, i32 N> T* pool_push(PoolPtr<T, N>& p, T a) {
	T* res = pool_push_empty(p);
	*res = a;
	return res;
}
template<typename T, i32 N> void pool_remove(PoolPtr<T, N>& p, T* ptr) {
	i64 idx = ptr - p.data;
	Assert(idx > 0 && idx < p.cap);

	*(u32*)ptr = p.head;
	p.head = idx;
}
template<typename T, i32 N> void pool_clear(PoolPtr<T, N>& p) {
	p.head = 0;
	p.max_idx = 0;
	ArrayZero(p.data);
}

////////////////////////////////////////////////////////////////////////
// Poolu32
template<typename T, i32 N> struct Poolu32 {
	static_assert(sizeof(T) >= 4);
	u32 head;
	static constexpr i32 cap = N;
	u32 max_idx;
	T data[N];
};

template<typename T, i32 N> T& pool_get(Poolu32<T, N>& p, u32 idx) {
	Assert(idx > 0);
	return p.data[idx];
}
template<typename T, i32 N> u32 pool_push_empty(Poolu32<T, N>& p) {
	u32 idx = p.head;
	if(idx > 0) {
		p.head = *(u32*)&p.data[idx];
	} else {
		idx = ++p.max_idx;
		Assert(idx < p.cap);
	}
	return idx;
}
template<typename T, i32 N> u32 pool_push(Poolu32<T, N>& p, T a) {
	u32 idx = pool_push_empty(p);
	p.data[idx] = a;
	return idx;
}
template<typename T, i32 N> void pool_remove(Poolu32<T, N>& p, u32 idx) {
	*(u32*)&p.data[idx] = p.head;
	p.head = idx;
}
template<typename T, i32 N> u32 pool_clear(Poolu32<T, N>& p) {
	p.head = 0;
	p.max_idx = 0;
	ArrayZero(p.data);
}

////////////////////////////////////////////////////////////////////////
// Pool
template<typename T, i32 N, typename Handle> struct Pool {
	static_assert(sizeof(T) >= 4);
	u32 head;
	static constexpr i32 cap = N;
	u32 max_idx;
	T data[N];
	u32 gens[N];
};

template<typename T, i32 N, typename Handle> T& pool_get(Pool<T, N, Handle>& p, Handle h) {
	Assert(pool_is_valid_handle(p, h));
	return p.data[h.idx];
}
template<typename T, i32 N, typename Handle> Handle pool_push_empty(Pool<T, N, Handle>& p) {
	u32 idx = p.head;
	if(idx > 0) {
		p.head = *(u32*)&p.data[idx];
	} else {
		idx = ++p.max_idx;
		Assert(idx < p.cap);
	}
	Handle res = {idx, p.gens[idx]};
	return res;
}
template<typename T, i32 N, typename Handle> Handle pool_push(Pool<T, N, Handle>& p, T a) {
	Handle h = pool_push_empty(p);
	p.data[h.idx] = a;
	return h;
}
template<typename T, i32 N, typename Handle> void pool_remove(Pool<T, N, Handle>& p, Handle h) {
	Assert(pool_is_valid_handle(p, h));
	p.gens[h.idx]++;
	*(u32*)&p.data[h.idx] = p.head;
	p.head = h.idx;
}
template<typename T, i32 N, typename Handle> Handle pool_get_handle(Pool<T, N, Handle>& p, u32 idx) {
	Handle res = {idx, p.gens[idx]};
	return res;
}
template<typename T, i32 N, typename Handle> void pool_clear(Pool<T, N, Handle>& p) {
	p.head = 0;
	p.max_idx = 0;
	ArrayZero(p.data);
	ArrayZero(p.gens);
}
template <typename T, i32 N, typename Handle> b32 pool_is_valid_handle(Pool<T, N, Handle>& p, Handle h) {
	if(h.idx == 0 || p.gens[h.idx] != h.gen) {
		return false;
	}
	return true;
}

///////////////////////////////////
// Dpool
template<typename T, typename Handle> struct DPool {
	static_assert(sizeof(T) >= 4);
	u32 head;
	u32 cap;
	u32 max_idx;
	Allocator alloc;
	T* data;
	u32* gens;
};

template<typename T, typename Handle> DPool<T, Handle> pool_make(Allocator alloc) {
	DPool<T, Handle> res = {
		.alloc = alloc,
	};
	return res;
}
template<typename T, typename Handle> T& pool_get(DPool<T, Handle>& p, Handle h) {
	Assert(pool_is_valid_handle(p, h));
	return p.data[h.idx];
}
template<typename T, typename Handle> void pool_grow(DPool<T, Handle>& p) {
	if(p.data) {
		u32 cap_old = p.cap;
		p.cap *= DEFAULT_RESIZE_FACTOR;
		SoA_Field fields[] = {
			SoA_push_field(p.data),
			SoA_push_field(p.gens),
		};
		mem_realloc_soa_zero(p.alloc, cap_old, p.cap, slice(fields));
	} else {
		p.cap = DEFAULT_CAPACITY;
		SoA_Field fields[] = {
			SoA_push_field(p.data),
			SoA_push_field(p.gens),
		};
		mem_alloc_soa_zero(p.alloc, p.cap, slice(fields));
	}
}
template<typename T, typename Handle> Handle pool_push_empty(DPool<T, Handle>& p) {
	u32 idx = p.head;
	if(idx > 0) {
		p.head = *(u32*)&p.data[idx];
	} else {
		idx = ++p.max_idx;
		if(idx >= p.cap) {
			pool_grow(p);
		}
	}
	Handle res = {idx, p.gens[idx]};
	return res;
}
template<typename T, typename Handle> Handle pool_push(DPool<T, Handle>& p, T a) {
	Handle h = pool_push_empty(p);
	p.data[h.idx] = a;
	return h;
}
template<typename T, typename Handle> void pool_remove(DPool<T, Handle>& p, Handle h) {
	Assert(pool_is_valid_handle(p, h));
	p.gens[h.idx]++;
	*(u32*)&p.data[h.idx] = p.head;
	p.head = h.idx;
}
template<typename T, typename Handle> u32 pool_clear(DPool<T, Handle>& p) {
	p.head = 0;
	p.max_idx = 0;
	MemZeroArray(p.data, p.max_idx);
	MemZeroArray(p.gens, p.max_idx);
}
template<typename T, typename Handle> b32 pool_is_valid_handle(DPool<T, Handle>& p, Handle h) {
	if(h.idx == 0 || p.gens[h.idx] != h.gen) {
		return false;
	}
	return true;
}

///////////////////////////////////
// PoolIterative
template<typename T, i32 N, typename Handle> struct PoolIterative {
	Pool<T, N, Handle> pool;
	SparseSet<N> set;
};

template <typename T, i32 N, typename Handle> struct PoolIterativeIter {
	PoolIterative<T, N, Handle>* pool;
	u32 idx;
	operator b32()    { if(idx < pool->set.count) { return true; } return false; }
	T& operator*()    { return pool->pool.data[pool->set.dense[idx]]; }
	void operator++() { idx++; }
	Handle id()       { return pool_get_handle(pool->pool, pool->set.dense[idx]); }
};
template<typename T, i32 N, typename Handle> PoolIterativeIter<T,N,Handle> pool_begin(PoolIterative<T,N,Handle>& pool) {
	return {&pool};
}

template<typename T, i32 N, typename Handle> T& pool_get(PoolIterative<T, N, Handle>& p, Handle h) {
	return pool_get(p.pool, h);
}
template<typename T, i32 N, typename Handle> Handle pool_push(PoolIterative<T, N, Handle>& p, T a) {
	Handle h = pool_push(p.pool, a);
	sparse_set_push(p.set, h.idx);
	return h;
}
template<typename T, i32 N, typename Handle> void pool_remove(PoolIterative<T, N, Handle>& p, Handle h) {
	sparse_set_remove(p.set, h.idx);
	pool_remove(p.pool, h);
}

////////////////////////////////////////////////////////////////////////
// Queue
template<typename T, i32 N> struct Queue {
	static constexpr u32 cap = N;
	u32 read;
	u32 write;
	T data[N];
};

template<typename T, i32 N> void queue_push(Queue<T, N>& q, T elem) {
	Assert(q.write - q.read < q.cap);
	q.data[q.write++ % q.cap] = elem;
}
template<typename T, i32 N> T queue_pop(Queue<T, N>& q) {
	Assert(q.write != q.read);
	return q.data[q.read++ % q.cap];
}
template<typename T, i32 N> T queue_back(Queue<T, N>& q) {
	Assert(q.write - q.read < q.cap);
	return q.data[(q.write-1) % q.cap];
}
template<typename T, i32 N> T queue_front(Queue<T, N>& q) {
	Assert(q.write != q.read);
	return q.data[(q.read) % q.cap];
}
template<typename T, i32 N> u32 queue_count(Queue<T, N>& q) { return q.write - q.read; }

template<typename T, i32 N> struct QueueSPSC {
	static constexpr u32 cap = N;
	alignas(64) u32 read;
	alignas(64) u32 write;
	T data[N];
};

template<typename T, i32 N> void queue_push(QueueSPSC<T, N>& q, T elem) {
	u32 write = atomic_load_explicit(&q.write, AtomicRelaxed);
	u32 read = atomic_load_explicit(&q.read, AtomicAcquire);
	// if(write - read >= q.cap) return false;
	Assert(write - read < q.cap);
	q.data[write % q.cap] = elem;
	atomic_inc_explicit(&q.write, AtomicRelease);
	// return true;
}
template<typename T, i32 N> T queue_pop(QueueSPSC<T, N>& q) {
	u32 read = atomic_load_explicit(&q.read, AtomicRelaxed);
	u32 write = atomic_load_explicit(&q.write, AtomicAcquire);   
	// if (read == write) return {};
	Assert(read != write);
	T res = q.data[read % q.cap];
	atomic_inc_explicit(&q.read, AtomicRelease);
	// return {res, true};
	return res;
}
template<typename T, i32 N> T queue_back(QueueSPSC<T, N>& q) {
	u32 write = atomic_load_explicit(&q.write, AtomicRelaxed);
	u32 read = atomic_load_explicit(&q.read, AtomicAcquire);
	// if(write - read >= q.cap) return {};
	Assert(write - read < q.cap);
	// return {q.data[(q.write-1) % q.cap], true};
	return q.data[(q.write-1) % q.cap];
}
template<typename T, i32 N> T queue_front(QueueSPSC<T, N>& q) {
	u32 read = atomic_load_explicit(&q.read, AtomicRelaxed);
	u32 write = atomic_load_explicit(&q.write, AtomicAcquire);   
	// if(read == write) return {};
	Assert(read != write);
	// return {q.data[(q.read) % q.cap], true};
	return q.data[(q.read) % q.cap];
}
template<typename T, i32 N> u32 queue_count(QueueSPSC<T, N>& q) { return atomic_load_explicit(&q.write, AtomicRelaxed) - atomic_load_explicit(&q.read, AtomicAcquire); }

template<typename T, i32 N> struct QueueMPMC {
	static constexpr u32 cap = N;
	struct Slot {
		T elem;
		u64 seq;
	};
	alignas(64) u32 read;
	alignas(64) u32 write;
	Slot data[N];
};

template<typename T, i32 N> QueueMPMC<T, N> queue_mpmc_make() {
	QueueMPMC<T, N> res = {};
	Loop(i, N) res.data[i].seq = i;
	return res;
}
template<typename T, i32 N> b32 queue_push(QueueMPMC<T, N>& q, T elem) {
	For {
		u32 write = atomic_load(&q.write);
		var& slot = q.data[write % q.cap];
		u64 seq = atomic_load(&slot.seq);
		i64 dif = (i64)seq - (i64)write;
		if(dif == 0) {
			if(atomic_cmp_swap(&q.write, &write, write+1)) {
				slot.elem = elem;
				atomic_store(&slot.seq, write+1);
				return true;
			}
		} else if(dif < 0) return false; // full
	}
}
template<typename T, i32 N> ResultOk<T> queue_pop(QueueMPMC<T, N>& q) {
	For {
		u32 read = atomic_load(&q.read);
		var& slot = q.data[read % q.cap];
		u64 seq = atomic_load(&slot.seq);
		i64 dif = (i64)seq - (i64)(read+1);
		if(dif == 0) {
			if(atomic_cmp_swap(&q.read, &read, read+1)) {
				T res = slot.elem;
				atomic_store(&slot.seq, read + N);
				return {res, true};
			}
		} else if(dif < 0) return {}; // empty
	}
}

////////////////////////////////////////////////////////////////////////
// IdPool
template<i32 N> struct IdPool {
	u32 count;
	static constexpr i32 cap = N;
	u32 ids[N];
};

template<i32 N> void id_pool_init(IdPool<N>& p)            { Loop(i, p.cap) p.ids[i] = i; }
template<i32 N> u32 id_pool_push(IdPool<N>& p)             { Assert(p.count <= p.cap); return p.ids[p.count++]; }
template<i32 N> void id_pool_remove(IdPool<N>& p, u32 id)  { p.ids[--p.count] = id; }
template<i32 N> void id_pool_clear(IdPool<N>& p)           { Loop(i, p.cap) p.ids[i] = i; p.count = 0; }

struct DIdPool {
	u32 count;
	u32 cap;
	u32* ids;
	Allocator alloc;
};

DIdPool id_pool_make(Allocator alloc);
u32 id_pool_push(DIdPool& p);
void id_pool_remove(DIdPool& p, u32 id);
void id_pool_destroy(DIdPool& p);
void id_pool_clear(DIdPool& p);

////////////////////////////////////////////////////////////////////////
// Map (hope u64 won't be collide)
enum MapSlot : u8 {
	MapSlot_Empty,
	MapSlot_Occupied,
	MapSlot_Deleted
};
template<typename V, i32 N> struct Map {
	u32 count;
	static constexpr u32 cap = N;
	V data[N];
	u64 hashes[N];
	MapSlot is_occupied[N];
};

template<typename V, i32 N> V* map_set(Map<V, N>& m, u64 hash, V val) {
	Assert(m.count < m.cap);
	u64 idx = mod_pow2(hash, m.cap);
	while(m.is_occupied[idx] == MapSlot_Occupied) {
		if(m.hashes[idx] == hash) break;
		idx = mod_pow2(idx + 1, m.cap);
	}
	m.hashes[idx] = hash;
	m.data[idx] = val;
	m.is_occupied[idx] = MapSlot_Occupied;
	m.count++;
	return &m.data[idx];
}
template<typename V, i32 N> ResultOk<V> map_get(Map<V, N>& m, u64 hash) {
	u64 idx = mod_pow2(hash, m.cap);
	Loop(i, m.cap) {
		if((m.is_occupied[idx] == MapSlot_Occupied) && (m.hashes[idx] == hash)) {
			return {m.data[idx], true};
		} 
		else if(m.is_occupied[idx] == MapSlot_Empty) {
			break;
		}
		idx = mod_pow2(idx + 1, m.cap);
	}
	return {};
}
template<typename V, i32 N> void map_remove(Map<V, N>& m, u64 hash) {
	u64 idx = mod_pow2(hash, m.cap);
	Loop(i, m.cap) {
		if((m.is_occupied[idx] == MapSlot_Occupied) && (m.hashes[idx] == hash)) {
			m.is_occupied[idx] = MapSlot_Deleted;
			m.count--;
			return;
		}
		idx = mod_pow2(idx + 1, m.cap);
	}
}
template<typename V, i32 N> void map_clear(Map<V, N>& m, u64 hash) {
	m.count = 0;
	u8* start = (u8*)m.data;
	u8* end = Offset(m.is_occupied, m.cap * sizeof(MapSlot));
	MemZero(start, u64(end - start));
}

///////////////////////////////////
// Dmap
template<typename V> struct Dmap {
	static constexpr f32 LF = 0.8;
	u32 count;
	u32 cap;
	Allocator alloc;
	V* data;
	u64* hashes;
	MapSlot* is_occupied;
};

template<typename V> Dmap<V> map_make(Allocator alloc) {
	Dmap<V> res = {
		.alloc = alloc,
	};
	return res;
}
template<typename V> void map_grow(Dmap<V>& m) {
	if(m.data) {
		V* old_data = m.data;
		u64* old_hashes = m.hashes;
		MapSlot* old_is_occupied = m.is_occupied;
		u32 old_cap = m.cap;
		m.cap *= DEFAULT_RESIZE_FACTOR;
		SoA_Field fields[] = {
			SoA_push_field(m.data),
			SoA_push_field(m.hashes),
			SoA_push_field(m.is_occupied),
		};
		mem_alloc_soa(m.alloc, m.cap, slice(fields));
		Loop(i, old_cap) {
			if(old_is_occupied[i] == MapSlot_Occupied) {
				map_set(m, old_hashes[i], old_data[i]);
			}
		}
		mem_free(m.alloc, old_data, mem_soa_size(old_cap, slice(fields)));
	} else {
		m.cap = DEFAULT_CAPACITY;
		SoA_Field fields[] = {
			SoA_push_field(m.data),
			SoA_push_field(m.hashes),
			SoA_push_field(m.is_occupied),
		};
		mem_alloc_soa(m.alloc, m.cap, slice(fields));
	}
}
template<typename V> V* map_set(Dmap<V>& m, u64 hash, V val) {
	if(m.count >= m.cap*m.LF) { map_grow(m); }
	u64 idx = mod_pow2(hash, m.cap);
	while(m.is_occupied[idx] == MapSlot_Occupied) {
		if(m.hashes[idx] == hash) break;
		idx = mod_pow2(idx + 1, m.cap);
	}
	m.hashes[idx] = hash;
	m.data[idx] = val;
	m.is_occupied[idx] = MapSlot_Occupied;
	m.count++;
	return &m.data[idx];
}
template<typename V> ResultOk<V> map_get(Dmap<V>& m, u64 hash) {
	if(!m.data) return {};
	u64 idx = mod_pow2(hash, m.cap);
	Loop(i, m.cap) {
		if((m.is_occupied[idx] == MapSlot_Occupied) && (m.hashes[idx] == hash)) {
			return {m.data[idx], true};
		} 
		else if(m.is_occupied[idx] == MapSlot_Empty) {
			break;
		}
		idx = mod_pow2(idx + 1, m.cap);
	}
	return {};
}
template<typename V> void map_remove(Dmap<V>& m, u64 hash) {
	u64 idx = mod_pow2(hash, m.cap);
	Loop(i, m.cap) {
		if((m.is_occupied[idx] == MapSlot_Occupied) && (m.hashes[idx] == hash)) {
			m.is_occupied[idx] = MapSlot_Deleted;
			m.count--;
			return;
		}
		idx = mod_pow2(idx + 1, m.cap);
	}
}
template<typename V> void map_clear(Dmap<V>& m) {
	m.count = 0;
	u8* start = (u8*)m.data;
	u8* end = Offset(m.is_occupied, m.cap * sizeof(MapSlot));
	MemZero(start, u64(end - start));
}

////////////////////////////////////////////////////////////////////////
// Sort

///////////////////////////////////
// Insert
template<typename T, typename Cmp> void sort_insert(Slice<T> slice, Cmp cmp) {
	for(i32 i = 1; i < slice.count; i++) {
		T key = slice[i];
		i32 j = i - 1;
		while(j >= 0 && cmp(key, slice[j])) {
			slice[j + 1] = slice[j];
			j--;
		}
		slice[j + 1] = key;
	}
}

///////////////////////////////////
// Quick
template<typename T, typename Cmp> i32 _lomuto_partition(T* arr, i32 low, i32 high, Cmp cmp) {
	T pivot = arr[high];
	i32 i = low;
	for(i32 j = low; j < high; j++) {
		if(cmp(arr[j], pivot)) {
			Swap(arr[i], arr[j]);
			i++;
		}
	}
	Swap(arr[i], arr[high]);
	return i;
}
template<typename T, typename Cmp> void _quick_sort(Slice<T> arr, i32 low, i32 high, Cmp cmp) {
	if(low < high) {
		i32 p = _lomuto_partition(arr.data, low, high, cmp);
		_quick_sort(arr, low, p - 1, cmp);
		_quick_sort(arr, p + 1, high, cmp);
	}
}
template<typename T, typename Cmp> void sort_quick(Slice<T> arr, Cmp cmp) { _quick_sort(arr, 0, arr.count-1, cmp); }

///////////////////////////////////
// Merge
template<typename T, typename Cmp> void _merge(T* arr, T* tmp, u32 left, u32 mid, u32 right, Cmp cmp) {
	u32 i = left;
	u32 j = mid + 1;
	u32 k = left;
	while(i <= mid && j <= right) {
		if(!cmp(arr[j], arr[i])) {
			tmp[k++] = arr[i++];
		} else {
			tmp[k++] = arr[j++];
		}
	}
	while(i <= mid) {
		tmp[k++] = arr[i++];
	}
	while(j <= right) {
		tmp[k++] = arr[j++];
	}
	for(u32 x = left; x <= right; ++x) {
		arr[x] = tmp[x];
	}
}
template<typename T, typename Cmp> void _merge_sort(T* arr, T* tmp, u32 left, u32 right, Cmp cmp) {
	if(left >= right) return;
	u32 mid = left + (right - left) / 2;
	_merge_sort(arr, tmp, left, mid, cmp);
	_merge_sort(arr, tmp, mid + 1, right, cmp);
	_merge(arr, tmp, left, mid, right, cmp);
}
template<typename T, typename Cmp> void sort_merge(Allocator alloc, Slice<T> arr, Cmp cmp) {
	if(arr.count < 2) return;
	Slice tmp = push_slice(alloc, T, arr.count);
	_merge_sort(arr.data, tmp.data, 0, arr.count-1, cmp);
}

///////////////////////////////////
// Radix
struct SortEntry {
	u32 sort_key;
	u32 idx;
};

u32 sort_i32_key_to_u32(i32 x);
u32 sort_f32_key_to_u32(f32 sort_key);
void sort_radix(Allocator alloc, Slice<SortEntry> arr);

////////////////////////////////////////////////////////////////////////
// List sort
template<typename T, typename Cmp> Slice<T> sort_list_insert(Allocator arena, T first, Cmp cmp) {
	var sorted_arr = array_make<T>(arena);
	for(T it = first; it != 0; it = it->next) {
		array_push(sorted_arr, it);
	}
	sort_insert(slice(sorted_arr), cmp);
	return slice(sorted_arr);
}

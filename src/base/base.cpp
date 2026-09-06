#include "base_impl.h"

Extern f32 time_dt;
Extern f32 time_now;

////////////////////////////////////////////////////////////////////////
// Basic

f32 BytesToKB(u64 x) { return (f32)x / 1024; };
f32 BytesToMB(u64 x) { return BytesToKB(x) / 1024; };
f32 BytesToGB(u64 x) { return BytesToMB(x) / 1024; };

////////////////////////////////////////////////////////////////////////
// Memory

void MemSet(void *d, i32 byte, u64 size)  { __builtin_memset(d, byte, size); }
void MemZero(void *d, u64 size)           { MemSet(d, 0, size); }
void MemCopy(void* d, void* s, u64 size)  { __builtin_memcpy(d, s, size); }
b32  MemMatch(void* a, void* b, u64 size) { return (__builtin_memcmp(a, b, size) == 0); }

u64 AlignUp(u64 x, u64 a)           { return (x + a - 1) & ~(a - 1); }
u64 AlignDown(u64 x, u64 a)         { return x & ~(a - 1); }
u64 AlignPadUp(u64 x, u64 a)        { return -x & (a - 1); }
u64 AlignPadDown(u64 x, u64 a)      { return x & (a - 1); }
b32 IsAligned(u64 x, u64 a)         { return ((a - 1) & x) == 0; }
u8* PtrAlignUp(void* x, u64 a)      { return (u8*)AlignUp(u64(x), a); }
u8* PtrAlignDown(void* x, u64 a)    { return (u8*)AlignDown(u64(x), a); }
u8* PtrAlignPadUp(void* x, u64 a)   { return (u8*)AlignPadUp(u64(x), a); }
u8* PtrAlignPadDown(void* x, u64 a) { return (u8*)AlignPadDown(u64(x), a); }
b32 PtrIsAligned(void* x, u64 a)    { return  IsAligned(u64(x), a); }
b32 IsPow2(u64 x)                   { return ((x - 1) & x) == 0; }
u8* Offset(void* x, u64 a)          { return (u8*)x + a; }
u8* OffsetBack(void* x, u64 a)      { return (u8*)x - a; }
u64 PtrDiff(void* a, void* b)       { return (u8*)a - (u8*)a; }
b32 PtrMatch(void* a, void* b)      { return (u8*)a == (u8*)b; }

////////////////////////////////////////////////////////////////////////
// Bits

u32 clz(u64 v)                     { return __builtin_clzll(v); }
u32 ctz(u64 v)                     { return __builtin_ctzll(v); }
u32 count_bits_set(u64 v)          { return __builtin_popcountll(v); }
u32 most_significant_bitu32(u32 v) { return 31 - clz(v); }
u64 most_significant_bitu64(u64 v) { return 63 - clz(v); }
u32 remove_lowest_bit(u64 v)       { return v & (v - 1);}
u32 remove_highest_bitu32(u32 v)   { return v ^ (1u << most_significant_bitu32(v)); }
u64 remove_highest_bitu64(u64 v)   { return v ^ (1u << most_significant_bitu64(v)); }

b32 bit_has(u64 x, u64 pos)       { return x & (1 << pos); }
u64 flag_clear(u64 x, u64 f)      { return x & ~f; }
u64 flag_toggle(u64 x, u64 f)     { return x ^ f; }
b32 flag_has(u64 x, u64 f)        { return (x & f) == (f); }
b32 flag_any(u64 x, u64 f)        { return x & f; }

////////////////////////////////////////////////////////////////////////
// Common operations

u64 mod_pow2(u64 x, u64 b)   { return x & (b - 1); }
u64 div_pow2(u64 x, u64 b)   { return x >> ctz(b); }
u64 div_ceil(u64 x, u64 b)   { return (x + b - 1) / b; }
u64 round_up(u64 x, u64 a)   { return div_ceil(x, a) * a; }
u64 round_down(u64 x, u64 a) { return x / a * a; }
u64 compose_64(u64 a, u64 b) { return (a << 32) | b; }
u32 next_pow2(u32 v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}
u32 prev_pow2(u32 n) {
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n - (n >> 1);
}

////////////////////////////////////////////////////////////////////////
// Asserts

void Trap()      { __builtin_trap(); }
void DebugTrap() { __builtin_debugtrap(); }

////////////////////////////////////////////////////////////////////////
// Types

void bitset_set(BitSet& bits, u64 idx)   { bits.words[idx >> 6] |= Bit(idx & 63); }
void bitset_clear(BitSet& bits, u64 idx) { bits.words[idx >> 6] &= ~Bit(idx & 63); }
b32 bitset_get(BitSet& bits, u64 idx)    { return (bits.words[idx >> 6] >> (idx & 63)) & 1; }
u64 bitset_word_count(BitSet& bits)      { return (bits.bit_count + 63) / 64; }

RingBuffer ring_make(void* base, u64 size) {
	RingBuffer res = {
		.base = (u8*)base,
		.size = size,
	};
	return res;
}

u64 ring_write(RingBuffer& ring, void* src, u64 src_size) {
	Assert(ring.size >= src_size);
	u64 offset = ring.write_pos % ring.size;
	u64 first = Min(ring.size - offset, src_size);
	u64 second = src_size - first;
	MemCopy(ring.base + offset, src, first);
	if (second) {
		MemCopy(ring.base, Offset(src, first), second);
	}
	ring.write_pos += src_size;
	return offset;
}

u64 ring_read(RingBuffer& ring, void* dst, u64 dst_size) {
	Assert(ring.size >= dst_size);
	u64 offset = ring.read_pos % ring.size;
	u64 first = Min(ring.size - offset, dst_size);
	u64 second = dst_size - first;
	MemCopy(dst, ring.base+offset, first);
	if (second) {
		MemCopy(Offset(dst, first), ring.base, second);
	}
	ring.read_pos += dst_size;
	return offset;
}

u64 ring_write_nowrap(RingBuffer& ring, void* src, u64 src_size, u64 align) {
	Assert(ring.size >= src_size);
	u64 offset = AlignUp(ring.write_pos, align) % ring.size;
	u64 tail = ring.size - offset;
	b32 wrap = src_size > tail;
	if (wrap) {
		ring.write_pos += tail;
		offset = 0;
	}
	MemCopy(ring.base + offset, src, src_size);
	ring.write_pos += src_size;
	return offset;
}

u64 ring_read_nowrap(RingBuffer& ring, void* dst, u64 dst_size) {
	Assert(ring.size >= dst_size);
	u64 offset = ring.read_pos % ring.size;
	u64 tail = ring.size - offset;
	b32 wrap = dst_size > tail;
	if (wrap) {
		ring.read_pos += tail;
		offset = 0;
	}
	MemCopy(dst, ring.base+offset, dst_size);
	ring.read_pos += dst_size;
	return offset;
}

////////////////////////////////////////////////////////////////////////
// Coroutine

u8* Restrict _coroutine_var(Coroutine* co, u32 size) {
	Assert(co->stack_pointer + size < CoroutineStackSize);
	u8* res = co->stack + co->stack_pointer;
	co->stack_pointer += size;
	return res;
}

////////////////////////////////////////////////////////////////////////
// Simd

f32x4 simd_load(void* p)                    { return {_mm_loadu_ps((f32*)p)}; }
void simd_store(f32x4 x, void* p)           { _mm_storeu_ps((f32*)p, x.p); }
f32x4 simd_splat(f32 x)                     { return {_mm_set1_ps(x)}; }
f32x4 simd_make(f32 x, f32 y, f32 z, f32 w) { return {_mm_setr_ps(x,y,z,w)}; }
f32x4 simd_zero()                           { return {.p = _mm_setzero_ps()}; }

f32x4 operator+(f32x4 a, f32x4 b) { return {_mm_add_ps(a.p, b.p)}; }
f32x4 operator-(f32x4 a, f32x4 b) { return {_mm_sub_ps(a.p, b.p)}; }
f32x4 operator-(f32x4 a)          { return {simd_zero() - a}; }
f32x4 operator*(f32x4 a, f32x4 b) { return {_mm_mul_ps(a.p, b.p)}; }
f32x4 operator/(f32x4 a, f32x4 b) { return {_mm_div_ps(a.p, b.p)}; }
f32x4 operator^(f32x4 a, f32x4 b) { return {_mm_xor_ps(a.p, b.p)}; }
f32x4 operator&(f32x4 a, f32x4 b) { return {_mm_and_ps(a.p, b.p)}; }
f32x4 operator|(f32x4 a, f32x4 b) { return {_mm_or_ps(a.p, b.p)}; }

f32x4 operator<(f32x4 a, f32x4 b)  { return {_mm_cmplt_ps(a.p, b.p)}; }
f32x4 operator<=(f32x4 a, f32x4 b) { return {_mm_cmple_ps(a.p, b.p)}; }
f32x4 operator>(f32x4 a, f32x4 b)  { return {_mm_cmpgt_ps(a.p, b.p)}; }
f32x4 operator>=(f32x4 a, f32x4 b) { return {_mm_cmpge_ps(a.p, b.p)}; }
f32x4 operator==(f32x4 a, f32x4 b) { return {_mm_cmpeq_ps(a.p, b.p)}; }
f32x4 operator!=(f32x4 a, f32x4 b) { return {_mm_cmpneq_ps(a.p, b.p)}; }

f32x4& operator+=(f32x4& a, f32x4 b) { a = a + b; return a; }
f32x4& operator-=(f32x4& a, f32x4 b) { a = a - b; return a; }
f32x4& operator*=(f32x4& a, f32x4 b) { a = a * b; return a; }
f32x4& operator/=(f32x4& a, f32x4 b) { a = a / b; return a; }
f32x4& operator^=(f32x4& a, f32x4 b) { a = a ^ b; return a; }
f32x4& operator&=(f32x4& a, f32x4 b) { a = a & b; return a; }
f32x4& operator|=(f32x4& a, f32x4 b) { a = a | b; return a; }

f32x4 simd_min(f32x4 a, f32x4 b)            { return {_mm_min_ps(a.p, b.p)}; }
f32x4 simd_max(f32x4 a, f32x4 b)            { return {_mm_max_ps(a.p, b.p)}; }
f32x4 simd_andnot(f32x4 a, f32x4 b)         { return {_mm_andnot_ps(a.p, b.p)}; };
f32x4 simd_sqrt(f32x4 a)                    { return {_mm_sqrt_ps(a.p)}; }
f32x4 simd_blend(f32x4 a, f32x4 m, f32x4 b) { return {_mm_blendv_ps(a.p,m.p,b.p)}; }
f32x4 simd_floor(f32x4 a)                   { return {_mm_floor_ps(a.p)}; }
u32 simd_movemask(f32x4 a)                  { return _mm_movemask_ps(a.p); }
f32x4 simd_unpacklo(f32x4 a, f32x4 b)       { return {_mm_unpacklo_ps(a.p, b.p)}; }
f32x4 simd_unpackhi(f32x4 a, f32x4 b)       { return {_mm_unpackhi_ps(a.p, b.p)}; }
f32x4 simd_movelh(f32x4 a, f32x4 b)         { return {_mm_movelh_ps(a.p, b.p)}; }
f32x4 simd_movehl(f32x4 a, f32x4 b)         { return {_mm_movehl_ps(a.p, b.p)}; }

f32x4 simd_cast_itof(f32x4 a) { return {_mm_castsi128_ps(a.p)}; }
f32x4 simd_cast_ftoi(f32x4 a) { return {_mm_castps_si128(a.p)}; }

const f32x4 f32x4SignMask = simd_cast_itof(f32x4(_mm_set1_epi32(U32_MAX-1)));

f32x4 simd_abs(f32x4 a)                     { return simd_andnot(f32x4(f32x4SignMask), a); }
f32x4 simd_sign_bit(f32x4 a)                { return a & f32x4(f32x4SignMask); };
f32x4 simd_sign_of(f32x4 a)                 { return simd_splat(1) | simd_sign_bit(a); };
f32x4 simd_clamp(f32x4 a, f32x4 x, f32x4 b) { return simd_min(simd_max(a, x), b); }
f32x4 simd_clamp01(f32x4 x)                 { return simd_clamp(simd_zero(), x, simd_splat(1)); }


// struct CoroCtx {
//   void *stack_pointer;
// };

// struct Coro {
//   CoroCtx ctx;
//   Coro* caller;
//   void (*fn)(void* arg);
//   void* arg;
//   void* stack_mem;
//   size_t stack_size;
//   int finished;
// };

// thread_local Coro  g_thread_ctx;
// thread_local Coro *g_current;

// __attribute__((naked)) void coro_return_to_caller(CoroCtx* from, CoroCtx* to) {
//   asm volatile(R"(
//     push %rbp
//     push %rbx
//     push %r12
//     push %r13
//     push %r14
//     push %r15

//     mov %rsp, (%rdi)
//     mov (%rsi), %rsp

//     pop %r15
//     pop %r14
//     pop %r13
//     pop %r12
//     pop %rbx
//     pop %rbp

//     ret
//   )");
// }

// __attribute__((naked)) void coro_switch(CoroCtx* from, CoroCtx* to) {
//   asm volatile(R"(
//     push %rbp
//     push %rbx
//     push %r12
//     push %r13
//     push %r14
//     push %r15

//     mov %rsp, (%rdi)
//     mov (%rsi), %rsp

//     pop %r15
//     pop %r14
//     pop %r13
//     pop %r12
//     pop %rbx
//     pop %rbp

//     ret
//   )");
// }

// [[noreturn]] __attribute__((naked)) void coro_trampoline() {
//   asm volatile(R"(
//     mov %r12, %rdi
//     call coro_start
//   )");
// }

// Coro *coro_make(void (*fn)(void *arg), void *arg, size_t stack_size) {
//   Coro *co = (Coro*)calloc(1, sizeof(Coro));
//   co->fn         = fn;
//   co->arg        = arg;
//   co->stack_size = stack_size;
//   co->stack_mem  = malloc(stack_size);

//   uint8_t *sp = (uint8_t *)co->stack_mem + stack_size;
//   sp = (uint8_t *)((uintptr_t)sp & ~(uintptr_t)0xF); /* 16-byte align */

//   /*
//     * Hand-build a stack frame that looks exactly like what coro_switch()
//     * would find if this coroutine had been running and just pushed its
//     * six callee-saved registers. Laid out high -> low address, matching
//     * push order rbp,rbx,r12,r13,r14,r15 (so pop order r15..rbp restores
//     * correctly), with the trampoline's address sitting where `ret`
//     * expects a return address:
//     *
//     *   [ coro_trampoline ]   <- "return address"
//     *   [ rbp = 0         ]
//     *   [ rbx = 0         ]
//     *   [ r12 = co        ]   <- trampoline reads this as its argument
//     *   [ r13 = 0         ]
//     *   [ r14 = 0         ]
//     *   [ r15 = 0         ]   <- co->ctx.rsp points here
//     */
//   sp -= sizeof(void *); *(void **)sp = (void *)coro_trampoline;
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* rbp */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* rbx */
//   sp -= sizeof(void *); *(void **)sp = (void *)co; /* r12 -> self */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r13 */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r14 */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r15 */

//   co->ctx.stack_pointer = sp;
//   return co;
// }

// void coro_restart(Coro *co) {
//   co->finished = 0;

//   uint8_t* sp = (uint8_t*)co->stack_mem + co->stack_size;

//   sp = (uint8_t*)((uintptr_t)sp & ~(uintptr_t)0xF);

//   sp -= sizeof(void *); *(void **)sp = (void *)coro_trampoline;
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* rbp */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* rbx */
//   sp -= sizeof(void *); *(void **)sp = (void *)co; /* r12 -> self */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r13 */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r14 */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r15 */

//   co->ctx.stack_pointer = sp;
// }

// void coro_destroy(Coro *co) {
//   if (!co) return;
//   free(co->stack_mem);
//   free(co);
// }

// Coro *coro_current(void) {
//   if (!g_current) g_current = &g_thread_ctx;
//   return g_current;
// }

// void coro_resume(Coro *co) {
//   co->caller = coro_current();
//   g_current  = co;
//   coro_switch(&co->caller->ctx, &co->ctx);
//   // Coro *prev = coro_current();

//   //   co->caller = prev;
//   //   g_current = co;

//   //   coro_switch(&prev->ctx, &co->ctx);
//   /* control returns here once `co` yields or finishes */
// }

// void coro_yield(void) {
//   Coro* co = coro_current();
//   g_current = co->caller;
//   coro_switch(&co->ctx, &co->caller->ctx);
//   /* control returns here once we're resumed again */
// }

// /* Called once, from coro_trampoline, the first time a coroutine runs. */
// C_LINKAGE void coro_start(Coro *co) {
//   co->fn(co->arg);
//   g_current = co->caller;
//   coro_switch(&co->ctx, &co->caller->ctx);
//   AssertAlways(0);
//   __builtin_unreachable(); /* a finished coroutine is never resumed again */
// }

// void example(void* ctx) {
//   u32* i = (u32*)ctx;
//   Loop (c, *i) {
//     // Info("from coroutine %i", c);
//     printf("example %i\n", *i);
//     coro_yield();
//   }
//   u32 a = 1;
// }

// void foo() {
//   u32 i = 2;
//   Coro* c = coro_make(example, &i, KB(8));
//   Info("start example");
//   Loop(i, 10) {
//     Info("From outside %i", i);
//     coro_restart(c);
//     coro_resume(c);
//   }

//   u32 a = 1;
// }



// template<i32 N> struct BitSetIter1 {
//   BitSetS<N>* bits;
//   u64 word_i;
//   u64 word;
//   operator bool() {
//     while (word == 0) {
//       if (word_i >= bitset_word_count(*bits))
//         return false;
//       word = bits->words[word_i];
//       ++word_i;
//     }
//     // u64 bit = ctz(word);
//     u64 word_i = word_i - 1;
//     // u64 idx = word_i * 64 + (64-bit);
//     word = remove_lowest_bit(word);
//     return true;
//   }
//   BitSetIter1& operator++() {}
//   // T& operator*() {
//   // }
// };

// template<i32 N> BitSetIter<N> bit_iter_begin(BitSetS<N>* bits) {
//   BitSetIter res = {
//     .bits = bits,
//   };
//   return res;
// }

// template<i32 N> b32 bit_iter_next(BitSetIter<N>* it, ThingId* out) {
//   while (it->word == 0) {
//     if (it->word_i >= bitset_word_count(*it->bits))
//       return false;
//     it->word = it->bits->words[it->word_i];
//     ++it->word_i;
//   }
//   u64 bit = ctz(it->word);
//   u64 word_i = it->word_i - 1;
//   u64 idx = word_i * 64 + (64-bit);
//   it->word = remove_lowest_bit(it->word);
//   *out = pool_get_handle(st->things, idx);
//   return true;
// }

// typedef u32 EntityId;
// struct Entity {
//   u32 slot;
//   // ...
// };

// Array<Entity, 1024> entities;
// Array<EntityId, 128> specific_entities;

// void push_to_specific(EntityId id) {
//   Entity* e = &entities[id];
//   u32 idx = specific_entities.count;
//   array_push(specific_entities, id);
//   e->slot = idx;
// }
// void remove_from_specific(EntityId id) {
//   Entity* e = &entities[id];

//   u32 idx = e->slot;
//   u32 last = specific_entities.count - 1;
//   EntityId moved = specific_entities[last];

//   specific_entities[idx] = moved;
//   --specific_entities.count;

//   entities[moved].slot = idx;
// }

// template <typename T> f64 benchmark(T func, u32 iterations) {
//   u64 start = cpu_timer_now();

//   Loop(i, iterations) {
//     func();
//   }

//   return tsc_to_ms(cpu_timer_now() - start);
// }


// struct Node {
//   ThingId id;
//   u32 next;
//   u32 prev;
// };

// struct List {
//   u32 first;
//   u32 last;
// };

// Poolu32<Node, 128> pool;

// void list_push(List& l, ThingId id) {
//   u32 idx = pool_push(pool, {id});
//   idll_list_push_back(pool.data, l, idx);
// }
// void list_remove(List& l, u32 idx) {
//   idll_list_remove(pool.data, l, idx);
//   pool_remove(pool, idx);
// }

// void foo() {
//   List x_list = {};
//   // list_push(pool, x_list, ThingId{1});
//   list_push(x_list, ThingId{1});
//   list_push(x_list, ThingId{2});
//   list_push(x_list, ThingId{3});

//   {
//     Scratch scratch;
//     struct Node {
//       u32 i;
//       Node* next;
//       Node* prev;
//     };
//     struct List {
//       Node* first;
//       Node* last;
//     };
//     List list = {};
//     Loop (i, 10) {
//       Node* n = push_struct(scratch, Node);
//       n->i = i;
//       dll_list_push_back(list, n);
//     }
//     LoopNode (it, list.first) {
//       Info("%i", it->i);
//     }
//   }

//   LoopIter (it, pool_begin(pool, x_list.first)) {
//     Info("%i", (*it).id);
//   }
//   list_remove(x_list, x_list.first);
//   LoopIter (it, pool_begin(pool, x_list.first)) {
//     Info("%i", (*it).id);
//   }

//   // LoopINode (it, x_list.first, pool.data) {
//   //   Info("%i", it);
//   // }

//   // list_push<Node, 128, List>(pool, x_list, id);
//   // list_push(pool, x_list, ThingId{1});
//   // list_push(pool, x_list, {2});
//   // list_push(pool, x_list, {3});
// }

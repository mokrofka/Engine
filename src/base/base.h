#pragma once

typedef __builtin_va_list VaList;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap) 									__builtin_va_end(ap)
#define va_arg(ap, type) 			__builtin_va_arg(ap, type)
#define va_copy(dest, src)  __builtin_va_copy(dest, src)

#if _WIN64
	#define OS_WINDOWS 1
#elif __linux__
	#define OS_LINUX 1
#elif __APPLE__
	#define OS_MAC 1
#endif
#if _MSC_VER
	#define COMPILER_MSVC 1
#elif __clang__
	#define COMPILER_CLANG 1
#elif __GNUC__
	#define COMPILER_GCC 1
#endif
#if __x86_64__ || _M_X64
	#define ARCH_X64 1
#elif __aarch64__ || _M_ARM64
	#define ARCH_ARM64 1
#endif

#if OS_LINUX
	typedef __UINT8_TYPE__  u8;
	typedef __UINT16_TYPE__ u16;
	typedef __UINT32_TYPE__ u32;
	typedef __UINT64_TYPE__ u64;
	typedef __uint128_t u128;

	typedef __INT8_TYPE__  i8;
	typedef __INT16_TYPE__ i16;
	typedef __INT32_TYPE__ i32;
	typedef __INT64_TYPE__ i64;

	typedef float  f32;
	typedef double f64;

	typedef u8  b8;
	typedef i32 b32;

	#define shared_function C_LINKAGE
#else 
	#error OS not supported
#endif

#if COMPILER_CLANG
	#define NO_DEBUG __attribute__((nodebug))
	#define INLINE   inline __attribute__((always_inline))
	#define NO_ASAN  __attribute__((no_sanitize("address")))
	#define read_only __attribute__((section(".rodata")))
	#define Packed __attribute__((packed))
#else
	#error Compiler not supported.
#endif

#define C_LINKAGE_BEGIN extern "C"{
#define C_LINKAGE_END }
#define C_LINKAGE extern "C"

#if ASAN_ENABLED
	C_LINKAGE void __asan_poison_memory_region(void const volatile* addr, u64 size);
	C_LINKAGE void __asan_unpoison_memory_region(void const volatile* addr, u64 size);
	#define AsanPoisonMemRegion(addr, size)   __asan_poison_memory_region((addr), (size))
	#define AsanUnpoisonMemRegion(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
	#define AsanPoisonMemRegion(addr, size)   ((void)(addr), (void)(size))
	#define AsanUnpoisonMemRegion(addr, size) ((void)(addr), (void)(size))
#endif

#define null 0
#define NoFlags 0
#define var auto
#define intern static
#define global_var static
#define local_persist  static
#define GlobalVar

#define U8_MAX  0xff
#define U16_MAX 0xffff
#define U32_MAX 0xffffffff
#define U64_MAX 0xffffffffffffffff
#define F32_MIN 1.17549435e-38f
#define F32_MAX 3.40282347e+38f
#define F64_MIN 2.2250738585072014e-308
#define F64_MAX 1.7976931348623157e+308
#define PAGE_SIZE 4096

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) ((x) << 40)
#define Thousand(x) ((x) * 1000)
#define Million(x)  ((x) * 1000000)
#define Billion(x)  ((x) * 1000000000)

#define OffsetOf(T,m)        (u64)(&((T*)0)->m)
#define ContainerOf(ptr,T,m) (T*)(((u8*)(ptr) - OffsetOf(T,m)))

template<typename T> void Swap(T& a, T& b) {
	T temp = a;
	a = b;
	b = temp;
}

NO_DEBUG f32 BytesToKB(u64 x);
NO_DEBUG f32 BytesToMB(u64 x);
NO_DEBUG f32 BytesToGB(u64 x);

////////////////////////////////////////////////////////////////////////
// Memory
void MemSet(void *d, i32 byte, u64 size);
void MemZero(void *d, u64 size);
void MemCopy(void* d, void* s, u64 size);
b32  MemMatch(void* a, void* b, u64 size);
#define MemZeroStruct(x)       MemZero((x), sizeof(*(x)))
#define MemZeroArray(x, c)     MemZero((x), sizeof(*(x)) * (c))
#define MemCopyStruct(d, s)    MemCopy((d), (s), sizeof(*(d)))
#define MemCopyArray(d, s, c)  MemCopy((d), (s), sizeof(*(d)) * (c))
#define MemMatchStruct(a, b)   MemMatch((a), (b), sizeof(*(a)))
#define MemMatchArray(a, b, c) MemMatch((a), (b), sizeof(*(a)) * (c))

u64 align_up(u64 x, u64 a);
u64 align_down(u64 x, u64 a);
u64 align_pad_up(u64 x, u64 a);
u64 align_pad_down(u64 x, u64 a);
b32 is_aligned(u64 x, u64 a);
u8* align_up_ptr(void* x, u64 a);
u8* align_down_ptr(void* x, u64 a);
u8* align_pad_up_ptr(void* x, u64 a);
u8* align_pad_down_ptr(void* x, u64 a);
b32 is_aligned_ptr(void* x, u64 a);
b32 is_pow2(u64 x);
u64 ptr_diff(void* a, void* b);
b32 ptr_match(void* a, void* b);
u8* Offset(void* x, u64 a);
u8* OffsetBack(void* x, u64 a);
#define OffsetAs(ptr,T,off)          (T*)(Offset((ptr), (off)))
#define OffsetStruct(ptr,T)          (T*)(Offset((ptr), sizeof(T)))
#define OffsetArray(ptr,T,c)         (T*)(Offset((ptr), sizeof(T) * (c)))
#define OffsetBackAs(ptr,T,off)      (T*)(OffsetBack((ptr), (off)))
#define OffsetBackStruct(ptr,T)      (T*)(OffsetBack((ptr), sizeof(T)))
#define OffsetBackArray(ptr,T,c)     (T*)(OffsetBack((ptr), sizeof(T) * (c)))

////////////////////////////////////////////////////////////////////////
// Bits
u32 clz(u64 v);
u32 clz32(u32 v);
u32 ctz(u64 v);
u32 ctz32(u32 v);
u32 count_ones(u64 v);
u32 most_significant_bit(u32 size);
u64 most_significant_bit(u64 size);
u32 remove_lowest_bit(u64 v);
b32 bit_has(u64 x, u64 pos);
u64 flag_clear(u64 x, u64 f);
u64 flag_toggle(u64 x, u64 f);
b32 flag_has(u64 x, u64 f);
b32 flag_any(u64 x, u64 f);

////////////////////////////////////////////////////////////////////////
// Common operations
#define Max(a, b)              ((a) > (b) ? (a) : (b))
#define Min(a, b)              ((a) < (b) ? (a) : (b))
#define Max3(a, b, c)          Max(Max((a),(b)), (c))
#define Min3(a, b, c)          Min(Min((a),(b)), (c))
#define ClampTop(x, a)         Min((x), (a))
#define ClampBot(x, a)         Max((x), (a))
#define Clamp(a, x, b)         ((x) < (a) ? (a) : (x) > (b) ? (b) : (x))
#define Clamp01(x)             Clamp(0, (x), 1)
#define Clamp11(x)             Clamp(-1, (x), 1)
#define ReverseClamp(a, x, b)  ((x) < (a) ? (b) : (b) < (x) ? (a) : (x))
#define Square(x)              ((x) * (x))
#define Cube(x)                ((x) * (x) * (x))
#define Sign(x)                (((x) < 0) ? -1 : ((x) > 0) ? 1 : 0)
#define Abs(x)                 (((x) < 0) ? -(x) : (x))
#define InRange(a, x, b)							((a) <= (x) && (x) < (b))
#define IsBetween(a, x, b)					((a) <= (x) && (x) <= (b))

u64 mod_pow2(u64 x, u64 b);
u64 div_pow2(u64 x, u64 b);
u64 div_ceil(u64 x, u64 b);
u64 round_up(u64 x, u64 a);
u64 round_down(u64 x, u64 a);
u64 compose_64(u32 a, u32 b);
u32 next_pow2(u32 x);
u32 prev_pow2(u32 x);

#define is_finite(x)   __builtin_isfinite((x))
#define is_nan(x)      __builtin_isnan((x))
#define is_inf(x)      __builtin_isinf((x))
#define Restrict       __restrict
#define Unreachable    __builtin_unreachable()
#define bit_cast(T, x) __builtin_bit_cast(T, (x))

////////////////////////////////////////////////////////////////////////
// Shenanigans
#define ArrayCount(x)   (sizeof((x)) / sizeof((x)[0]))
#define ArrayRand(arr)  arr[rand_u32(0, ArrayCount(arr))]
#define ArrayZero(arr)  MemZeroArray((arr), ArrayCount((arr)))
#define ArrayCopy(d, s) MemCopyArray((d), (s), ArrayCount((s)))
#define Assign(a,b)    (*((u8**)(&(a))) = (u8*)(b))
#define _Stringify(S)  #S
#define Stringify(S)   _Stringify(S)
#define _Glue(A,B)     A##B
#define Glue(A,B)      _Glue(A,B)
#define _Def(val, def)    								(((val) == 0) ? (def) : (val))
#define _DefSet(val, def) 								if(val == 0) val = def
#define _DefIfSet(val, expr, def) if(expr) val = def
#define MakeId(T) struct T {u32 idx : 24; u32 gen : 8;}
#define For 																						for(;;)
#define Loop(it, c)          					for(i32 it = 0; it < c; ++it)
#define LoopNoInc(it, c)          for(i32 it = 0; it < c;)
#define LoopReverse(it, c)    				for(i32 it = (c) - 1; it >= 0; --it)
#define LoopOff(it, li, hi)       for(i32 it = (li); it < (hi); ++it)
#define LoopArray(it, array)        Loop(it, ArrayCount(array))
#define LoopArrayReverse(it, array) LoopReverse(it, ArrayCount(array))
#define LoopEnum(it, type)        for(type it = (type)0; it < type##_COUNT; it = (type)(it+1))
#define LoopEnumNonZero(it, type) for(type it = (type)1; it < type##_COUNT; it = (type)(it+1))
#define LoopRange(it, range)      for(i32 it = (range).min; it < (range).max; ++it)
#define LoopNode(it, first)       for(var* it = first; it != 0; it = it->next)
#define LoopNodeReverse(it, last) for(var* it = last; it != 0; it = it->prev)
#define LoopHNode(it, first, arr) for(var it = first; it.idx != 0; it = (arr)[it.idx].next)
#define LoopINode(it, first, arr) for(var it = first; it != 0; it = (arr)[it].next)
// #define LoopIter(it, begin) 						for(var it = begin; it; ++it)
#define LoopIter(it, begin) 						for(var it = begin; iter_next(it);)

// #define X(a, ...) X_IMPL(a, ##__VA_ARGS__, 3)
// #define X_IMPL(a, value, ...) x_foo(a, value)

// #define X_1(a) x_foo(a, 3)
// #define X_2(a, value) x_foo(a, value)
// #define X_SELECT(_1, _2, NAME, ...) NAME
// #define X(...) X_SELECT(__VA_ARGS__, X_2, X_1)(__VA_ARGS__)

void Trap();
void DebugTrap();
#define InvalidDefaultCase  default: {InvalidPath;}
#define NotImplemented      Assert(!"Not Implemented!")
#define AssertAlways(x)     (x) ? NoOp(0) : Trap()
#define NoOp(x) (void)(x)
#if BUILD_DEBUG
	#define InvalidPath Assert(!"Invalid Path!")
	#define Assert(x) (x) ? NoOp(0) : DebugTrap()
	#define AssertMsg(x, message, ...) (x) ? NoOp(0) : (_log_output(LogLevel_Error, message, ##__VA_ARGS__), DebugTrap())
	#define AssertOk(x) Assert(x)
#else
	#define InvalidPath Unreachable
	#define Assert(x)
	#define AssertMsg(x, message, ...)
	#define AssertOk(x) x
#endif

////////////////////////////////////////////////////////////////////////
// Atomic Operations
#define AtomicSeqCst __ATOMIC_SEQ_CST
#define AtomicRelaxed __ATOMIC_RELAXED
#define AtomicAcquire __ATOMIC_ACQUIRE
#define AtomicRelease __ATOMIC_RELEASE
#define atomic_inc_explicit(x, order)       __atomic_fetch_add((x), 1, (order))
#define atomic_dec_explicit(x, order)       __atomic_fetch_sub((x), 1, (order))
#define atomic_add_explicit(x, v, order)    __atomic_fetch_add((x), (v), (order))
#define atomic_sub_explicit(x, v, order)    __atomic_fetch_sub((x), (v), (order))
#define atomic_load_explicit(x, order)      __atomic_load_n((x), (order))
#define atomic_store_explicit(x, v, order)  __atomic_store_n((x), (v), (order))
#define atomic_or_explicit(x, v, order)     __atomic_fetch_or((x), (v), (order))
#define atomic_and_explicit(x, v, order)    __atomic_fetch_and((x), (v), (order))
#define atomic_nand_explicit(x, v, order)   __atomic_fetch_nand((x), (v), (order))
#define atomic_xor_explicit(x, v, order)    __atomic_fetch_xor((x), (v), (order))
#define atomic_swap_explicit(x, v, order)   __atomic_exchange_n((x), (v), (order))
#define atomic_cmp_swap_explicit(x, expect, v, success_order, failure_order) __atomic_compare_exchange_n((x), (expect), (v), 0, (success_order), (failure_order))
#define atomic_cmp_swap_weak_explicit(x, expect, v, success_order, failure_order) __atomic_compare_exchange_n((x), (expect), (v), 1, (success_order), (failure_order))
#define atomic_inc(x)      atomic_inc_explicit((x), AtomicSeqCst)
#define atomic_dec(x)      atomic_dec_explicit((x), AtomicSeqCst)
#define atomic_add(x, v)   atomic_add_explicit((x), (v), AtomicSeqCst)
#define atomic_sub(x, v)   atomic_sub_explicit((x), (v), AtomicSeqCst)
#define atomic_load(x)     atomic_load_explicit((x), AtomicSeqCst)
#define atomic_store(x, v) atomic_store_explicit((x), (v), AtomicSeqCst)
#define atomic_or(x, v)    atomic_or_explicit((x), (v), AtomicSeqCst)
#define atomic_and(x, v)   atomic_and_explicit((x), (v), AtomicSeqCst)
#define atomic_nand(x, v)  atomic_nand_explicit((x), (v), AtomicSeqCst)
#define atomic_xor(x, v)   atomic_xor_explicit((x), (v), AtomicSeqCst)
#define atomic_swap(x, v)  atomic_swap_explicit((x), (v), AtomicSeqCst)
#define atomic_cmp_swap(x, expect, v) atomic_cmp_swap_explicit((x), (expect), (v), (AtomicSeqCst), (AtomicSeqCst))
#define atomic_cmp_swap_weak(x, expect, v) atomic_cmp_swap_weak_explicit((x), (expect), (v), (AtomicSeqCst), (AtomicSeqCst))
inline b32 atomic_cmp_set(u32* x, u32 expect, u32 v) { return atomic_cmp_swap(x, &expect, v); }
inline u32 atomic_cmp_swap_old(u32* x, u32 expect, u32 v) { u32 res = expect; atomic_cmp_swap(x, &res, v); return res; }

////////////////////////////////////////////////////////////////////////
// Defer
struct _DeferDummy {};
template <typename F> struct _Defer { F f; ~_Defer() { f(); } };
template <typename F> _Defer<F> operator+(_DeferDummy, F f) { return {f}; }
#define defer auto Glue(_defer_, __LINE__) = _DeferDummy{} + [&]()
#define DeferLoop(begin, end) for(int _i_ = ((begin), 1); _i_; _i_ = 0, (end))
#define DeferLoopIf(begin, end) for(int _i_ = (begin); _i_; _i_ = false, (end))

////////////////////////////////////////////////////////////////////////
// Circular Linked List
#define DLL_list_init(a, n, next, prev) \
	(a[n].next = n,                        \
		a[n].prev = n)
#define DLL_list_push(a, first, n, next, prev) \
	a[n].next = first;                            \
	a[n].prev = a[first].prev;                    \
	a[a[first].prev].next = n;                    \
	a[first].prev = n;
#define DLL_list_insert(a, first, n, next, prev) \
	a[n].next = a[first].next;                      \
	a[n].prev = first;                              \
	a[a[first].next].prev = n;                      \
	a[first].next = n;
#define DLL_list_remove(a, n, next, prev) \
	a[a[n].next].prev = a[n].prev;           \
	a[a[n].prev].next = a[n].next;
#define dll_list_init(a, x) DLL_list_init(a, x, next, prev)
#define dll_list_push(a, first, n) DLL_list_push(a, first, n, next, prev)
#define dll_list_insert(a, first, n) DLL_list_insert(a, first, n, next, prev)
#define dll_list_remove(a, n) DLL_list_remove(a, n, next, prev)

#define PDLL_list_init(first, next, prev) \
	(first)->next = first;                   \
	(first)->prev = first;
#define PDLL_list_push(first, n, next, prev) \
	(n)->next = (first);                        \
	(n)->prev = (first)->prev;                  \
	(first)->prev->next = n;                    \
	(first)->prev = n;
#define PDLL_list_insert(first, n, next, prev) \
	(n)->next = (first)->next;                    \
	(n)->prev = first;                            \
	(first)->next->prev = n;                      \
	(first)->next = n;
#define PDLL_list_remove(n, next, prev) \
	(n)->next->prev = (n)->prev;           \
	(n)->prev->next = (n)->next;
#define pdll_list_init(first) PDLL_list_init(first, next, prev)
#define pdll_list_push(first, n) PDLL_list_push(first, n, next, prev)
#define pdll_list_insert(first, n) PDLL_list_insert(first, n, next, prev)
#define pdll_list_remove(n)	PDLL_list_remove(n, next, prev)

////////////////////////////////////////////////////////////////////////
// Doulby Linked List
#define DLL_push_back(first, last, n, next, prev)    \
	((n)->prev = (last),                                \
	 (n)->next = null,                                  \
	 ((last) ? ((last)->next = (n)) : ((first) = (n))), \
	 (last) = (n))
#define DLL_remove(first, last, n, next, prev) \
	(((n)->prev ? ((n)->prev->next = (n)->next)   \
													: ((first) = (n)->next)),         \
		((n)->next ? ((n)->next->prev = (n)->prev)   \
													: ((last) = (n)->prev)))

////////////////////////////////////////////////////////////////////////
// Singly Linked List
#define SLL_stack_push(first, n, next) \
	((n)->next = (first), (first) = (n))
#define SLL_stack_pop(first, next) \
	((first) = (first)->next)
#define SLL_queue_push(first, last, n, next)         \
	((n)->next = null,                                  \
		((last) ? ((last)->next = (n)) : ((first) = (n))), \
		(last) = (n))
#define SLL_queue_pop(first, last, next)          \
	(((first) == (last)) ? ((first) = (last) = null) \
																						: ((first) = (first)->next))

#define dll_push_back(first, last, n)  DLL_push_back(first, last, n, next, prev)
#define dll_remove(first, last, n)     DLL_remove(first, last, n, next, prev)
#define sll_stack_push(first, n)       SLL_stack_push(first, n, next)
#define sll_stack_pop(first)           SLL_stack_pop(first, next)
#define sll_queue_push(first, last, n) SLL_queue_push(first, last, n, next)
#define sll_queue_pop(first, lsat, n)  SLL_queue_pop(first, last, next)

////////////////////////////////////////////////////////////////////////
// Indexed Doubly Linked List
#define IDLL_push_back(arr, first, last, n, next, prev)  \
	((arr)[n].prev = (last),                                \
		(arr)[n].next = 0,                                     \
		((last) ? ((arr)[last].next = (n)) : ((first) = (n))), \
		(last) = (n))
#define IDLL_remove(arr, first, last, n, next, prev)           \
	(((arr)[n].prev ? ((arr)[(arr)[n].prev].next = (arr)[n].next) \
																	: ((first) = (arr)[n].next)),                 \
		((arr)[n].next ? ((arr)[(arr)[n].next].prev = (arr)[n].prev) \
																	: ((last) = (arr)[n].prev)))
#define idll_push_back(arr, first, last, n)  IDLL_push_back(arr, first, last, n, next, prev)
#define idll_remove(arr, first, last, n)     IDLL_remove(arr, first, last, n, next, prev)

////////////////////////////////////////////////////////////////////////
// Handler Linked List
#define HDLL_push_back(arr, first, last, n, next, prev)           \
		((arr)[n.idx].prev = (last),                                    \
			(arr)[n.idx].next = {},                                        \
			((last.idx) ? ((arr)[last.idx].next = (n)) : ((first) = (n))), \
			(last) = (n))
#define HDLL_remove(arr, first, last, n, next, prev)                                                    \
		(((arr)[n.idx].elem.prev.idx ? ((arr)[(arr)[n.idx].elem.prev.idx].elem.next = (arr)[n.idx].elem.next) \
																															: ((first) = (arr)[n.idx].elem.next)),                                   \
			((arr)[n.idx].elem.next.idx ? ((arr)[(arr)[n.idx].elem.next.idx].elem.prev = (arr)[n.idx].elem.prev) \
																															: ((last) = (arr)[n.idx].elem.prev)))
#define hdll_push_back(arr, first, last, n)  HDLL_push_back(arr, first, last, n, next, prev)
#define hdll_remove(arr, first, last, n)     HDLL_remove(arr, first, last, n, next, prev)

u64 cpu_now();
void cpu_find_frequency();

// @Configuration
#define MEM_GUARD 1
#define MEM_TRACK 1
#define PROFILE_BUILD 1
#define DEV_BUILD 1

typedef u32 Futex;
struct Mutex { Futex futex; };
struct CondVar { Futex futex; };
struct Semaphore { Futex futex; };
struct OS_Handle { u64 v; };

struct Region {
	union {
		u64 base;
		u64 offset;
	};
	union {
		u64 size;
		u64 count;
	};
};

struct String {
	u8* str;
	u64 size;
	String() = default;
	NO_DEBUG String(const char* str_);
	NO_DEBUG String(u8* str, u64 size):str(str),size(size){}
};

struct String64 {
	u8 str[64];
	u32 size;
	operator String();
};

struct HotReloadData {
	void* ctx;
	String lib_path;
};

template<i32 N> struct BitArray {
	u64 words[N/64];
	static constexpr u64 bit_count = N * 64;
};

template<i32 N> void bit_array_set(BitArray<N>& bits, u64 idx)   { bits.words[idx >> 6] |=   u64(1)<<(idx & 63); }
template<i32 N> void bit_array_clear(BitArray<N>& bits, u64 idx) { bits.words[idx >> 6] &= ~(u64(1)<<(idx & 63)); }
template<i32 N> b32 bit_array_get(BitArray<N>& bits, u64 idx)    { return (bits.words[idx >> 6] >> (idx & 63))&1; }
template<i32 N> u64 bit_array_word_count(BitArray<N>& bits)      { return (bits.bit_count + 63) / 64; }

struct RingBuffer {
	u8* base;
	u64 size;
	u64 write_pos;
	u64 read_pos;
};

RingBuffer ring_make(void* base, u64 size);
u64 ring_write(RingBuffer& ring, void* src, u64 src_size);
u64 ring_read(RingBuffer& ring, void* dst, u64 dst_size);
u64 ring_write_nowrap(RingBuffer& ring, void* src, u64 src_size);
u64 ring_read_nowrap(RingBuffer& ring, void* dst, u64 read_size);

template<typename T> struct Slice {
	T* data;
	union {
		u64 count;
		u64 size;
	};
	NO_DEBUG T& operator[](u64 idx) {
		Assert(idx < count);
		return data[idx];
	}
	NO_DEBUG Slice(T* data, u64 count):data(data), count(count){}
	Slice() = default;
	T* begin() { return data; }
	T* end() { return data + count; }
};

template<typename T> NO_DEBUG Slice<T> slice(Slice<T> a, u64 li, u64 hi)      { Assert(li <= hi && hi <= a.count); return Slice(a.data + li, hi - li); }
template<typename T> NO_DEBUG Slice<T> slice_n(Slice<T> a, u64 off, u64 size) { Assert(off+size <= a.count); return Slice(a.data + off, size); }
template<typename T> NO_DEBUG Slice<T> slice_prefix(Slice<T> a, u64 n)        { Assert(n <= a.count); return Slice(a.data, n); }
template<typename T> NO_DEBUG Slice<T> slice_postfix(Slice<T> a, u64 n)       { Assert(n <= a.count); return Slice(a.data + (a.count - n), n); }
template<typename T> NO_DEBUG Slice<T> slice_skip(Slice<T> a, u64 n)          { Assert(n <= a.count); return Slice(a.data + n, a.count - n); }
template<typename T> NO_DEBUG Slice<T> slice_chop(Slice<T> a, u64 n)          { Assert(n <= a.count); return Slice(a.data, a.count - n); }
template<typename T, NO_DEBUG u64 N> Slice<T> slice(T (&a)[N])                { return Slice(a, N); }
template<typename T> NO_DEBUG Slice<u8> slice_to_bytes(Slice<T> s)            { return Slice((u8*)s.data, s.count * sizeof(T)); }
template<typename T> NO_DEBUG Slice<u8> slice_struct_to_bytes(T* s)           { return Slice((u8*)s, sizeof(T)); }
template<typename T> NO_DEBUG u64 slice_size(Slice<T> s)                      { return s.count * sizeof(T); }
template<typename To, typename From> NO_DEBUG Slice<To> slice_reinterpret(Slice<From> s) {
	Assert((s.count * sizeof(From)) % sizeof(To) == 0);
	return Slice((To*)s.data, (s.count*sizeof(From)) / sizeof(To));
}
template<typename T> NO_DEBUG void slice_copy(Slice<T> dst, Slice<T> src) { MemCopyArray(dst.data, src.data, src.count); }

////////////////////////////////////////////////////////////////////////
// RandyGaul stackless coroutine
#define CoroutineMaxDepth 8
#define CoroutineCaseOffset Million(1)
#define CoroutineStackSize (512)
struct Coroutine {
	f32 wait_elapsed;
	f32 for_elapsed;
	u32 flag;
	u32 index;
	u32 line[CoroutineMaxDepth];
	u32 stack_pointer;
	u8 stack[CoroutineStackSize];
};
u8* Restrict _coroutine_var(Coroutine& co, u32 size);
#define co_var(co, T) *(T*)_coroutine_var(co, sizeof(T))

#define co_begin(co)              \
	do {                             \
		var& __co = co;															  \
		__co.flag = 0;                  \
		switch(__co.line[__co.index]) { \
			default:

#define co_case(name)     \
	case __LINE__:           \
	name: __co.line[__co.index] = __LINE__;

#define co_wait(time)                \
	do {                                \
		case __LINE__:                     \
			__co.line[__co.index] = __LINE__; \
			__co.wait_elapsed += dt;          \
			if(__co.wait_elapsed < time) {    \
				co_exit();																							\
			} else __co.wait_elapsed = 0;     \
	} while(0)

#define co_for(time)                 \
	do {                                \
		case __LINE__:                     \
			__co.line[__co.index] = __LINE__; \
			__co.for_elapsed += dt;           \
			if(__co.for_elapsed >= (time)) {  \
				__co.for_elapsed = 0;            \
				break;                           \
			}
#define co_for_end() \
		co_exit(); \
	} while (0)

#define co_exit()   \
	do {               \
		__co.flag = 1;    \
		goto __co_end;    \
	} while(0)

#define co_yield()                  \
	do {                               \
		__co.line[__co.index] = __LINE__; \
		co_exit();                        \
		case __LINE__:;                   \
	} while(0)

#define co_call(...)                                                                            \
	__co.flag = 0;                                                                                 \
	case __LINE__:                                                                                 \
		Assert(__co.index < CoroutineMaxDepth);                                                       \
		__co.line[__co.index++] = __LINE__;                                                           \
		__VA_ARGS__;                                                                                  \
		__co.index--;                                                                                 \
		if(__co.flag) {                                                                               \
			goto __co_end;                                                                               \
		} else {                                                                                      \
			case __LINE__ + CoroutineCaseOffset: __co.line[__co.index] = __LINE__ + CoroutineCaseOffset; \
		}

#define co_end()          																	  \
	}                        																	  \
	__co.line[__co.index] = 0; 																	\
	__co_end:;               																	  \
	if(__co.index == 0) __co.stack_pointer = 0; \
	} while(0)

////////////////////////////////////////////////////////////////////////
// Simd
// #include <smmintrin.h>

typedef float __v4sf __attribute__((__vector_size__(16)));
typedef unsigned int __v4su __attribute__((__vector_size__(16)));
typedef f32 __m128 __attribute__((__vector_size__(16), __aligned__(16)));
typedef f32 __m128_u __attribute__((__vector_size__(16), __aligned__(1)));
typedef int __v4si __attribute__((__vector_size__(16)));
typedef long long __m128i __attribute__((__vector_size__(16), __aligned__(16)));

union f32x4 {
	__m128 p;
	f32 v[4];
};

void cpu_relax();
#define SimdShuffle(a, b, c, d) (((d) << 6) | ((c) << 4) | ((b) << 2) | (a))
#define simd_shuffle(a, b, imm) f32x4(_mm_shuffle_ps(a.p, b.p, imm))

f32x4 simd_load(void* p);
void simd_store(f32x4 x, void* p);
f32x4 simd_splat(f32 x);
f32x4 simd_make(f32 x, f32 y, f32 z, f32 w);
f32x4 simd_zero();

f32x4 operator+(f32x4 a, f32x4 b);
f32x4 operator-(f32x4 a, f32x4 b);
f32x4 operator-(f32x4 a);
f32x4 operator*(f32x4 a, f32x4 b);
f32x4 operator/(f32x4 a, f32x4 b);
f32x4 operator^(f32x4 a, f32x4 b);
f32x4 operator&(f32x4 a, f32x4 b);
f32x4 operator|(f32x4 a, f32x4 b);

f32x4 operator<(f32x4 a, f32x4 b);
f32x4 operator<=(f32x4 a, f32x4 b);
f32x4 operator>(f32x4 a, f32x4 b);
f32x4 operator>=(f32x4 a, f32x4 b);
f32x4 operator==(f32x4 a, f32x4 b);
f32x4 operator!=(f32x4 a, f32x4 b);

f32x4& operator+=(f32x4& a, f32x4 b);
f32x4& operator-=(f32x4& a, f32x4 b);
f32x4& operator*=(f32x4& a, f32x4 b);
f32x4& operator/=(f32x4& a, f32x4 b);
f32x4& operator^=(f32x4& a, f32x4 b);
f32x4& operator&=(f32x4& a, f32x4 b);
f32x4& operator|=(f32x4& a, f32x4 b);

f32x4 simd_min(f32x4 a, f32x4 b);
f32x4 simd_max(f32x4 a, f32x4 b);
f32x4 simd_andnot(f32x4 a, f32x4 b);
f32x4 simd_sqrt(f32x4 a);
f32x4 simd_blend(f32x4 a, f32x4 m, f32x4 b);
f32x4 simd_floor(f32x4 a);
u32 simd_movemask(f32x4 a);
f32x4 simd_unpacklo(f32x4 a, f32x4 b);
f32x4 simd_unpackhi(f32x4 a, f32x4 b);
f32x4 simd_movelh(f32x4 a, f32x4 b);
f32x4 simd_movehl(f32x4 a, f32x4 b);
f32x4 simd_abs(f32x4 a);
f32x4 simd_sign_bit(f32x4 a);
f32x4 simd_sign_of(f32x4 a);
f32x4 simd_clamp(f32x4 a, f32x4 x, f32x4 b);
f32x4 simd_clamp01(f32x4 x);

#define LOG_TRACE_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_WARN_ENABLED 1
#define LOG_ERROR_ENABLED 1
enum LogLevel {
	LogLevel_Trace = 1,
	LogLevel_Debug,
	LogLevel_Info,
	LogLevel_Warn,
	LogLevel_Error,
};

void _log_output(LogLevel level, String fmt, ...); // with \n
void print(String fmt, ...);
void println(String fmt, ...);

#if LOG_TRACE_ENABLED
	#define Trace(message, ...) _log_output(LogLevel_Trace, message, ##__VA_ARGS__)
#else
	#define Trace(message, ...)
#endif

#if LOG_DEBUG_ENABLED
	#define Debug(message, ...) _log_output(LogLevel_Debug, message, ##__VA_ARGS__)
#else
	#define Debug(message, ...)
#endif

#if LOG_INFO_ENABLED
	#define Info(message, ...) _log_output(LogLevel_Info, message, ##__VA_ARGS__)
#else
	#define Info(message, ...)
#endif

#if LOG_WARN_ENABLED
	#define Warn(message, ...) _log_output(LogLevel_Warn, message, ##__VA_ARGS__)
#else
	#define Warn(message, ...)
#endif

#if LOG_ERROR_ENABLED
	#define Error(message, ...) _log_output(LogLevel_Error, message, ##__VA_ARGS__);
#else
	#define Error(message, ...)
#endif

////////////////////////////////////////////////////////////////////////
// @Math
const f32 PI             = 3.14159265358f;
const f32 Tau            = PI * 2;
const f32 EulerNumber    = 2.71828182846f;
const f32 GoldBig        = 1.61803398875f;
const f32 GoldSmall      = 0.61803398875f;
const f32 FloatEpsilon   = 1e-5f;
const f32 MachineEpsilon = 1.1920929e-7f;

///////////////////////////////////
// v2
union v2 {
	struct {
		f32 x;
		f32 y;
	};
	f32 v[2];
	v2() = default;
	NO_DEBUG explicit v2(f32 s):x(s),y(s){};
	NO_DEBUG v2(f32 x, f32 y):x(x),y(y){}
	NO_DEBUG v2(const union v2u& v);
};
union v2i {
	struct {
		i32 x;
		i32 y;
	};
	i32 v[2];
	v2i() = default;
	v2i(i32 x, i32 y):x(x),y(y){}
};
union v2u {
	struct {
		u32 x;
		u32 y;
	};
	u32 v[2];
	v2u() = default;
	v2u(u32 x, u32 y):x(x),y(y){}
};
union v2b {
	struct {
		b32 x;
		b32 y;
	};
	b32 v[2];
	v2b() = default;
	v2b(b32 x, b32 y):x(x),y(y){}
};

///////////////////////////////////
// v3
union v3 {
	struct {
		f32 x;
		f32 y;
		f32 z;
	};
	struct {
		v2 xy;
	};
	f32 v[3];
	v3() = default;
	explicit v3(f32 s):x(s),y(s),z(s){};
	v3(f32 x, f32 y, f32 z):x(x),y(y),z(z){}
};
union v3u {
	struct {
		u32 x;
		u32 y;
		u32 z;
	};
	u32 v[3];
	v3u() = default;
	v3u(u32 x, u32 y, u32 z):x(x),y(y),z(z){}
};
union v3b {
	struct {
		b32 x;
		b32 y;
		b32 z;
	};
	b32 v[3];
	v3b() = default;
	v3b(b32 x, b32 y, b32 z):x(x),y(y),z(z){}
};

///////////////////////////////////
// v4
union v4 {
	struct {
		f32 x;
		f32 y;
		f32 z;
		f32 w;
	};
	v3 xyz;
	f32 v[4];
	v4() = default;
	NO_DEBUG explicit v4(f32 s):x(s),y(s),z(s){};
	NO_DEBUG v4(f32 x, f32 y, f32 z, f32 w):x(x),y(y),z(z),w(w){}
	NO_DEBUG v4(v3 v, f32 w):x(v.x),y(v.y),z(v.z),w(w){}
};
struct v4u8 {
	u8 v[4];
};
struct v4u {
	u32 v[4];
};

struct m2x2 { f32 v[2][2]; };
struct m3x2 { f32 v[3][2]; };
struct m3x3 { f32 v[3][3]; };
struct m4x3 { f32 v[3][4]; };
struct m4x4 { f32 v[4][4]; };

////////////////////////////////////////////////////////////////////////
// Ranges
struct Rng1u {
	u32 min;
	u32 max;
};
struct Rng1i {
	i32 min;
	i32 max;
};
struct Rng1u64 {
	u64 min;
	u64 max;
};
struct Rng1 {
	f32 min;
	f32 max;
};
union Rng2 {
	struct {
		v2 min;
		v2 max;
	};
	struct {
		f32 x0;
		f32 y0;
		f32 x1;
		f32 y1;
	};
	Rng2() = default;
	Rng2(v2 min, v2 max):min(min),max(max){}
};
union Rng2u {
	struct {
		v2u min;
		v2u max;
	};
	struct {
		u32 x0;
		u32 y0;
		u32 x1;
		u32 y1;
	};
	Rng2u() = default;
	Rng2u(v2u min, v2u max):min(min),max(max){}
};
union Rng3 {
	struct {
		v3 min;
		v3 max;
	};
	struct {
		f32 x0;
		f32 y0;
		f32 z0;
		f32 x1;
		f32 y1;
		f32 z1;
	};
	Rng3() = default;
	Rng3(v3 min, v3 max):min(min),max(max){}
};

struct Transform {
	v3 pos;
	v4 rot;
	v3 scale;
};

struct Ray {
	v3 pos;
	v3 dir;
};

////////////////////////////////////////////////////////////////////////
// Float Ops
NO_DEBUG f32 sin(f32 x);
NO_DEBUG f32 cos(f32 x);
NO_DEBUG f32 tan(f32 x);
NO_DEBUG f32 asin(f32 x);
NO_DEBUG f32 acos(f32 x);
NO_DEBUG f32 atan2(f32 y, f32 x);
NO_DEBUG f32 sqrt(f32 x);
NO_DEBUG f32 pow(f32 a, f32 b);
NO_DEBUG f32 floor(f32 x);
NO_DEBUG f32 ceil(f32 x);
NO_DEBUG f32 round(f32 x);
NO_DEBUG f32 mod(f32 a, f32 b);
NO_DEBUG f32 exp(f32 x);
NO_DEBUG f32 loge(f32 x);
NO_DEBUG f32 log2(f32 x);
NO_DEBUG f32 log10(f32 x);
NO_DEBUG void sincos(f32 rad, f32* a, f32* b);

NO_DEBUG f32 sind(f32 x);
NO_DEBUG f32 cosd(f32 x);

f32 deg2rad(f32 degrees);
f32 rad2deg(f32 radians);
f32 ceil_to(f32 x, f32 step);
f32 floor_to(f32 x, f32 step);
f32 round_to(f32 x, f32 step);

////////////////////////////////////////////////////////////////////////
// Color
v4 rgba_from_u32(u32 hex);
u32 u32_from_rgba(v4 rgba);

////////////////////////////////////////////////////////////////////////
// Hash
#define HASH_DEFAULT_SEED 0x9e3779b97f4a7c15ull
u64 hash(u64 x);
u64 hash(u64 a, u64 b);
u64 hash(u64 a, u64 b, u64 c);
u64 hash(u64 a, u64 b, u64 c, u64 d);
u64 hash(String x, u64 seed = HASH_DEFAULT_SEED);
u64 hash_bytes(void *data, u64 size, u64 seed = HASH_DEFAULT_SEED);

////////////////////////////////////////////////////////////////////////
// Random
NO_DEBUG u64 rand_u64();
NO_DEBUG u32 rand_u32();
NO_DEBUG u32 rand_u32(u32 max);
NO_DEBUG u32 rand_u32(u32 min, u32 max);
NO_DEBUG i32 rand_i32();
NO_DEBUG i32 rand_i32(i32 min, i32 max);
NO_DEBUG f32 rand_f32();
NO_DEBUG f32 rand_f32_11();
NO_DEBUG f32 rand_f32(f32 max);
NO_DEBUG f32 rand_f32(f32 min, f32 max);
NO_DEBUG f32 rand_f32_centered(f32 base, f32 radius);
NO_DEBUG f32 rand_f32_signed(f32 magnitude);
NO_DEBUG b32 rand_b32();
NO_DEBUG void rand_set_seed();
template<typename T> void rand_shuffle(Slice<T> data) {
	Loop(i, data.count) {
		u32 j = rand_u32(i, data.count);
		Swap(data[i], data[j]);
	}
}
template<typename T> T rand_choice(Slice<T> s) { return s[rand_u32()%s.count]; }

////////////////////////////////////////////////////////////////////////
// Misc
i32 ipow(i32 base, u32 exponent);

f32 safe_divn(f32 a, f32 b, f32 n);
f32 safe_div0(f32 a, f32 b);
f32 safe_div1(f32 a, f32 b);
i32 wrap_i32(i32 min, i32 x, i32 max);
f32 wrap_f32(f32 min, f32 x, f32 max);
f32 lerp(f32 a, f32 t, f32 b);
f32 lerp_clamp(f32 a, f32 t, f32 b);
f32 unlerp(f32 a, f32 x, f32 b);
f64 unlerp_f64(f64 a, f64 x, f64 b);
f32 remap(f32 x, f32 old_min, f32 old_max, f32 new_min, f32 new_max);
f32 remap(f32 x, f32 old_max, f32 new_max);
f64 remap_f64(f64 x, f64 old_min, f64 old_max, f64 new_min, f64 new_max);
f64 remap_f64(f64 x, f64 old_max, f64 new_max);
f32 remap_clamp(f32 x, f32 old_min, f32 old_max, f32 new_min, f32 new_max);
f32 approach(f32 from, f32 to, f32 step);

f32 exp_decay(f32 x, f32 target, f32 decay, f32 dt);

f32 wrap_2pi(f32 a);
f32 wrap_pi(f32 a);
f32 angle_delta(f32 from, f32 to);
f32 angle_delta_cw(f32 from, f32 to);
f32 angle_delta_ccw(f32 from, f32 to);
f32 angle_delta_longest(f32 from, f32 to);
f32 angle_approach(f32 from, f32 to, f32 step);

v3 v3_bezier2(v3 p0, v3 p1, v3 p2, f32 t);

////////////////////////////////////////////////////////////////////////
// Vector2
NO_DEBUG v2  operator+(v2 a, v2 b);
NO_DEBUG v2  operator-(v2 a, v2 b);
NO_DEBUG v2  operator*(v2 v, f32 scalar);
NO_DEBUG v2  operator*(f32 scalar, v2 v);
NO_DEBUG v2  operator/(v2 v, f32 scalar);
NO_DEBUG v2  operator+=(v2& a, v2 b);
NO_DEBUG v2  operator-=(v2& a, v2 b);
NO_DEBUG v2  operator*=(v2& v, f32 scalar);
NO_DEBUG v2  operator/=(v2& v, f32 scalar);
NO_DEBUG b32 operator==(v2 a, v2 b);
NO_DEBUG b32 operator!=(v2 a, v2 b);
NO_DEBUG v2  operator-(v2 v);

NO_DEBUG b32 operator==(v2u a, v2u b);
NO_DEBUG b32 operator!=(v2u a, v2u b);

NO_DEBUG v2i v2i_of_v2(v2 v);
NO_DEBUG v2  v2_of_v3(v3 v);
NO_DEBUG v2  v2_of_v4(v4 v);
NO_DEBUG v3  v2_to_v3(v2 v, f32 a);
NO_DEBUG v4  v2_to_v4(v2 v, f32 a, f32 b);
NO_DEBUG v2  v2_of_v2i(v2i v);
NO_DEBUG v2  v2_of_v2u(v2u v);
NO_DEBUG v2  v2_up();
NO_DEBUG v2  v2_down();
NO_DEBUG v2  v2_left();
NO_DEBUG v2  v2_right();
NO_DEBUG v2  v2_abs(v2 v);
NO_DEBUG v2  v2_min(v2 a, v2 b);
NO_DEBUG v2  v2_max(v2 a, v2 b);
NO_DEBUG v2  v2_clamp(v2 min, v2 v, v2 max);
NO_DEBUG v2  v2_sign(v2 v);
NO_DEBUG v2  v2_invert(v2 v);
NO_DEBUG v2b v2_greater(v2 a, v2 b);
NO_DEBUG v2b v2_less(v2 a, v2 b);
NO_DEBUG b32 v2_greater_all(v2 a, v2 b);
NO_DEBUG b32 v2_less_all(v2 a, v2 b);
NO_DEBUG b32 v2_greater_any(v2 a, v2 b);
NO_DEBUG b32 v2_less_any(v2 a, v2 b);
NO_DEBUG v2  v2_rand(v2 a, v2 b);
NO_DEBUG v2  v2_swizzle(v2 v, u32 x, u32 y);
NO_DEBUG v3  v2_swizzle(v2 v, u32 x, u32 y, u32 z);

NO_DEBUG f32 v2_length(v2 v);
NO_DEBUG f32 v2_length_sqr(v2 v);
NO_DEBUG v2  v2_norm(v2 v);
NO_DEBUG f32 v2_distance(v2 a, v2 b);
NO_DEBUG f32 v2_distance_sqr(v2 a, v2 b);
NO_DEBUG f32 v2_dot(v2 a, v2 b);
NO_DEBUG f32 v2_cross(v2 a, v2 b);
NO_DEBUG v2  v2_lerp(v2 a, f32 t, v2 b);
NO_DEBUG v2  v2_hadamard(v2 a, v2 b);
NO_DEBUG v2  v2_hadamard_div(v2 a, v2 b);
NO_DEBUG v2  v2_project(v2 v, v2 dir);
NO_DEBUG v2  v2_project_on_unit(v2 v, v2 dir);
NO_DEBUG f32 v2_project_length(v2 v, v2 dir);
NO_DEBUG v2  v2_reject(v2 v, v2 dir);
NO_DEBUG v2  v2_reject_on_unit(v2 v, v2 dir);
NO_DEBUG v2  v2_reflect(v2 v, v2 norm);
NO_DEBUG f32 v2_angle(v2 a, v2 b);
NO_DEBUG f32 v2_angle(v2 v);
NO_DEBUG v2  v2_from_angle(f32 rad);

v2 v2_approach(v2 current, v2 target, f32 step);
v2 v2_clamp_length(f32 min, v2 v, f32 max);
v2 v2_bounce(v2 v, v2 normal, f32 restitution);

NO_DEBUG v2  v2_flip_y(v2 v);
NO_DEBUG v2  v2_remap_01_to_11(v2 pos, v2 range);
NO_DEBUG f32 v2_line_angle(v2 start, v2 end);
NO_DEBUG v2  v2_cw90(v2 v);
NO_DEBUG v2  v2_ccw90(v2 v);
NO_DEBUG v2  v2_rotate(v2 v, f32 sine, f32 cosine);
NO_DEBUG v2  v2_rotate(v2 v, f32 rad);
NO_DEBUG v2  v2_rotate_around(v2 v, v2 pivot, f32 rad);

////////////////////////////////////////////////////////////////////////
// Vector3
NO_DEBUG v3  operator+(v3 a, v3 b);
NO_DEBUG v3  operator-(v3 a, v3 b);
NO_DEBUG v3  operator*(v3 v, f32 scalar);
NO_DEBUG v3  operator*(f32 scalar, v3 v);
NO_DEBUG v3  operator/(v3 v, f32 scalar);
NO_DEBUG v3  operator+=(v3& a, v3 b);
NO_DEBUG v3  operator-=(v3& a, v3 b);
NO_DEBUG v3  operator*=(v3& v, f32 scalar);
NO_DEBUG v3  operator/=(v3& v, f32 scalar);
NO_DEBUG b32 operator==(v3 a, v3 b);
NO_DEBUG b32 operator!=(v3 a, v3 b);
NO_DEBUG v3  operator-(v3 v);

NO_DEBUG b32 operator==(v3u a, v3u b);

NO_DEBUG v3  v3_of_v4(v4 v);
NO_DEBUG v4  v3_to_v4(v3 v, f32 a);
NO_DEBUG v3  v3_up();
NO_DEBUG v3  v3_down();
NO_DEBUG v3  v3_left();
NO_DEBUG v3  v3_right();
NO_DEBUG v3  v3_forward();
NO_DEBUG v3  v3_back();
NO_DEBUG v3  v3_abs(v3 v);
NO_DEBUG v3  v3_min(v3 a, v3 b);
NO_DEBUG v3  v3_max(v3 a, v3 b);
NO_DEBUG v3  v3_clamp(v3 min, v3 v, v3 max);
NO_DEBUG v3  v3_sign(v3 v);
NO_DEBUG v3  v3_invert(v3 v);
NO_DEBUG b32 v3_equal(v3 a, v3 b);
NO_DEBUG v3b v3_greater(v3 a, v3 b);
NO_DEBUG v3b v3_less(v3 a, v3 b);
NO_DEBUG b32 v3_greater_all(v3 a, v3 b);
NO_DEBUG b32 v3_less_all(v3 a, v3 b);
NO_DEBUG b32 v3_greater_any(v3 a, v3 b);
NO_DEBUG b32 v3_less_any(v3 a, v3 b);
NO_DEBUG v3  v3_rand(v3 a, v3 b);
NO_DEBUG v2  v3_swizzle(v3 v, u32 x, u32 y);
NO_DEBUG v3  v3_swizzle(v3 v, u32 x, u32 y, u32 z);

NO_DEBUG f32 v3_length(v3 v);
NO_DEBUG f32 v3_length_sqr(v3 v);
NO_DEBUG v3  v3_norm(v3 v);
NO_DEBUG f32 v3_distance(v3 a, v3 b);
NO_DEBUG f32 v3_distance_sqr(v3 a, v3 b);
NO_DEBUG f32 v3_dot(v3 a, v3 b);
NO_DEBUG v3  v3_cross(v3 a, v3 b);
NO_DEBUG v3  v3_lerp(v3 a, f32 t, v3 b);
NO_DEBUG v3  v3_hadamard(v3 a, v3 b);
NO_DEBUG v3  v3_hadamard_div(v3 a, v3 b);
NO_DEBUG v3  v3_project(v3 a, v3 b);
NO_DEBUG v3  v3_project_on_norm(v3 a, v3 b);
NO_DEBUG f32 v3_length_projection(v3 a, v3 b);
NO_DEBUG v3  v3_reject(v3 a, v3 b);
NO_DEBUG v3  v3_reject_on_norm(v3 a, v3 b);
NO_DEBUG v3  v3_reflect(v3 v, v3 normal);
NO_DEBUG f32 v3_angle(v3 a, v3 b);

v3 v3_approach(v3 current, v3 target, f32 step);
v3 v3_clamp_length(f32 min, v3 v, f32 max);
v3 v3_bounce(v3 v, v3 normal, f32 restitution);

NO_DEBUG v3  v3_pos_of_mat4(m4x4 mat);
NO_DEBUG v3  v3_rotate_x(v3 v, f32 rad);
NO_DEBUG v3  v3_rotate_y(v3 v, f32 rad);
NO_DEBUG v3  v3_rotate_z(v3 v, f32 rad);
v3 v3_rotate_around_pivot(v3 pos, v3 pivot, v4 q);


v3 v3_barycentric(v3 v, v3 a, v3 b, v3 c);
v3 v3_perpendicular(v3 v);
v3 v3_rotate_around_axis(v3 v, v3 axis, f32 rad);
v3 v3_refract(v3 v, v3 n, f32 r);

////////////////////////////////////////////////////////////////////////
// Vector4

NO_DEBUG v4  operator+(v4 a, v4 b);
NO_DEBUG v4  operator-(v4 a, v4 b);
NO_DEBUG v4  operator*(v4 v, f32 scalar);
NO_DEBUG v4  operator*(f32 scalar, v4 v);
NO_DEBUG v4  operator/(v4 v, f32 scalar);
NO_DEBUG v4  operator+=(v4& a, v4 b);
NO_DEBUG v4  operator-=(v4& a, v4 b);
NO_DEBUG v4  operator*=(v4& v, f32 scalar);
NO_DEBUG v4  operator/=(v4& v, f32 scalar);
NO_DEBUG v4  operator-(v4 v);

NO_DEBUG v4  v4_zero();
NO_DEBUG v4  v4_one();
NO_DEBUG v4  v4_abs(v4 v);
NO_DEBUG v4  v4_set_w(v4 v, f32 w);
NO_DEBUG v4  v4_min(v4 a, v4 b);
NO_DEBUG v4  v4_max(v4 a, v4 b);
NO_DEBUG v4  v4_invert(v4 v);
NO_DEBUG b32 v4_equal(v4 a, v4 b);

NO_DEBUG f32 v4_length(v4 v);
NO_DEBUG f32 v4_length_squared(v4 v);
NO_DEBUG v4  v4_norm(v4 v);
NO_DEBUG f32 v4_distance(v4 a, v4 b);
NO_DEBUG f32 v4_distance_sqr(v4 a, v4 b);
NO_DEBUG f32 v4_dot(v4 v);
NO_DEBUG v4  v4_lerp(v4 a, f32 t, v4 b);
NO_DEBUG v4  v4_hadamard(v4 a, v4 b);

Transform transform_mul(Transform a, Transform b);
v3 transform_point(Transform t, v3 p);
v3 transform_inverse_point(Transform t, v3 p);
v3 transform_direction(Transform t, v3 v);

////////////////////////////////////////////////////////////////////////
// Quatornion
v4 quat_identity();
v3 quat_forward(v4 q);
v3 quat_right(v4 q);
v3 quat_up(v4 q);
v4 quat_norm(v4 q);
v4 quat_conjugate(v4 q);
v4 quat_inverse(v4 q);
v4 quat_axis_angle(v3 axis, f32 rad);
v3 quat_rotate(v4 q, v3 v);
v4 quat_mul(v4 a, v4 b);
f32 quat_angle(v4 a, v4 b);
v4 quat_slerp(v4 a, f32 t, v4 b);
v4 quat_nlerp(v4 a, f32 t, v4 b);
v4 quat_rotate_towards(v4 a, v4 b, f32 max_rad);
v4 quat_from_to(v3 a, v3 b);
v4 quat_from_euler(v3 e);
v4 quat_look_rotation(v3 dir, v3 up);
v3 quat_rotate_point(v4 q, v3 point, v3 pivot);

////////////////////////////////////////////////////////////////////////
// m2x2
m2x2 operator*(m2x2 a, m2x2 b);
m2x2& operator*=(m2x2& a, m2x2 b);
v2 operator*(m2x2 mat, v2 v);
m2x2 m2x2_identity();
m2x2 m2x2_rotate(f32 rad);
m2x2 m2x2_scale(v2 scale);
m2x2 m2x2_inverse(m2x2 m);
m2x2 m2x2_scale_all_elements(m2x2 mat, f32 scale);

////////////////////////////////////////////////////////////////////////
// m3x2
m3x2 operator*(m3x2 a, m3x2 b);
m3x2& operator*=(m3x2 a, m3x2 b);
v2 operator*(m3x2 mat, v2 v);

m3x2 m3x2_identity();
m3x2 m3x2_translate(v2 pos);
m3x2 m3x2_scale(v2 scale);
m3x2 m3x2_rotate(f32 rad);
m3x2 m3x2_transform(v2 pos, v2 scale, f32 rad);

////////////////////////////////////////////////////////////////////////
// m3x3
m3x3 operator*(m3x3 a, m3x3 b);
m3x3& operator*=(m3x3& a, m3x3 b);
v3 operator*(m3x3 mat, v3 vec);

m3x3 m3x3_identity();
m3x3 m3x3_translate(v2 pos);
m3x3 m3x3_scale(v2 scale);
m3x3 m3x3_rotate(f32 rad);
m3x3 m3x3_inverse(m3x3 m);
m3x3 m3x3_scale_all_elements(m3x3 m, f32 scale);

////////////////////////////////////////////////////////////////////////
// m4x3
m4x3 operator*(m4x3 a, m4x3 b);
m4x3& operator*=(m4x3& a, m4x3 b);
v3 operator*(m4x3 mat, v3 v);
m4x3 m4x3_identity();
m4x3 m4x3_translate(v3 pos);
m4x3 m4x3_scale(v3 scale);
m4x3 m4x3_scale_translate(v3 scale, v3 pos);
m4x3 m4x3_transform(v3 scale, v3 pos, v4 q);
m4x3 m4x3_from_quat(v4 q);

////////////////////////////////////////////////////////////////////////
// m4x4
v3 m4x4_forward(m4x4 matrix);
v3 m4x4_backward(m4x4 matrix);
v3 m4x4_up(m4x4 matrix);
v3 m4x4_down(m4x4 matrix);
v3 m4x4_right(m4x4 matrix);
v3 m4x4_left(m4x4 matrix);

m4x4 operator*(m4x4 a, m4x4 b);
m4x4& operator*=(m4x4& a, m4x4 b);
v4 operator*(m4x4 mat, v4 v);
v3 operator*(m4x4 mat, v3 v);

m4x4 m4x4_identity();
m4x4 m4x4_translate(v3 pos);
m4x4 m4x4_scale(v3 scale);
m4x4 m4x4_scale_translate(v3 scale, v3 pos);
m4x4 m4x4_transform(v3 pos, v3 scale, v4 q);
m4x4 m4x4_transform(Transform trans);
m4x4 m4x4_scale_all_elements(m4x4 mat, f32 scale);
m4x4 m4x4_rotate_x(f32 rad);
m4x4 m4x4_rotate_y(f32 rad);
m4x4 m4x4_rotate_z(f32 rad);
m4x4 m4x4_rotate_xyz(v3 rad);
m4x4 m4x4_rotate_around_axis(v3 axis, f32 rad);
m4x4 m4x4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
m4x4 m4x4_orthographic_inverse(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
m4x4 m4x4_perspective(f32 fov_radians, f32 aspect_ratio, f32 near, f32 far);
m4x4 m4x4_perspective_inverse(f32 fov, f32 aspect, f32 near, f32 far);
m4x4 m4x4_look_at(v3 pos, v3 target, v3 up = v3_up());
m4x4 m4x4_look_at_inverse(v3 pos, v3 target, v3 up = v3_up());
m4x4 m4x4_transpose(m4x4 matrix);
m4x4 m4x4_inverse(m4x4 matrix);
m4x4 m4x4_from_quat(v4 q);
m4x4 m4x4_from_m4x3(m4x3 m);
m4x4 m4x4_inverse_rigid(m4x4 m);

////////////////////////////////////////////////////////////////////////
// Misc
Ray ray_make(v3 pos, v3 dir);
Ray ray_from_screen(v2 screen_pos, v2u viewport_rect, v3 origin, m4x4 view, m4x4 projection);

////////////////////////////////////////////////////////////////////////
// Range Ops

///////////////////////////////////
// Dim1
Rng1u rng1u_shift(Rng1u r, u32 x);
Rng1u rng1u_pad(Rng1u r, u32 x);
u32 rng1u_center(Rng1u r);
b32 rng1u_contains(Rng1u r, u32 x);
u32 rng1u_dim(Rng1u r);
Rng1u rng1u_union(Rng1u a, Rng1u b);
Rng1u rng1u_intersect(Rng1u a, Rng1u b);
u32 rng1u_clamp(Rng1u r, u32 v);

Rng1i rng1i_shift(Rng1i r, i32 x);
Rng1i rng1i_pad(Rng1i r, i32 x);
i32 rng1i_center(Rng1i r);
b32 rng1i_contains(Rng1i r, i32 x);
i32 rng1i_dim(Rng1i r);
Rng1i rng1i_union(Rng1i a, Rng1i b);
Rng1i rng1i_intersect(Rng1i a, Rng1i b);
i32 rng1i_clamp(Rng1i r, i32 v);

Rng1u64 rng1u64_shift(Rng1u64 r, u64 x);
Rng1u64 rng1u64_pad(Rng1u64 r, u64 x);
u64 rng1u64_center(Rng1u64 r);
b32 rng1u64_contains(Rng1u64 r, u64 x);
u64 rng1u64_dim(Rng1u64 r);
Rng1u64 rng1u64_union(Rng1u64 a, Rng1u64 b);
Rng1u64 rng1u64_intersect(Rng1u64 a, Rng1u64 b);
u64 rng1u64_clamp(Rng1u64 r, u64 v);

Rng1 rng1_shift(Rng1 r, f32 x);
Rng1 rng1_pad(Rng1 r, f32 x);
f32 rng1_center(Rng1 r);
b32 rng1_contains(Rng1 r, f32 x);
f32 rng1_dim(Rng1 r);
Rng1 rng1_union(Rng1 a, Rng1 b);
Rng1 rng1_intersect(Rng1 a, Rng1 b);
b32 rng1_overlaps(Rng1 a, Rng1 b);
f32 rng1_clamp(Rng1 r, f32 v);

f32 rng1_lerp(Rng1 r, f32 t);
f32 rng1_unlerp(Rng1 r, f32 x);
f32 rng1_remap(f32 x, Rng1 from, Rng1 to);
Rng1 rng1_subrng(Rng1 r, Rng1 sub);
Rng1 rng1_subrng01(Rng1 r, Rng1 sub);
f32 rng1_align_center(Rng1 r, f32 size);

///////////////////////////////////
// Dim2
f32 rng2u_width(Rng2u r);
f32 rng2u_height(Rng2u r);

f32 rng2_width(Rng2 r);
f32 rng2_height(Rng2 r);
v2 rng2_top_right(Rng2 r);
v2 rng2_bottom_left(Rng2 r);

Rng2 rng2_shift(Rng2 r, v2 x);
Rng2 rng2_pad(Rng2 r, f32 x);
Rng2 rng2_pad(Rng2 r, v2 x);
v2 rng2_center(Rng2 r);
b32 rng2_contains(Rng2 r, v2 x);
v2 rng2_dim(Rng2 r);
Rng2 rng2_union(Rng2 a, Rng2 b);
Rng2 rng2_intersection(Rng2 a, Rng2 b);
b32 rng2_intersect(Rng2 a, Rng2 b);
v2 rng2_clamp(Rng2 r, v2 x);
Rng1 rng2_rng_x(Rng2 r);
Rng1 rng2_rng_y(Rng2 r);

Rng2 rng2_lerp(Rng2 a, f32 t, Rng2 b);
v2 rng2_remap(v2 p, Rng2 from, Rng2 to);
v2 rng2_remap(v2 p, v2 from, Rng2 to);
Rng2 rng2_remap_rng(Rng2 r, Rng2 from, Rng2 to);
Rng2 rng2_remap_rng(Rng2 r, v2 from, Rng2 to);
Rng2 rng2_make(v2 min, v2 size);             
Rng2 rng2_make_centered(v2 pos, v2 halfdim); 
Rng2 rng2_scale_centered(Rng2 r, v2 scale);  
Rng2 rng2_scale(Rng2 r, v2 scale);           
Rng2 rng2_subrng_x(Rng2 r, Rng1 x);
Rng2 rng2_subrng_y(Rng2 r, Rng1 y);
Rng2 rng2_subrng_x01(Rng2 r, Rng1 sub);
Rng2 rng2_subrng_y01(Rng2 r, Rng1 sub);
Rng2 rng2_cut_left(Rng2* r, f32 amount);
Rng2 rng2_cut_right(Rng2* r, f32 amount);
Rng2 rng2_cut_top(Rng2* r, f32 amount);
Rng2 rng2_cut_bottom(Rng2* r, f32 amount);
void rng2_split_x(Rng2 r, f32 t, Rng2* left, Rng2* right);
void rng2_split_y(Rng2 r, f32 t, Rng2* top, Rng2* bottom);
Rng2 rng2_col(Rng2 r, i32 idx, i32 count);
Rng2 rng2_row(Rng2 r, i32 idx, i32 count);
Rng2 rng2_grid_cell(Rng2 r, i32 row, i32 col, i32 rows, i32 cols);
Rng2 rng2_align_dim_at_center(Rng2 r, v2 size);
Rng2 rng2_aspect_fit(Rng2 r, Rng2 fit);
Rng2 rng2_top(Rng2 r, f32 height);
Rng2 rng2_bottom(Rng2 r, f32 height);
Rng2 rng2_left(Rng2 r, f32 width);
Rng2 rng2_right(Rng2 r, f32 width);

///////////////////////////////////
// Dim3
Rng3 rng3_shift(Rng3 r, v3 x);
Rng3 rng3_pad(Rng3 r, f32 x);
v3 rng3_center(Rng3 r);
b32 rng3_contains(Rng3 r, v3 x);
v3 rng3_dim(Rng3 r);
Rng3 rng3_union(Rng3 a, Rng3 b);
Rng3 rng3_intersect(Rng3 a, Rng3 b);
v3 rng3_clamp(Rng3 r, v3 x);

Rng3 rng3_make(v3 min, v3 size);
Rng3 rng3_make_centered(v3 pos, v3 halfdim);
Rng3 rng3_scale_centered(Rng3 r, f32 scale);
Rng3 rng3_scale_centered(Rng3 r, v3 scale);
Rng3 rng3_scale(Rng3 r, f32 scale);
Rng3 rng3_scale(Rng3 r, v3 scale);

f32 smoothstep(f32 t);
f32 smootherstep(f32 t);

////////////////////////////////////////////////////////////////////////
// Ease
f32 ease_quad_in(f32 t);
f32 ease_quad_out(f32 t);
f32 ease_quad_in_out(f32 t);

f32 ease_cube_in(f32 t);
f32 ease_cube_out(f32 t);
f32 ease_cube_in_out(f32 t);

f32 ease_quart_in(f32 t);
f32 ease_quart_out(f32 t);
f32 ease_quart_in_out(f32 t);

f32 ease_quint_in(f32 t);
f32 ease_quint_out(f32 t);
f32 ease_quint_in_out(f32 t);

f32 ease_sin_in(f32 t);
f32 ease_sin_out(f32 t);
f32 ease_sin_in_out(f32 t);

f32 ease_circ_in(f32 t);
f32 ease_circ_out(f32 t);
f32 ease_circ_in_out(f32 t);

f32 ease_exp_in(f32 t);
f32 ease_exp_out(f32 t);
f32 ease_exp_in_out(f32 t);

f32 ease_elastic_in(f32 t);
f32 ease_elastic_out(f32 t);
f32 ease_elastic_in_out(f32 t);

f32 ease_back_in(f32 t);
f32 ease_back_out(f32 t);
f32 ease_back_in_out(f32 t);

f32 ease_bounce_in(f32 t);
f32 ease_bounce_out(f32 t);
f32 ease_bounce_in_out(f32 t);

////////////////////////////////////////////////////////////////////////
// @Mem
const u32 MEM_DEFAULT_ALIGNMENT = sizeof(void*);
const u32 ARENA_DEFAULT_RESERVE_SIZE = MB(64);
const u32 ARENA_DEFAULT_COMMIT_SIZE  = KB(4);

enum AllocatorType {
	AllocatorType_Arena = 1,
	AllocatorType_ArenaList,
	AllocatorType_Alloc,
};

struct Allocator {
	AllocatorType type;
	void* ctx;
};

struct AllocatorInfo {
	AllocatorInfo* first;
	AllocatorInfo* last;
	AllocatorInfo* next;
	AllocatorInfo* prev;
	AllocatorInfo* parent;
	u32 child_count;
	String64 name;
	String64 file;
	u32 line;
	AllocatorType type;
	u32 thread_idx;
	u64 pos;
	u64 children_size;
	u64 cap;
	u64 allocs_count;
	u64 frees_count;
	u64 allocs_per_frame;
};

struct AllocatorInfoList {
	AllocatorInfo* first;
	AllocatorInfo* last;
	u32 count;
};

struct ArenaParams {
	String name;
	String file;
	u32 line;
	u64 reserve_size;
	u64 commit_size;
	b32 lock;
};

struct Arena {
#if MEM_TRACK
	AllocatorInfo* info;
#endif
	u8* base;
	u64 pos;
	u64 cmt;
	u64 cap;
	b32 lock;
	Mutex mutex;
	operator Allocator();
};

struct Temp {
	Arena* arena;
	u64 pos;
};

struct ArenaBlock {
	u64 pos;
	u64 cap;
	ArenaBlock* next;
};

struct ArenaList {
	Allocator alloc;
	ArenaBlock* current;
	ArenaBlock* first;
	operator Allocator();
};

struct MemNode {
	MemNode* next;
};

struct AllocParams {
	String name;
	String file;
	u32 line;
	b32 lock;
};

struct Alloc {
#if MEM_TRACK
	AllocatorInfo* info;
#endif
	Allocator alloc;
	MemNode pools[32];
	b32 lock;
	Mutex mutex;
	operator Allocator();
};

typedef u32 GpuMemId;
struct GpuBlockList {
	u32 next;
	b32 is_allocated;
	Region range;
};

struct GpuAllocSegList {
	u64 pos;
	u64 cap;
	Allocator alloc;
	GpuBlockList* data;
	u32 range_count;
	u32 range_cap;
	u32 heads[32];
};

struct SoA_Field {
	void** dst_ptr;
	u32 elem_size;
	u32 align;
};

struct MemFormatSize {
	String format;
	u32 format_id;
	f32 size;
};

const u32 MemTrackMaxAllocators = 128;

struct MemTrackState {
	Mutex mutex;
	AllocatorInfoList roots;
	AllocatorInfo* free;
	AllocatorInfo infos[MemTrackMaxAllocators];
	u32 infos_alocs_per_frame[MemTrackMaxAllocators];
	u32 allocated_infos_count;
};

u8*  mem_alloc(Allocator alloc, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
u8*  mem_alloc_zero(Allocator alloc, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
u8*  mem_realloc(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
u8*  mem_realloc_zero(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align = MEM_DEFAULT_ALIGNMENT);
void mem_free(Allocator alloc, void* ptr, u64 size);

#define mem_realloc_array(a, ptr, old_c, c)      (decltype(ptr))mem_realloc((a), (ptr), sizeof(*(ptr))*(old_c), sizeof(*(ptr))*(c), alignof(*(ptr)))
#define mem_realloc_array_zero(a, ptr, old_c, c) (decltype(ptr))mem_realloc_zero((a), (ptr), sizeof(*(ptr))*(old_c), sizeof(*(ptr))*(c), alignof(*(ptr)))
#define push_buffer(a, z, ...)           mem_alloc(a,      z, ##__VA_ARGS__)
#define push_buffer_zero(a, z, ...)      mem_alloc_zero(a, z, ##__VA_ARGS__)
#define push_struct(a, T)            (T*)mem_alloc(a,      sizeof(T),     alignof(T))
#define push_struct_zero(a, T)       (T*)mem_alloc_zero(a, sizeof(T),     alignof(T))
#define push_array(a, T, c)          (T*)mem_alloc(a,      sizeof(T)*(c), alignof(T))
#define push_array_zero(a, T, c)     (T*)mem_alloc_zero(a, sizeof(T)*(c), alignof(T))

template<typename T> Slice<T> slice_free(Allocator alloc, Slice<T> slice) {
	mem_free(alloc, slice.data);
}
template<typename T> Slice<T> slice_clone(Allocator alloc, Slice<T> slice) {
	T* data = push_array(alloc, T, slice.count);
	MemCopyArray(data, slice.data, slice.count);
	return {data, slice.count};
}
#define push_slice(a, T, c) Slice(push_array(a, T, c), c)
#define push_slice_zero(a, T, c) Slice(push_array_zero(a, T, c), c)
#define push_buffer_slice(a, z) Slice(push_buffer(a, z), z)

AllocatorInfoList mem_track_info();
void mem_track_init();
void mem_track_end();

#define arena_make(...) _arena_make(ArenaParams{.reserve_size = ARENA_DEFAULT_RESERVE_SIZE, .commit_size = ARENA_DEFAULT_COMMIT_SIZE, .name = __func__, .file = __FILE__, .line = __LINE__, __VA_ARGS__})
Arena _arena_make(ArenaParams params);
void  arena_destroy(Arena& arena);
void  arena_clear(Arena& arena);

Temp temp_begin(Arena* arena);
void temp_end(Temp temp);

ArenaList alloc_arena_list_make(Allocator alloc);
void alloc_arena_list_clear(ArenaList& arena);

#define alloc_make(alloc, ...) _alloc_make(alloc, AllocParams{.name = __func__, .file = __FILE__, .line = __LINE__, __VA_ARGS__})
Alloc _alloc_make(Allocator alloc, AllocParams params);
void alloc_destroy(Alloc alloc);

////////////////////////////////////////////////////////////////////////
// tlsf TODO: implement

// #define BinCount      32
// #define SubbinCount   32
// #define MinAllocation 128
// struct TLSF_Allocator {
//   u32 bin_bitmap;
//   u32 sub_bin_bitmaps[BinCount];
// };
// TLSF_Allocator tlsf_init();
// u8* tlsf_alloc(TLSF_Allocator& allocator, u64 size, u64 alignment = MEM_DEFAULT_ALIGNMENT);

////////////////////////////////////////////////////////////////////////
// atlas TODO: implement

////////////////////////////////////////////////////////////////////////
// GPU segregated pow2 list
GpuAllocSegList gpu_alloc_seglist_make(Allocator alloc);
GpuMemId gpu_alloc_seglist_alloc(GpuAllocSegList& a, u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
void gpu_alloc_seglist_free(GpuAllocSegList& a, GpuMemId h);
u64 gpu_alloc_seglist_get(GpuAllocSegList& a, GpuMemId h);

#define SoA_push_field(ptr) {(void**)(&ptr), sizeof(*ptr), alignof(*ptr)}

u64 mem_soa_size(u32 count, Slice<SoA_Field> fields);
u8* mem_alloc_soa(Allocator alloc, u32 count, Slice<SoA_Field> fields);
u8* mem_realloc_soa(Allocator alloc, u32 old_count, u32 new_count, Slice<SoA_Field> fields);
u8* mem_alloc_soa_zero(Allocator alloc, u32 count, Slice<SoA_Field> fields);
u8* mem_realloc_soa_zero(Allocator alloc, u32 old_count, u32 new_count, Slice<SoA_Field> fields);

u64 offset_push(u64& offset, u64 size, u64 align = 1);
MemFormatSize mem_format_size(f32 value);

////////////////////////////////////////////////////////////////////////
// @TCTX

struct Scratch {
	Temp temp;
	NO_DEBUG operator Allocator();
	Scratch();
	NO_DEBUG Scratch(Allocator conflict);
	NO_DEBUG ~Scratch();
};

struct TCTX {
	Arena arenas[2];
	u32 id;
};

void tctx_init();
u32 tctx_id();

////////////////////////////////////////////////////////////////////////
// @Str

struct Dstring {
	u8* str;
	u32 size;
	u32 cap;
	Allocator alloc;
	operator String();
};

struct StringNode {
	StringNode* next;
	String string;
};

struct StringList {
	StringNode* first;
	StringNode* last;
	u32 node_count;
	u32 total_size;
};

Slice<u8> dstr_slice(Dstring dstr);
Dstring dstr_make(Allocator alloc);
void dstr_push(Dstring& arr, String str);
void dstr_pushf(Dstring& arr, String fmt, ...);
void dstr_clear(Dstring& arr);

u64 cstr_length(const void* c);

u32 u32_length(u32 x);
u32 u64_length(u64 x);
u32 u32_write(u8* dest, u32 x);
u32 u64_write(u8* dest, u64 x);
u32 i32_length(i32 x);
u32 i64_length(i64 x);
u32 i32_write(u8* dest, i32 x);
u32 i64_write(u8* dest, i64 x);
u32 f32_length(f32 x, u32 precision);
u32 f64_length(f64 x, u32 precision);
u32 f32_write(u8* dest, f32 value, u32 precision);
u32 f64_write(u8* dest, f64 value, u32 precision);
void u64_hex_write(u8* dest, u64 value);
u32 my_sprintf(u8* buf, String fmt, VaList argc);

////////////////////////////////////////////////////////////////////////
// Character Classification & Conversion Functions

b32 char_is_space(u8 c);
b32 char_is_newline(u8 c);
b32 char_is_ws(u8);
b32 char_is_upper(u8 c);
b32 char_is_lower(u8 c);
b32 char_is_alpha(u8 c);
b32 char_is_slash(u8 c);
b32 char_is_digit(u8 c);
u8 char_to_lower(u8 c);
u8 char_to_upper(u8 c);
u8 char_to_correct_slash(u8 c);
b32 char_is_number_cont(u8 c);

////////////////////////////////////////////////////////////////////////
// String Constructors

String str_make(Slice<u8> str);
#define S(str) String((u8*)str, sizeof(str)-1)
String str_range(u8* first, u8* one_past_last);

////////////////////////////////////////////////////////////////////////
// String Stylization

String upper_from_str(Allocator arena, String string);
String lower_from_str(Allocator arena, String string);

////////////////////////////////////////////////////////////////////////
// String Matching

b32 str_match(String str0, String str1);
b32 str_matchi(String str0, String str1);
u64 str_find_needle(String string, String needle);
b32 str_ends_with(String string, String end);

////////////////////////////////////////////////////////////////////////
// String Slicing

String str_substr(String str, Rng1u range);
String str_prefix(String str, u64 size);
String str_skip(String str, u64 amt);
String str_postfix(String str, u64 size);
String str_chop(String str, u64 amt);

////////////////////////////////////////////////////////////////////////
// String Formatting & Copying

String push_str_cat(Allocator arena, String s1, String s2);
String push_str_copy(Allocator arena, String s);
String push_strfv(Allocator arena, String fmt, VaList args);
String push_strf(Allocator arena, String fmt, ...);

////////////////////////////////////////////////////////////////////////
// String List Construction Functions

StringNode* str_list_push_node(StringList&, StringNode* node);
StringNode* str_list_push(Allocator arena, StringList& list, String string);
StringNode* str_list_pushf(Allocator arena, StringList& list, String fmt, ...);

////////////////////////////////////////////////////////////////////////
// String utils

void str_copy(String64& dest, String str);
String str_next_word(String line, u32& start);
// String str_read_line(Range* range);
String str_trim(String string);
i32 str_index_of_char(String str, u8 c);

////////////////////////////////////////////////////////////////////////
// String <=> Integer Conversions
// ...

u64 u64_from_str(String str);
i64 i64_from_str(String str);
f64 f64_from_str(String str);
u32 u32_from_str(String str);
i32 i32_from_str(String str);
f32 f32_from_str(String str);

////////////////////////////////////////////////////////////////////////
// String Path Helpers

// one/two/three -> one/two
String str_chop_last_slash(String s);

// one/two/three -> one/two/
String str_chop_past_last_slash(String s);

// one/two/three -> two/three
String str_skip_slash(String s);

// one/two/three -> three
String str_skip_last_slash(String s);

// name.fmt -> name
String str_chop_last_dot(String s);

// name.fmt -> fmt
String str_skip_last_dot(String s);

////////////////////////////////////////////////////////////////////////
// Wchar stuff

u64 wchar_to_char(char* out, const wchar_t* in, u64 out_size);
String push_str_wchar(Allocator arena, const wchar_t* in, u32 wchar_length);

////////////////////////////////////////////////////////////////////////
// @Containers
const u32 DEFAULT_CAPACITY = 8;
const u32 DEFAULT_RESIZE_FACTOR = 2;

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
template<typename T, i32 N> u32 array_push_elems(Array<T, N>& arr, Slice<T> elems) {
	Assert(arr.count + elems.count <= arr.cap);
	MemCopyArray(arr.data + arr.count, elems.data, elems.count);
	u32 res = arr.count;
	arr.count += elems.count;
	return res;
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

////////////////////////////////////////////////////////////////////////
// SparseSet
template<typename T, i32 N> struct SparseSet {
	static constexpr i32 cap = N;
	u32 count;
	u32 sparse[N];
	u32 dense[N];
	T data[N];
};

template<typename T, i32 N, typename Handle> T& sparse_set_get(SparseSet<T, N>& a, Handle h) {
	Assert(h.idx < a.count);
	Assert(a.gens[h.idx]== id_generation(h.gen));
	u32 idx = a.sparse[h.idx];
	return a.data[idx];
}
template<typename T, i32 N, typename Handle> Handle sparse_set_push(SparseSet<T, N>& a, T elem) {
	Assert(a.count < a.cap);
	u32 idx = a.count++;
	a.sparse[idx] = idx;
	a.dense[idx] = idx;
	a.data[idx] = elem;
	Handle res = {idx, a.gens[idx]};
	return res;
}
template<typename T, i32 N, typename Handle> void sparse_set_remove(SparseSet<T, N>& a, Handle h) {
	Assert(h.idx < a.count);
	u32 idx_removed = a.sparse[h.idx];
	u32 idx_last = a.count - 1;
	a.data[idx_removed] = a.data[idx_last];
	u32 last_entity = a.dense[idx_last];
	a.sparse[last_entity] = idx_removed;
	a.dense[idx_removed] = last_entity;
	--a.count;
}

template<i32 N> struct SparseSetIdx {
	u32 count;
	u32 sparse[N];
	u32 dense[N];
};

template<i32 N> void sparse_set_push(SparseSetIdx<N>& a, u32 id) {
	Assert(a.count < N);
	a.sparse[id] = a.count;
	a.dense[a.count] = id;
	a.count++;
}
template<i32 N> void sparse_set_remove(SparseSetIdx<N>& a, u32 id) {
	Assert(id < N);
	u32 idx_removed = a.sparse[id];
	u32 idx_last = a.count - 1;
	u32 last_entity = a.dense[idx_last];
	a.sparse[last_entity] = idx_removed;
	a.dense[idx_removed] = last_entity;
	--a.count;
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
template<typename T, i32 N> void pool_clear(Poolu32<T, N>& p) {
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
	u8 gens[N];
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
// DPool
template<typename T, typename Handle> struct DPool {
	static_assert(sizeof(T) >= 4);
	u32 head;
	u32 cap;
	u32 max_idx;
	Allocator alloc;
	T* data;
	u8* gens;
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
	NoOp(read);
	// if(write - read >= q.cap) return false;
	Assert(write - read < q.cap);
	q.data[write % q.cap] = elem;
	atomic_inc_explicit(&q.write, AtomicRelease);
	// return true;
}
template<typename T, i32 N> T queue_pop(QueueSPSC<T, N>& q) {
	u32 read = atomic_load_explicit(&q.read, AtomicRelaxed);
	u32 write = atomic_load_explicit(&q.write, AtomicAcquire);   
	NoOp(write);
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
	NoOp(write);
	NoOp(read);
	// if(write - read >= q.cap) return {};
	Assert(write - read < q.cap);
	// return {q.data[(q.write-1) % q.cap], true};
	return q.data[(q.write-1) % q.cap];
}
template<typename T, i32 N> T queue_front(QueueSPSC<T, N>& q) {
	u32 read = atomic_load_explicit(&q.read, AtomicRelaxed);
	u32 write = atomic_load_explicit(&q.write, AtomicAcquire);   
	NoOp(write);
	NoOp(read);
	// if(read == write) return {};
	Assert(read != write);
	// return {q.data[(q.read) % q.cap], true};
	return q.data[(q.read) % q.cap];
}
template<typename T, i32 N> u32 queue_count(QueueSPSC<T, N>& q) { return atomic_load_explicit(&q.write, AtomicAcquire) - atomic_load_explicit(&q.read, AtomicRelaxed); }

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
			if(atomic_cmp_swap_weak(&q.write, &write, write+1)) {
				slot.elem = elem;
				atomic_store(&slot.seq, write+1);
				return true;
			}
		} else if(dif < 0) return false; // full
	}
}
template<typename T, i32 N> b32 queue_pop(QueueMPMC<T, N>& q, T* out) {
	For {
		u32 read = atomic_load(&q.read);
		var& slot = q.data[read % q.cap];
		u64 seq = atomic_load(&slot.seq);
		i64 dif = (i64)seq - (i64)(read+1);
		if(dif == 0) {
			if(atomic_cmp_swap_weak(&q.read, &read, read+1)) {
				T res = slot.elem;
				atomic_store(&slot.seq, read + N);
				*out = res;
				return true;
			}
		} else if(dif < 0) return false; // empty
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
// Map
#define HASH_TOMBSTONE U64_MAX
#define HASH_UNUSED 0

template<typename V, i32 N> struct Map {
	u32 count;
	static constexpr u32 cap = N;
	V data[N];
	u64 hashes[N];
};

template<typename T, i32 N> T* map_set(Map<T, N>& m, u64 hash, T val) {
	Assert(m.count < m.cap);
	u64 idx = mod_pow2(hash, m.cap);
	while(m.hashes[idx] != HASH_UNUSED) {
		if(m.hashes[idx] == hash) break;
		idx = mod_pow2(idx + 1, m.cap);
	}
	m.hashes[idx] = hash;
	m.data[idx] = val;
	m.count++;
	return &m.data[idx];
}
template<typename T, i32 N> b32 map_get(Map<T, N>& m, u64 hash, T* out) {
	u64 idx = mod_pow2(hash, m.cap);
	Loop(i, m.cap) {
		if(m.hashes[idx] == hash) {
			*out = m.data[idx];
			return true;
		} 
		else if(m.hashes[idx] == HASH_UNUSED) break;
		idx = mod_pow2(idx + 1, m.cap);
	}
	return false;
}
template<typename T, i32 N> void map_remove(Map<T, N>& m, u64 hash) {
	u64 idx = mod_pow2(hash, m.cap);
	Loop(i, m.cap) {
		if(m.hashes[idx] == hash) {
			m.hashes[idx] = HASH_TOMBSTONE;
			m.count--;
			return;
		} else if(m.hashes[idx] == HASH_UNUSED) break;
		idx = mod_pow2(idx + 1, m.cap);
	}
}
template<typename T, i32 N> void map_clear(Map<T, N>& m, u64 hash) {
	m.count = 0;
	ArrayZero(m.hashes);
}
template<typename T, i32 N> b32 map_exists(Map<T, N>& m, u64 hash) {
	u64 idx = mod_pow2(hash, m.cap);
	Loop(i, m.cap) {
		if(m.hashes[idx] == hash) {
			return true;
		} 
		else if(m.hashes[idx] == HASH_UNUSED) break;
		idx = mod_pow2(idx + 1, m.cap);
	}
	return false;
}

///////////////////////////////////
// DMap
template<typename V> struct DMap {
	static constexpr f32 LF = 0.8;
	u32 count;
	u32 cap;
	Allocator alloc;
	V* data;
	u64* hashes;
};

template<typename V> DMap<V> map_make(Allocator alloc) {
	DMap<V> res = {
		.alloc = alloc,
	};
	return res;
}
template<typename T> void map_grow(DMap<T>& m) {
	if(m.data) {
		T* old_data = m.data;
		u64* old_hashes = m.hashes;
		u32 old_cap = m.cap;
		m.cap *= DEFAULT_RESIZE_FACTOR;
		SoA_Field fields[] = {
			SoA_push_field(m.data),
			SoA_push_field(m.hashes),
		};
		mem_alloc_soa(m.alloc, m.cap, slice(fields));
		Loop(i, old_cap) {
			if(old_hashes[i] != HASH_UNUSED && old_hashes[i] != HASH_TOMBSTONE) {
				map_set(m, old_hashes[i], old_data[i]);
			}
		}
		mem_free(m.alloc, old_data, mem_soa_size(old_cap, slice(fields)));
	} else {
		m.cap = DEFAULT_CAPACITY;
		SoA_Field fields[] = {
			SoA_push_field(m.data),
			SoA_push_field(m.hashes),
		};
		mem_alloc_soa(m.alloc, m.cap, slice(fields));
	}
}
template<typename T> T* map_set(DMap<T>& m, u64 hash, T val) {
	if(m.count >= m.cap*m.LF) map_grow(m);
	u64 idx = mod_pow2(hash, m.cap);
	while(m.hashes[idx] != HASH_UNUSED) {
		if(m.hashes[idx] == hash) break;
		idx = mod_pow2(idx + 1, m.cap);
	}
	m.hashes[idx] = hash;
	m.data[idx] = val;
	m.count++;
	return &m.data[idx];
}
template<typename T> b32 map_get(DMap<T>& m, u64 hash, T* out) {
	if(!m.data) return {};
	u64 idx = mod_pow2(hash, m.cap);
	Loop(i, m.cap) {
		if(m.hashes[idx] == hash) {
			*out = m.data[idx];
			return true;
		} 
		else if(m.hashes[idx] == HASH_UNUSED) break;
		idx = mod_pow2(idx + 1, m.cap);
	}
	return false;
}
template<typename T> void map_remove(DMap<T>& m, u64 hash) {
	u64 idx = mod_pow2(hash, m.cap);
	Loop(i, m.cap) {
		if(m.hashes[idx] == hash) {
			m.hashes[idx] = HASH_TOMBSTONE;
			m.count--;
			return;
		} else if(m.hashes[idx] == HASH_UNUSED) break;
		idx = mod_pow2(idx + 1, m.cap);
	}
}
template<typename T> void map_clear(DMap<T>& m) {
	m.count = 0;
	MemZeroArray(m.hashes, m.cap);
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
u32 sort_i32_key_to_u32(i32 x);
u32 sort_f32_key_to_u32(f32 sort_key);

struct SortEntry {
	u32 sort_key;
	u32 idx;
};
struct SortEntry64 {
	u64 sort_key;
	union {
		u32 idx;
		void* p;
	};
};

void sort_radix(Slice<SortEntry> arr);
void sort_radix_msd(Slice<SortEntry> arr);
void sort_radix64(Slice<SortEntry64> arr);
void sort_radix64_msd(Slice<SortEntry64> arr);

////////////////////////////////////////////////////////////////////////
// List sort
// template<typename T, typename Cmp> Slice<T> sort_list_insert(Allocator arena, T first, Cmp cmp) {
// 	var sorted_arr = array_make<T>(arena);
// 	for(T it = first; it != 0; it = it->next) {
// 		array_push(sorted_arr, it);
// 	}
// 	sort_insert(slice(sorted_arr), cmp);
// 	return slice(sorted_arr);
// }

////////////////////////////////////////////////////////////////////////
// @OS core
typedef u32 FilePropertyFlags;
enum {
	FilePropertyFlag_IsFolder = 1<<0,
};

struct FileProperties {
	u64 size;
	u64 modified;
	u64 created;
	FilePropertyFlags flags;
};

typedef u32 OS_FileIterFlags;
enum {
	OS_FileIterFlag_SkipFolders     = 1<<0,
	OS_FileIterFlag_SkipFiles       = 1<<1,
	OS_FileIterFlag_SkipHiddenFiles = 1<<2,
	OS_FileIterFlag_Done            = 1<<31,
};

struct OS_FileIter {
	OS_FileIterFlags flags;
	u8 memory[800];
};

struct OS_FileInfo {
	String name;
	FileProperties props;
};

typedef u32 OS_AccessFlags;
enum {
	OS_AccessFlag_Read       = 1<<0,
	OS_AccessFlag_Write      = 1<<1,
	OS_AccessFlag_Trunc      = 1<<2,
	OS_AccessFlag_Execute    = 1<<3,
	OS_AccessFlag_Append     = 1<<4,
	OS_AccessFlag_ShareRead  = 1<<5,
	OS_AccessFlag_ShareWrite = 1<<6,
};

typedef u32 OS_WatchFlags;
enum {
	OS_WatchFlag_Create = 1<<0,
	OS_WatchFlag_Delete = 1<<1,
	OS_WatchFlag_Modify = 1<<2,
};

struct OS_Watch{
	OS_Handle handle;
	OS_WatchFlags flags;
};

typedef void ThreadEntryPointFn(void* p);
struct Thread { u64 v; };
// struct Mutex { u64 v; };
struct RWMutex { u64 v; };
// struct CondVar { u64 v; };
struct Barrier { u64 v; };

struct _LockScope {
	Mutex& mutex;
	_LockScope(Mutex& mutex_);
	~_LockScope();
};
#define LockScope(m) _LockScope Glue(_lock_scope, __LINE__)(m)

String os_cur_filepath();
String os_cur_directory();
String os_cur_binary_name();
u64 os_commited_size();
u64 os_reserved_size();
Slice<String> os_args();

void os_init(Slice<char*> args);
void os_exit(i32 exit_code);

u64 os_timer_frequency();
u64 os_now_ns();
void os_sleep_ms(u64 ms);
void os_sleep_us(u64 us);
void os_console_write(String message, u32 color);
String os_get_environment(String name);

///////////////////////////////////
// Memory
u8*  os_reserve(u64 size);
b32  os_commit(void* ptr, u64 size);
void os_decommit(void* ptr, u64 size);
void os_release(void* ptr, u64 size);

//////////////////////////////////////////////////////////////////////////
// Files
OS_Handle      os_file_open(String path, OS_AccessFlags flags);
void           os_file_close(OS_Handle file);
u64            os_file_read(OS_Handle file, Slice<u8> out_data);
u64            os_file_write(OS_Handle file, Slice<u8> data);
u64            os_file_size(OS_Handle file);
FileProperties os_file_properties(OS_Handle file);
Slice<u8>      os_file_path_read_all(Allocator arena, String path);
String         os_file_path_read_all_str(Allocator arena, String path);
u64            os_file_path_write_all(String path, Slice<u8> data);
b32            os_file_path_exists(String path);
b32            os_file_path_copy(String src, String dst);
void           os_file_path_copy_mtime(String src, String dst);
FileProperties os_file_path_properties(String path);
u64      os_file_path_mtime(String path);
void           os_file_path_rename(String path, String new_name);
void           os_file_path_remove(String path);
void           os_file_path_move(String src, String dst);

///////////////////////////////////
// Directory
OS_Handle os_directory_open(String path);
OS_Handle os_directory_make(String path);
b32       os_directory_path_exist(String path);

///////////////////////////////////
// Watch
OS_Watch      os_watch_open(OS_WatchFlags flags);
void          os_watch_close(OS_Watch watch);
OS_Handle     os_watch_attach(OS_Watch watch, String name);
void          os_watch_deattach(OS_Watch watch, OS_Handle attached);
Slice<String> os_watch_check(Allocator arena, OS_Watch watch);

///////////////////////////////////
// Directory iteration
OS_FileIter*       os_file_iter_begin(Allocator arena, String path, OS_FileIterFlags flags);
b32                os_file_iter_next(Allocator arena, OS_FileIter* iter, OS_FileInfo* info_out);
void               os_file_iter_end(OS_FileIter* iter);
Slice<OS_FileInfo> os_file_iter_directory(Allocator arena, String path, OS_FileIterFlags flags);

///////////////////////////////////
// Processes
OS_Handle os_process_make(Slice<String> arr);
OS_Handle os_process_make(StringList list);
i32       os_process_join(OS_Handle handle);

///////////////////////////////////
// Threads
Thread os_thread_make(ThreadEntryPointFn* func, void* ptr);
b32 os_thread_join(Thread handle);
void os_thread_detach(Thread handle);

///////////////////////////////////
// Sync primitives
void os_futex_wait(Futex& futex, u32 expect_val);
void os_futex_wake(Futex& futex, u32 num_waiters);

void os_mutex_lock(Mutex& mutex);
b32  os_mutex_try_lock(Mutex& mutex); // returns 1 on success
void os_mutex_unlock(Mutex& mutex);

void os_cond_wait(CondVar& cv, Mutex& mutex);
void os_cond_wake_one(CondVar& cv);
void os_cond_wake_all(CondVar& cv);

void os_sem_wait(Semaphore& s);
b32 os_sem_try_wait(Semaphore& s);
void os_sem_post(Semaphore& s);

////////////////////////////////////////////////////////////////////////
// Lib
OS_Handle os_lib_open(String path);
void      os_lib_close(OS_Handle lib);
void*     os_lib_get_proc(OS_Handle lib, String name);

////////////////////////////////////////////////////////////////////////
// @OS Gfx
enum Key {
	// Control keys
	Key_Null,
	Key_Backspace = 1,
	Key_Enter,
	Key_Tab,
	Key_Delete,
	Key_LShift,
	Key_Shift = Key_LShift,
	Key_RShift,
	Key_LControl,
	Key_Ctrl = Key_LControl,
	Key_RControl,
	Key_LAlt,
	Key_Alt = Key_LAlt,
	Key_RAlt,
	Key_Escape,
	Key_Capslock,
	Key_Super,

	// Navigation
	Key_Space,
	Key_Pageup,
	Key_Pagedown,
	Key_End,
	Key_Home,
	Key_Left,
	Key_Up,
	Key_Right,
	Key_Down,

	// Special keys
	Key_Pause,
	Key_Print,
	Key_Printscreen,
	Key_Lsuper,
	Key_Rsuper,
	Key_Apps,
	Key_Numlock,

	// Numbers
	Key_0,
	Key_1,
	Key_2,
	Key_3,
	Key_4,
	Key_5,
	Key_6,
	Key_7,
	Key_8,
	Key_9,

	// Letters
	Key_A,
	Key_B,
	Key_C,
	Key_D,
	Key_E,
	Key_F,
	Key_G,
	Key_H,
	Key_I,
	Key_J,
	Key_K,
	Key_L,
	Key_M,
	Key_N,
	Key_O,
	Key_P,
	Key_Q,
	Key_R,
	Key_S,
	Key_T,
	Key_U,
	Key_V,
	Key_W,
	Key_X,
	Key_Y,
	Key_Z,

	// Function keys
	Key_F1,
	Key_F2,
	Key_F3,
	Key_F4,
	Key_F5,
	Key_F6,
	Key_F7,
	Key_F8,
	Key_F9,
	Key_F10,
	Key_F11,
	Key_F12,

	// Symbols
	Key_Semicolon,
	Key_Apostrophe,
	Key_Quote = Key_Apostrophe,
	Key_Comma,
	Key_Dot,
	Key_Equal,
	Key_Minus,
	Key_Grave,
	Key_LBracket,
	Key_RBracket,
	Key_Slash,
	Key_Backslash,

	Key_COUNT
};

enum MouseButton {
	MouseButton_Left,
	MouseButton_Right,
	MouseButton_Middle,
	MouseButton_COUNT,
};

typedef u32 OS_Modifiers;
enum {
	OS_Modifier_Ctrl  = 1<<0,
	OS_Modifier_Shift = 1<<1,
	OS_Modifier_Alt   = 1<<2,
};

enum OS_EventKind {
	OS_EventType_Key,
	OS_EventType_MouseButton,
	OS_EventType_MouseMove,
	OS_EventType_Scroll,
	OS_EventType_Modifier,
};

struct OS_InputEvent {
	OS_EventKind type;
	Key key;
	MouseButton mouse_button;
	u32 character;
	b32 is_pressed;
	OS_Modifiers modifier;
	f32 x, y;
	f32 scroll;
};

u32 os_key_to_character(Key key, OS_Modifiers modifiers);

void os_gfx_init();
void os_gfx_shutdown();

////////////////////////////////////////////////////////////////////////
// Window related
void   os_pump_messages();
b32    os_window_should_close();
void   os_window_close();
v2u    os_win_size();
u32    os_win_width();
u32    os_win_height();
v2u    os_screen_size();
u32    os_screen_width();
u32    os_screen_height();
void   os_get_gfx_handlers(void* out);
void   os_clipboard_text_set(String str);
String os_clipboard_text_get();

////////////////////////////////////////////////////////////////////////
// Cursor
void os_cursor_show();
void os_cursor_hide();
b32  os_cursor_is_hiden();
void os_cursor_confine_window();
void os_cursor_release_window();
void os_cursor_lock();
void os_cursor_unlock();

////////////////////////////////////////////////////////////////////////
// Input
void os_input_update();
Slice<OS_InputEvent> os_get_input_events();

b32 os_key_is_down(Key key);
b32 os_key_is_up(Key key);
b32 os_key_was_down(Key key);
b32 os_key_was_up(Key key);
b32 os_key_is_pressed(Key key);
b32 os_key_is_released(Key key);

OS_Modifiers os_key_modifiers();

b32 os_mouse_is_button_down(MouseButton button);
b32 os_mouse_is_button_up(MouseButton button);
b32 os_mouse_was_button_down(MouseButton button);
b32 os_mouse_was_button_up(MouseButton button);
b32 os_mouse_is_button_pressed(MouseButton button);
b32 os_mouse_is_button_released(MouseButton button);
v2  os_mouse_pos();
f32 os_mouse_wheel();
f32 os_mouse_wheel_horizontal();
v2  os_mouse_dt();
void os_mouse_set_pos(v2i pos); // works only when cursor is hidden

const char* imgui_platform_get_clipboard_text(struct ImGuiContext* ctx);
void imgui_platform_set_clipboard_text(struct ImGuiContext* ctx, const char* text);

////////////////////////////////////////////////////////////////////////
// @Thread

const u32 Thread_MaxTasks = 1024;
const u32 Thread_MaxCounters = 1024;
const u32 Thread_NumWorkers = 4;

struct WaitGroup { u32 idx; };

enum TaskPriority {
	TaskPriority_High,
	TaskPriority_Low,
	TaskPriority_COUNT,
};

typedef void TaskFn(void* ctx);

struct TaskDesc {
	void* ctx;
	TaskFn* fn;
	TaskPriority priority;
};

struct Task {
	TaskFn* fn;
	void* ctx;
	WaitGroup wg;
	TaskPriority priority;
};

typedef void (*ParallelForFn)(void* ctx, u32 base, u32 count);

struct ParallelForCtx {
	void* ctx;
	ParallelForFn fn;
	u32 next_idx;
	u32 count;
	u32 chunk_size;
};

struct WaitGroupSlot {
	u32 count;
	Futex is_done;
	u32 waiting_count;
};

struct ThreadPool {
	Arena arena;
	Thread threads[Thread_NumWorkers];

	QueueMPMC<Task, Thread_MaxTasks> tasks[TaskPriority_COUNT];
	u32 wg_write;
	WaitGroupSlot wg_slots[Thread_MaxCounters];

	Semaphore tasks_available;
	u32 working_num;
	u32 remaining_tasks;
	Futex is_waiting_remaning_tasks;

	Mutex task_mutex;
	Mutex ctx_mutex;

	u8 ctx_buffer[KB(4)];
	u32 ctx_write;
};

WaitGroup thread_wg_make(u32 count);
WaitGroup thread_push(TaskDesc desc);
WaitGroup thread_push(void* ctx, TaskFn* fn, TaskPriority prio = TaskPriority_High);
WaitGroup thread_push_batch(Slice<TaskDesc> tasks);
void thread_wg_wait(WaitGroup wg);
void thread_wait_remanings();
u8* _thread_push_ctx(u64 size, u64 align = MEM_DEFAULT_ALIGNMENT);
#define thread_push_ctx(T) *(T*)_thread_push_ctx(sizeof(T), alignof(T))
void thread_pool_init();

template<typename T, typename F> void thread_parallel_for(u32 chunk_size, Slice<T> s, F fn) {
	if(s.count == 0) return;
	Scratch scratch;
	u32 task_count = div_ceil(s.count, chunk_size);
	var tasks = push_slice_zero(scratch, TaskDesc, task_count);
	u32 task_id = 0;
	for(u32 i = 0; i < s.count; i += chunk_size) {
		struct TaskCtx {
			Slice<T> s;
			F fn;
		};
		var& ctx = thread_push_ctx(TaskCtx);
		ctx.s = slice_n(s, i, Min(chunk_size, s.size - i));
		ctx.fn = fn;
		tasks[task_id].ctx = &ctx;
		tasks[task_id].fn = [](void* ctx) {
			TaskCtx data = *(TaskCtx*)ctx;
			data.fn(data.s);
		};
		task_id++;
	}
	WaitGroup wg = thread_push_batch(tasks);
	thread_wg_wait(wg);
}
////////////////////////////////////////////////////////////////////////
// @Profile

const u32 ProfRecordHistoryNum = 30;

enum ProfType {
	ProfType_Default,
	ProfType_Sleep,
	ProfType_Worker,
	ProfType_Async,
};

struct ProfAnchor {
	ProfType type;
	u32 depth;
	u64 tsc_elapsed_excl; // without children
	// u64 hit_count;
	String label;
	String func;
	u64 tsc_start;
	u64 tsc_end;
};

enum ProfEventType {
	ProfEventType_Push,
	ProfEventType_Pop,
};

struct ProfEvent {
	ProfEventType type;
	ProfType prof_type;
	u64 tsc;
	String label;
	String func;
};

struct _ProfBlock {
	String label;
	String func;
	ProfType type;
	_ProfBlock(String label_, String func_, ProfType type_ = ProfType_Default);
	~_ProfBlock();
};

struct ProfFrameTime {
	u64 tsc_start;
	u64 tsc_end;
};

struct ProfFrame {
	ProfFrameTime frame_time;
	Slice<ProfAnchor> anchors;
};

struct ProfThread {
	DArray<ProfEvent> events[2];
	DArray<ProfAnchor> recorded_anchors[ProfRecordHistoryNum];
	DArray<ProfAnchor> launch_anchors;
	DArray<ProfAnchor> delayed_anchors;
};

struct ProfState {
	Arena arena;
	Alloc gpa;
	String strs[KB(4)];
	BitArray<KB(4)> strs_exist;
	ProfFrameTime current_frame_time;
	ProfFrameTime frames_times[ProfRecordHistoryNum];
	ProfFrameTime launch_time;
	ProfThread prof_threads[Thread_NumWorkers+1];
	u32 current_write;
	b32 paused;
};

void prof_init(Allocator arena);
void prof_begin_frame();
void prof_end_frame();
ProfFrame prof_get_prev_frame();
void prof_launch_begin();
void prof_launch_end();

#if PROFILE_BUILD
	#define ProfBlock(Name, ...) _ProfBlock Glue(__profiler_block, __LINE__)(Name, __func__, ##__VA_ARGS__)
	#define ProfFunc ProfBlock(__func__)
#else
	#define ProfBlock(Name, ...)
	#define ProfFunc
#endif

extern f32 time_dt;
extern f32 time_now;
extern u32 current_frame;
extern u64 cpu_frequency;
extern ThreadPool thread_st;
extern ProfState prof_st;



#pragma once

#define MEM_GUARD 1
#define MEM_TRACK 1
#define PROFILE_BUILD 1
#define DEV_BUILD 1

// template<typename _Tp> struct _TypeIdentity { typedef _Tp type; };
// template<typename _Tp> using TypeIdentity = _TypeIdentity<_Tp>::type;

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

	typedef __INT8_TYPE__  i8;
	typedef __INT16_TYPE__ i16;
	typedef __INT32_TYPE__ i32;
	typedef __INT64_TYPE__ i64;

	typedef float  f32;
	typedef double f64;

	typedef u8  b8;
	typedef i32 b32;
#else 
	#error OS not supported
#endif

////////////////////////////////////////////////////////////////////////
// OS

#if OS_WINDOWS
	#define shared_function C_LINKAGE __declspec(dllexport)
	#error not implemented
#elif OS_LINUX
	#define shared_function C_LINKAGE
#else
	#error OS not supported.
#endif

////////////////////////////////////////////////////////////////////////
// Compiler

#if COMPILER_CLANG
	#define NO_DEBUG __attribute__((nodebug))
	#define INLINE   inline __attribute__((always_inline))
	#define NO_ASAN  __attribute__((no_sanitize("address")))
	#define read_only __attribute__((section(".rodata")))
	#define Packed __attribute__((packed))
#else
	#error Compiler not supported.
#endif

#if BUILD_DEBUG
	#define DebugDo(...) __VA_ARGS__
#else
	#define DebugDo(...)
#endif

#define C_LINKAGE_BEGIN extern "C"{
#define C_LINKAGE_END }
#define C_LINKAGE extern "C"

////////////////////////////////////////////////////////////////////////
// Address Sanitizer

#if ASAN_ENABLED
	C_LINKAGE void __asan_poison_memory_region(void const volatile* addr, u64 size);
	C_LINKAGE void __asan_unpoison_memory_region(void const volatile* addr, u64 size);
	#define AsanPoisonMemRegion(addr, size)   __asan_poison_memory_region((addr), (size))
	#define AsanUnpoisonMemRegion(addr, size) __asan_unpoison_memory_region((addr), (size))
#else
	#define AsanPoisonMemRegion(addr, size)   ((void)(addr), (void)(size))
	#define AsanUnpoisonMemRegion(addr, size) ((void)(addr), (void)(size))
#endif

////////////////////////////////////////////////////////////////////////
// Basic

#define null 0
#define NoFlags 0
#define var auto

#define intern static
#define global_var static
#define local_persist  static
#define GlobalVar

#define U8_MAX  0xFF
#define U16_MAX 0xFFFF
#define U32_MAX 0xFFFFFFFF
#define U64_MAX 0xFFFFFFFFFFFFFFFF
#define PAGE_SIZE 4096

template<typename T> void Swap(T& a, T& b) {
	T temp = a;
	a = b;
	b = temp;
}

#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)
#define GB(x) ((x) << 30)
#define TB(x) ((x) << 40)
#define Thousand(x) ((x) * 1000)
#define Million(x)  ((x) * 1000000)
#define Billion(x)  ((x) * 1000000000)
NO_DEBUG f32 BytesToKB(u64 x);
NO_DEBUG f32 BytesToMB(u64 x);
NO_DEBUG f32 BytesToGB(u64 x);

////////////////////////////////////////////////////////////////////////
// Memory

#define OffsetOf(T,m)                (u64)(&((T*)0)->m)
#define ContainerOf(ptr,T,m)         (T*)(((u8*)(ptr) - OffsetOf(T,m)))
#define OffsetAs(ptr,T,off)          (T*)(Offset((ptr), (off)))
#define OffsetStruct(ptr,T)          (T*)(Offset((ptr), sizeof(T)))
#define OffsetArray(ptr,T,c)         (T*)(Offset((ptr), sizeof(T) * (c)))
#define OffsetBackAs(ptr,T,off)      (T*)(OffsetBack((ptr), (off)))
#define OffsetBackStruct(ptr,T)      (T*)(OffsetBack((ptr), sizeof(T)))
#define OffsetBackArray(ptr,T,c)     (T*)(OffsetBack((ptr), sizeof(T) * (c)))

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
u8* Offset(void* x, u64 a);
u8* OffsetBack(void* x, u64 a);
u64 ptr_diff(void* a, void* b);
b32 ptr_match(void* a, void* b);

////////////////////////////////////////////////////////////////////////
// Bits

u32 clz(u64 v);
u32 clz_u32(u32 v);
u32 ctz(u64 v);
u32 ctz_u32(u32 v);
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
#define Max3(a, b, c)          (Max(Max((a),(b)), (c)))
#define Min3(a, b, c)          (Min(Min((a),(b)), (c)))
#define ClampTop(x, a)         (Min((x), (a)))
#define ClampBot(x, a)         (Max((x), (a)))
#define Clamp(a, x, b)         Min(Max(a, x), b)
#define Clamp01(x)             Clamp(0, (x), 1)
#define Clamp11(x)             Clamp(-1, (x), 1)
#define ReverseClamp(a, x, b)  (((x) < (a)) ? (b) : ((b) < (x)) ? (a) : (x))
#define Square(x)              ((x) * (x))
#define Cube(x)                ((x) * (x) * (x))
#define Sign(x)                (((x) < 0) ? -1 : ((x) > 0) ? 1 : 0)
#define Abs(x)                 (((x) < 0) ? -(x) : (x))
#define InRange(a, x, b)							((a) <= (x) && (x) < (b))

u64 mod_pow2(u64 x, u64 b);
u64 div_pow2(u64 x, u64 b);
u64 div_ceil(u64 x, u64 b);
u64 round_up(u64 x, u64 a);
u64 round_down(u64 x, u64 a);
u64 compose_64(u32 a, u32 b);
u32 next_pow2(u32 v);
u32 prev_pow2(u32 n);

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
#define Scope(...)     ({__VA_ARGS__})
#define _Def(val, def)    								(((val) == 0) ? (def) : (val))
#define _DefSet(val, def) 								if(val == 0) val = def
#define _DefIfSet(val, expr, def) if(expr) val = def

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
#define LoopIter(it, begin) 						for(var it = begin; it; ++it)

////////////////////////////////////////////////////////////////////////
// Asserts

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
#else
	#define InvalidPath Unreachable
	#define Assert(x)
	#define AssertMsg(x, message, ...)
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

#define atomic_inc(x)      __atomic_fetch_add((x), 1, AtomicSeqCst)
#define atomic_dec(x)      __atomic_fetch_sub((x), 1, AtomicSeqCst)
#define atomic_add(x, v)   __atomic_fetch_add((x), (v), AtomicSeqCst)
#define atomic_sub(x, v)   __atomic_fetch_sub((x), (v), AtomicSeqCst)
#define atomic_load(x)     __atomic_load_n((x), AtomicSeqCst)
#define atomic_store(x, v) __atomic_store_n((x), (v), AtomicSeqCst)
#define atomic_or(x, v)    __atomic_fetch_or((x), (v), AtomicSeqCst)
#define atomic_and(x, v)   __atomic_fetch_and((x), (v), AtomicSeqCst)
#define atomic_nand(x, v)  __atomic_fetch_nand((x), (v), AtomicSeqCst)
#define atomic_xor(x, v)   __atomic_fetch_xor((x), (v), AtomicSeqCst)
#define atomic_swap(x, v)  __atomic_exchange_n((x), (v), AtomicSeqCst)
#define atomic_cmp_swap(x, expect, v) __atomic_compare_exchange_n((x), (expect), (v), 0, (AtomicSeqCst), (AtomicSeqCst))

inline b32 atomic_cmp_set(u32* x, u32 expect, u32 v) { return atomic_cmp_swap(x, &expect, v); }
inline u32 atomic_cmp_swap_old(u32* x, u32 expect, u32 v) { u32 res = expect; atomic_cmp_swap(x, &res, v); return res; }

////////////////////////////////////////////////////////////////////////
// Doulby Linked List
#define DLL_push_back(first, last, n, next, prev)     \
	((n)->prev = (last),                                \
	 (n)->next = null,                                  \
	 ((last) ? ((last)->next = (n)) : ((first) = (n))), \
	 (last) = (n))

#define DLL_push_front(first, last, n, next, prev)     \
	((n)->next = (first),                                \
	 (n)->prev = null,                                   \
	 ((first) ? ((first)->prev = (n)) : ((last) = (n))), \
	 (first) = (n))

#define DLL_remove(first, last, n, next, prev) \
	(((n)->prev ? ((n)->prev->next = (n)->next)  \
													: ((first) = (n)->next)),        \
		((n)->next ? ((n)->next->prev = (n)->prev)  \
													: ((last) = (n)->prev)))

#define DLL_pop_back(first, last, n, next, prev)  \
	(((n) = (last)),                                \
			((n) ? (                                      \
					((last) = (n)->prev),                       \
					((last) ? ((last)->next = null)             \
													: ((first) = null))                 \
			) : 0))

#define DLL_pop_front(first, last, n, next, prev) \
	(((n) = (first)),                               \
			((n) ? (                                      \
					((first) = (n)->next),                      \
					((first) ? ((first)->prev = null)           \
														: ((last) = null))                 \
			) : 0))

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
#define dll_push_front(first, last, n) DLL_push_front(first, last, n, next, prev)
#define dll_remove(first, last, n)     DLL_remove(first, last, n, next, prev)
#define dll_pop_back(first, last, n)   DLL_pop_back(first, last, n, next, prev)
#define dll_pop_front(first, last, n)  DLL_pop_front(first, last, n, next, prev)

#define dll_list_push_back(list, n)  DLL_push_back((list)->first, (list)->last, n, next, prev)
#define dll_list_push_front(list, n) DLL_push_front((list)->first, (list)->last, n, next, prev)
#define dll_list_remove(list, n)     DLL_remove((list)->first, (list)->last, n, next, prev)
#define dll_list_pop_back(list, n)   DLL_pop_back((list)->first, (list)->last, n, next, prev)
#define dll_list_pop_front(list, n)  DLL_pop_front((list)->first, (list)->last, n, next, prev)

#define sll_stack_push(first, n)       SLL_stack_push(first, n, next)
#define sll_stack_pop(first)           SLL_stack_pop(first, next)
#define sll_queue_push(first, last, n) SLL_queue_push(first, last, n, next)
#define sll_queue_pop(first, lsat, n)  SLL_queue_pop(first, last, next)

#define sll_list_queue_push(list, n) SLL_queue_push(list.first, list.last, n, next)
#define sll_list_queue_pop(list)     SLL_queue_pop(list.first, list.last, next)

////////////////////////////////////////////////////////////////////////
// Indexed Doubly Linked List
#define IDLL_push_back(arr, first, last, n, next, prev)   \
		((arr)[n].prev = (last),                                \
			(arr)[n].next = 0,                                     \
			((last) ? ((arr)[last].next = (n)) : ((first) = (n))), \
			(last) = (n))

#define IDLL_push_front(arr, first, last, n, next, prev)   \
		((arr)[n].next = (first),                                \
			(arr)[n].prev = 0,                                      \
			((first) ? ((arr)[first].prev = (n)) : ((last) = (n))), \
			(first) = (n))

#define IDLL_remove(arr, first, last, n, next, prev)            \
		(((arr)[n].prev ? ((arr)[(arr)[n].prev].next = (arr)[n].next) \
																		: ((first) = (arr)[n].next)),                 \
			((arr)[n].next ? ((arr)[(arr)[n].next].prev = (arr)[n].prev) \
																		: ((last) = (arr)[n].prev)))

#define IDLL_pop_back(arr, first, last, n, next, prev)  \
		(((n) = (last)),                                      \
				((n) ? (                                            \
						((last) = (arr)[n].prev),                         \
						((last) ? ((arr)[last].next = 0) : ((first) = 0)) \
				) : 0))

#define IDLL_pop_front(arr, first, last, n, next, prev)  \
		(((n) = (first)),                                      \
				((n) ? (                                             \
						((first) = (arr)[n].next),                         \
						((first) ? ((arr)[first].prev = 0) : ((last) = 0)) \
				) : 0))

////////////////////////////////////////////////////////////////////////
// Indexed Singly Linked List
#define ISLL_stack_push(arr, first, n, next) \
		((arr)[n].next = (first), (first) = (n))

#define ISLL_stack_pop(arr, first, next) \
		((first) = (arr)[first].next)

#define ISLL_queue_push(arr, first, last, n, next)        \
		((arr)[n].next = 0,                                     \
			((last) ? ((arr)[last].next = (n)) : ((first) = (n))), \
			(last) = (n))

#define ISLL_queue_pop(arr, first, last, next) \
		(((first) == (last)) ? ((first) = (last) = 0)      \
																							: ((first) = (arr)[first].next))

#define idll_push_back(arr, first, last, n)  IDLL_push_back(arr, first, last, n, next, prev)
#define idll_push_front(arr, first, last, n) IDLL_push_front(arr, first, last, n, next, prev)
#define idll_remove(arr, first, last, n)     IDLL_remove(arr, first, last, n, next, prev)
#define idll_pop_back(arr, first, last, n)   IDLL_pop_back(arr, first, last, n, next, prev)
#define idll_pop_front(arr, first, last, n)  IDLL_pop_front(arr, first, last, n, next, prev)

#define idll_list_push_back(arr, list, n)  IDLL_push_back(arr, list.first, list.last, n, next, prev)
#define idll_list_push_front(arr, list, n) IDLL_push_front(arr, list.first, list.last, n, next, prev)
#define idll_list_remove(arr, list, n)     IDLL_remove(arr, list.first, list.last, n, next, prev)
#define idll_list_pop_back(arr, list, n)   IDLL_pop_back(arr, list.first, list.last, n, next, prev)
#define idll_list_pop_front(arr, list, n)  IDLL_pop_front(arr, list.first, list.last, n, next, prev)

#define isll_stack_push(arr, first, n)       ISLL_stack_push(arr, first, n, next)
#define isll_stack_pop(arr, first)           ISLL_stack_pop(arr, first, next)
#define isll_queue_push(arr, first, last, n) ISLL_queue_push(arr, first, last, n, next)
#define isll_queue_pop(arr, first, last)     ISLL_queue_pop(arr, first, last, next)

#define isll_list_queue_push(arr, list, n)   ISLL_queue_push(arr, list.first, list.last, n, next)
#define isll_list_queue_pop(arr, list)       ISLL_queue_pop(arr, list.first, list.last, next)

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

#define hdll_list_push_back(arr, list, n)  HDLL_push_back(arr, list.first, list.last, n, next, prev)
#define hdll_list_remove(arr, list, n)     HDLL_remove(arr, list.first, list.last, n, next, prev)

////////////////////////////////////////////////////////////////////////
// Defer
template<typename F> struct _Defer {
	F f;
	~_Defer() { f(); }
};
#define defer(code) auto Glue(_defer_, __LINE__) = _Defer([&](){ code; })
#define DeferLoop(begin, end) for(int _i_ = ((begin), 1); _i_; _i_ = 0, (end))
#define DeferLoopIf(begin, end) for(int _i_ = (begin); _i_; _i_ = false, (end))

template<typename T> struct ResultOk {
	T value;
	b32 ok;
};
template<typename T, typename Err = b32> struct ResultErr {
	T value;
	Err err;
};

///////////////////////////////////
// or_else
#define OrElse(value, ok, expr, fallback)   \
	Scope(                                     \
		var _temp = (expr);                       \
		_temp.ok ? _temp.value : Scope(fallback); \
	)
#define or_else(expr, fallback) OrElse(value, ok, expr, fallback)
#define OrElseErr(value, err, expr, fallback) \
	Scope(                                       \
		var _temp = (expr);                         \
		!_temp.err ? _temp.value : Scope(fallback); \
	)
#define or_else_err(expr, fallback) OrElseErr(value, err, expr, fallback)

///////////////////////////////////
// or_return
#define OrReturn(value, ok, expr) \
	Scope(                           \
		var _temp = (expr);             \
		if(!_temp.ok) return {};       \
		_temp.value;                    \
	)
#define or_return(expr) OrReturn(value, ok, expr)
#define OrReturnErr(value, err, expr)        \
	Scope(                                      \
		var _temp = (expr);                        \
		if(!_temp.err) return {.err = _temp.err}; \
		_temp.value;                               \
	)
#define or_return_err(expr) OrReturn(value, err, expr)

///////////////////////////////////
// or_continue
#define OrContinue(value, ok, expr) \
	Scope(                             \
		var _temp = (expr);               \
		if(!_temp.ok) continue;          \
		_temp.value;                      \
	)
#define or_continue(expr) OrContinue(value, ok, expr)
#define OrContinueErr(value, err, expr) \
	Scope(                                 \
		var _temp = (expr);                   \
		if(_temp.err) continue; _temp.value; \
	)
#define or_continue_err(expr) OrContinueErr(value, err, expr)

///////////////////////////////////
// or_break
#define OrBreak(value, ok, expr)     \
	Scope(                              \
		var _temp = (expr);                \
		if(!_temp.ok) break; _temp.value; \
	)
#define or_break(expr) OrBreak(value, ok, expr)
#define OrBreakErr(value, err, expr) \
	Scope(                              \
		var _temp = (expr);                \
		if(_temp.err) break; _temp.value; \
	)
#define or_break_err(expr) OrBreakErr(value, err, expr)

typedef u32 Futex;
struct Mutex { Futex futex; };
struct CondVar { Futex futex; };
struct Semaphore { Futex futex; };

const u32 DEFAULT_CAPACITY = 8;
const u32 DEFAULT_RESIZE_FACTOR = 2;

#define Introspect

struct BitArrayD {
	u64* words;
	u64 bit_count;
};

void bit_array_set(BitArrayD& bits, u64 idx);
void bit_array_clear(BitArrayD& bits, u64 idx);
b32 bit_array_get(BitArrayD& bits, u64 idx);
u64 bit_array_word_count(BitArrayD& bits);

template<i32 N> struct BitArray {
	u64 words[N/64];
	static constexpr u64 bit_count = N * 64;
};

template<i32 N> void bit_array_set(BitArray<N>& bits, u64 idx)   { bits.words[idx >> 6] |=   u64(1)<<(idx & 63); }
template<i32 N> void bit_array_clear(BitArray<N>& bits, u64 idx) { bits.words[idx >> 6] &= ~(u64(1)<<(idx & 63)); }
template<i32 N> b32 bit_array_get(BitArray<N>& bits, u64 idx)    { return (bits.words[idx >> 6] >> (idx & 63))&1; }
template<i32 N> u64 bit_array_word_count(BitArray<N>& bits)      { return (bits.bit_count + 63) / 64; }

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

////////////////////////////////////////////////////////////////////////
// RandyGaul stackless coroutine

#define CoroutineMaxDepth 8
#define CoroutineCaseOffset (1024 * 1024)
#define CoroutineStackSize (512)

struct Coroutine {
	f32 elapsed;
	u32 flag;
	u32 index;
	u32 line[CoroutineMaxDepth];
	u32 stack_pointer;
	u8 stack[CoroutineStackSize];
};

u8* Restrict _coroutine_var(Coroutine* co, u32 size);
#define co_var(co, T) *(T*)_coroutine_var(co, sizeof(T))

#define co_begin(co)          do { co->flag = 0; switch(co->line[co->index]) { default: 
#define co_case(co, name)     case __LINE__: name: co->line[co->index] = __LINE__;
#define co_wait(co, time, dt) do { case __LINE__: co->line[co->index] = __LINE__; co->elapsed += dt; do { if(co->elapsed < time) { co->flag = 1; goto __co_end; } else { co->elapsed = 0; } } while(0); } while(0)
#define co_exit(co)           do { co->flag = 1; goto __co_end; } while(0)
#define co_yield(co)          do { co->line[co->index] = __LINE__; co_exit(co); case __LINE__:; } while(0)
#define co_call(co, ...)      co->flag = 0; case __LINE__: Assert(co->index < CoroutineMaxDepth); co->line[co->index++] = __LINE__; __VA_ARGS__; co->index--; do { if(co->flag) { goto __co_end; } else { case __LINE__ + CoroutineCaseOffset: co->line[co->index] = __LINE__ + CoroutineCaseOffset; } } while(0)
#define co_end(co)            } co->line[co->index] = 0; __co_end:; co->stack_pointer = 0; } while(0)

////////////////////////////////////////////////////////////////////////
// Simd

// #include <smmintrin.h>

void cpu_relax();

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

#define SimdShuffle(a, b, c, d) (((d) << 6) | ((c) << 4) | ((b) << 2) | (a))
#define simd_shuffle(a, b, imm) f32x4(_mm_shuffle_ps(a.p, b.p, imm))

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

u64 cpu_now();
void cpu_find_frequency();

extern f32 time_dt;
extern f32 time_now;
extern u32 current_frame;
extern u64 cpu_frequency;

// #define X(a, ...) X_IMPL(a, ##__VA_ARGS__, 3)
// #define X_IMPL(a, value, ...) x_foo(a, value)

// #define X_1(a) x_foo(a, 3)
// #define X_2(a, value) x_foo(a, value)
// #define X_SELECT(_1, _2, NAME, ...) NAME
// #define X(...) X_SELECT(__VA_ARGS__, X_2, X_1)(__VA_ARGS__)

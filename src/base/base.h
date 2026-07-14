#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <initializer_list>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t	i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

typedef u8  b8;
typedef i32 b32;

typedef u64 DenseTime;
typedef va_list VaList;

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
#define global static
#define local  static
#define Extern

const u64 U8_MAX  = 0xFF;
const u64 U16_MAX = 0xFFFF;
const u64 U32_MAX = 0xFFFFFFFF;
const u64 U64_MAX = 0xFFFFFFFFFFFFFFFF;
const u64 INVALID_ID = U32_MAX;
const u64 PAGE_SIZE = 4096;

template<typename T> void Swap(T& a, T& b) {
  T temp = a;
  a = b;
  b = temp;
}
template<typename T> b32 equal(T a, T b) { return a == b; }

NO_DEBUG constexpr u64 KB(u64 x) { return x << 10; }
NO_DEBUG constexpr u64 MB(u64 x) { return x << 20; }
NO_DEBUG constexpr u64 GB(u64 x) { return x << 30; }
NO_DEBUG constexpr u64 TB(u64 x) { return x << 40; }
NO_DEBUG constexpr u64 Thousand(u64 x) { return x * 1000; }
NO_DEBUG constexpr u64 Million(u64 x)  { return x * 1000000; }
NO_DEBUG constexpr u64 Billion(u64 x)  { return x * 1000000000; }
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

template<typename T> void MemZeroStruct(T* x)                { MemZero(x, sizeof(*x)); };
template<typename T> void MemZeroArray(T* x, u64 c)          { MemZero(x, sizeof(*x) * c); };
template<typename T> void MemCopyStruct(T* d, T* s)          { MemCopy(d, s, sizeof(*d)); }
template<typename T> void MemCopyArray(T* d, T* s, u64 c)    { MemCopy(d, s, sizeof(*d) * c); }
template<typename T> b32  MemMatchStruct(T* a, T* b)         { return MemMatch(a, b, sizeof(*a)); }
template<typename T> b32  MemMatchArray(T* a, T* b, u64 c)   { return MemMatch(a, b, sizeof(*a) * c); }

u64 AlignUp(u64 x, u64 a);
u64 AlignDown(u64 x, u64 a);
u64 AlignPadUp(u64 x, u64 a);
u64 AlignPadDown(u64 x, u64 a);
b32 IsAligned(u64 x, u64 a);
u8* PtrAlignUp(void* x, u64 a);
u8* PtrAlignDown(void* x, u64 a);
u8* PtrAlignPadUp(void* x, u64 a);
u8* PtrAlignPadDown(void* x, u64 a);
b32 PtrIsAligned(void* x, u64 a);
b32 IsPow2(u64 x);
u8* Offset(void* x, u64 a);
u8* OffsetBack(void* x, u64 a);
u64 PtrDiff(void* a, void* b);
b32 PtrMatch(void* a, void* b);

////////////////////////////////////////////////////////////////////////
// Bits

u32 clz(u64 val);
u32 ctz(u64 val);
u32 count_bits_set(u64 val);
u32 most_significant_bitu32(u32 size);
u64 most_significant_bitu64(u64 size);

#define Bit(x) (1 << (x))
b32 BitHas(u64 x, u64 pos);
u64 FlagSet(u64 x, u64 f);
u64 FlagClear(u64 x, u64 f);
u64 FlagToggle(u64 x, u64 f);
b32 FlagHas(u64 x, u64 f);
b32 FlagEquals(u64 x, u64 f);
b32 FlagIntersects(u64 x, u64 f);
b32 FlagIsSubset(u64 x, u64 f);

////////////////////////////////////////////////////////////////////////
// Common operations

#define Max(a, b)              ((a) > (b) ? (a) : (b))
#define Min(a, b)              ((a) < (b) ? (a) : (b))
#define Max3(a, b, c)          (Max(Max((a),(b)), (c)))
#define Min3(a, b, c)          (Min(Min((a),(b)), (c)))
#define ClampTop(x, a)         (Min((x), (a)))
#define ClampBot(x, a)         (Max((x), (a)))
#define Clamp(a, x, b)         (((x) < (a)) ? (a) : ((x) > (b)) ? (b) : (x))
#define Clamp01(x)             Clamp(0, (x), 1)
#define Clamp11(x)             Clamp(-1, (x), 1)
#define ReverseClamp(a, x, b)  (((x) < (a)) ? (b) : ((b) < (x)) ? (a) : (x))
#define Square(x)              ((x) * (x))
#define Cube(x)                ((x) * (x) * (x))
#define Sign(x)                (((x) < 0) ? -1 : ((x) > 0) ? 1 : 0)
#define Abs(x)                 (((x) < 0) ? -(x) : (x))

u64 ModPow2(u64 x, u64 b);
u64 DivPow2(u64 x, u64 b);
u64 CeilIntDiv(u64 x, u64 b);
u64 RoundUp(u64 x, u64 a);
u64 RoundDown(u64 x, u64 a);
u64 Compose64Bit(u64 a, u64 b);
u32 next_pow2(u32 v);
u32 prev_pow2(u32 n);

#define is_finite_f32(x) __builtin_isfinite((x))
#define is_nan_f32(x)    __builtin_isnan((x))
#define is_inf_f32(x)    __builtin_isinf((x))

////////////////////////////////////////////////////////////////////////
// Shenanigans

#define ArrayCount(x)   (sizeof((x)) / sizeof((x)[0]))
#define ArrayRand(arr)  arr[rand_rng_u32(0, ArrayCount(arr)-1)]
#define ArrayZero(arr)  MemZeroArray((arr), ArrayCount((arr)))
#define ArrayCopy(d, s) MemCopyArray((d), (s), ArrayCount((d)))
#define Assign(a,b)    (*((u8**)(&(a))) = (u8*)(b))
#define _Stringify(S)  #S
#define Stringify(S)   _Stringify(S)
#define _Glue(A,B)     A##B
#define Glue(A,B)      _Glue(A,B)
#define Transmute(T, x) (*(T*)&(x))

#define Loop(it, c)                      for (i32 it = 0; it < c; ++it)
#define LoopReverse(it, count)           for (i32 it = (count) - 1; it >= 0; --it)
#define LoopOff(it, li, hi)              for (i32 it = (li); it < (hi); ++it)
#define LoopElement(it, array)           for (i32 it = 0; it < ArrayCount(array); ++it)
#define LoopEnum(it, type)               for (type it = (type)0; it < type##_COUNT; it = (type)(it+1))
#define LoopEnumNonZero(it, type)        for (type it = (type)1; it < type##_COUNT; it = (type)(it+1))
#define LoopRange(it, range)             for (i32 it = (range).min; it < (range).max; ++it)
#define LoopNode(it, first)              for (var* it = first; it != 0; it = it->next)
#define LoopNodeReverse(it, last)        for (var* it = last; it != 0; it = it->prev)
#define LoopINode(it, first, arr)        for (u32 it = first; it != 0; it = (arr)[it].next)
#define LoopINodeReverse(it, last, arr)  for (u32 it = last; it != 0; it = (arr)[it].prev)
#define LoopHNode(it, first, arr)        for (var it = first; it.idx != 0; it = (arr)[it.idx].elem.next)

////////////////////////////////////////////////////////////////////////
// Asserts

void Trap();
void DebugTrap();

#define InvalidPath         Assert(!"Invalid Path!")
#define InvalidDefaultCase  default: {InvalidPath;}
#define NotImplemented      Assert(!"Not Implemented!")
#define AssertAlways(x)     if (!(x)) { Trap(); }
#define UnusedVariable(x)   (void)x

#if BUILD_DEBUG
  #define Assert(x) ((x) ? (void)0 : DebugTrap())
  #define AssertMsg(x, message, ...) if (!(x)) { _log_output(LogLevel_Error, message, ##__VA_ARGS__); DebugTrap(); }
#else
  #define Assert(x)
  #define AssertMsg(x, message, ...)
#endif

////////////////////////////////////////////////////////////////////////
// Atomic Operations

#define AtomicSeqCst __ATOMIC_SEQ_CST
#define AtomicRelaxed __ATOMIC_RELAXED
#define AtomicAcquire __ATOMIC_ACQUIRE
#define AtomicRelease __ATOMIC_RELEASE

#define atomic_inc(x)                     __atomic_fetch_add((x), 1, AtomicSeqCst)
#define atomic_dec(x)                     __atomic_fetch_sub((x), 1, AtomicSeqCst)
#define atomic_add(x, v)                  __atomic_fetch_add((x), (v), AtomicSeqCst)
#define atomic_sub(x, v)                  __atomic_fetch_sub((x), (v), AtomicSeqCst)
#define atomic_load(x)                    __atomic_load_n((x), AtomicSeqCst)
#define atomic_store(x, v)                __atomic_store_n((x), (v), AtomicSeqCst)
#define atomic_or(x, v)                   __atomic_fetch_or((x), (v), AtomicSeqCst)
#define atomic_and(x, v)                  __atomic_fetch_and((x), (v), AtomicSeqCst)
#define atomic_xor(x, v)                  __atomic_fetch_xor((x), (v), AtomicSeqCst)
#define atomic_exchange(x, v)             __atomic_exchange_n((x), (v), AtomicSeqCst)
#define atomic_cmp_exchange(x, expect, v) __atomic_compare_exchange_n((x), (expect), (new), 0, AtomicSeqCst, AtomicSeqCst)
#define atomic_cond_exchange(x, v, c) ({ u32 _new = (c); __atomic_compare_exchange_n((x), (&_new), (v), 0, AtomicSeqCst, AtomicSeqCst); _new; })

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
              : ((first) = null)),                \
      DebugDo((n)->next = null, (n)->prev = null) \
    ) : 0))

#define DLL_pop_front(first, last, n, next, prev) \
  (((n) = (first)),                               \
    ((n) ? (                                      \
      ((first) = (n)->next),                      \
      ((first) ? ((first)->prev = null)           \
               : ((last) = null)),                \
      DebugDo((n)->next = null, (n)->prev = null) \
    ) : 0))

////////////////////////////////////////////////////////////////////////
// Singly Linked List
#define SLL_stack_push(first, n, next) \
  ((n)->next = (first), (first) = (n))

#define SLL_stack_pop(first, next) \
  ((first) = (first)->next)

#define SLL_queue_push(first, last, n, next)          \
  ((n)->next = null,                                  \
   ((last) ? ((last)->next = (n)) : ((first) = (n))), \
   (last) = (n))

#define SLL_queue_pop(first, last, next)           \
  (((first) == (last)) ? ((first) = (last) = null) \
                       : ((first) = (first)->next))

#define dll_push_back(first, last, n)  DLL_push_back(first, last, n, next, prev)
#define dll_push_front(first, last, n) DLL_push_front(first, last, n, next, prev)
#define dll_remove(first, last, n)     DLL_remove(first, last, n, next, prev)
#define dll_pop_back(first, last, n)   DLL_pop_back(first, last, n, next, prev)
#define dll_pop_front(first, last, n)  DLL_pop_front(first, last, n, next, prev)

#define dll_list_push_back(list, n)  DLL_push_back(list.first, list.last, n, next, prev)
#define dll_list_push_front(list, n) DLL_push_front(list.first, list.last, n, next, prev)
#define dll_list_remove(list, n)     DLL_remove(list.first, list.last, n, next, prev)
#define dll_list_pop_back(list, n)   DLL_pop_back(list.first, list.last, n, next, prev)
#define dll_list_pop_front(list, n)  DLL_pop_front(list.first, list.last, n, next, prev)

#define sll_stack_push(first, n)       SLL_stack_push(first, n, next)
#define sll_stack_pop(first)           SLL_stack_pop(first, next)
#define sll_queue_push(first, last, n) SLL_queue_push(first, last, n, next)
#define sll_queue_pop(first, lsat, n)  SLL_queue_pop(first, last, next)

#define sll_list_queue_push(list, n) SLL_queue_push(list.first, list.last, n, next)
#define sll_list_queue_pop(list)     SLL_queue_pop(list.first, list.last, next)

////////////////////////////////////////////////////////////////////////
// Indexed Doubly Linked List
// index = 0 is null
#define IDLL_push_back(arr, first, last, n, next, prev)   \
  ((arr)[n].prev = (last),                                \
   (arr)[n].next = 0,                                     \
   ((last) ? ((arr)[last].next = (n)) : ((first) = (n))), \
   (last) = (n))

#define IDLL_push_front(arr, first, last, n, next, prev) \
  ((arr)[n].next = (first),                                    \
   (arr)[n].prev = 0,                                          \
   ((first) ? ((arr)[first].prev = (n)) : ((last) = (n))),     \
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
  ((arr)[n.idx].elem.prev = (last),                                    \
   (arr)[n.idx].elem.next = {},                                        \
   ((last.idx) ? ((arr)[last.idx].elem.next = (n)) : ((first) = (n))), \
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

template<typename F>
struct _Defer {
  F f;
  ~_Defer() { f(); }
};
#define defer(code) auto Glue(_defer_, __LINE__) = _Defer([&](){ code; })
#define DeferLoop(begin, end) for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define DeferLoopIf(begin, end) for (int _i_ = (begin); _i_; _i_ = false, (end))

////////////////////////////////////////////////////////////////////////
// Error handling

template <typename E = b32>
struct _Unexpected {
  E e;
};

template <typename E = b32>
_Unexpected<E> Err(E err = {}) {
  return {err};
}

template<typename T, typename E = b32>
struct Result {
  union {
    T v;
    E e;
  };
  b32 err;
  Result() = default;
  Result(T val) : v(val), err(false) {}
  Result(_Unexpected<E> err_) {
    v = {};
    e = err_.e;
    err = true;
  }
  template<typename U> Result(_Unexpected<U> u) : e(E(u.e)), err(true) {}
  operator T() const { return v; }
  T& operator*()     { return v; }
  T* operator->()    { return &v; }
  E  error()         { Assert(!err); return e; }
};

#define Try(expr)                       \
  ({                                    \
    var _r = (expr);                    \
    if (_r.err) return Err(_r.error()); \
    *_r;                                \
  })

////////////////////////////////////////////////////////////////////////
// Types

const u32 DEFAULT_CAPACITY = 8;
const u32 DEFAULT_RESIZE_FACTOR = 2;

#define Introspect

#define MakeId(T) \
  struct T {      \
    u32 idx;      \
    u32 gen;      \
  };

struct ArrayBit {
  u64* words;
  u64 bit_count;
};

void array_bit_set(ArrayBit* bits, u64 idx);
void array_bit_clear(ArrayBit* bits, u64 idx);
b32 array_bit_get(ArrayBit* bits, u64 idx);

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
#define ring_write_struct(ring, ptr) ring_write((ring), (ptr), sizeof(*(ptr)))
#define ring_read_struct(ring, ptr) ring_read((ring), (ptr), sizeof(*(ptr)))
#define ring_write_array(ring, ptr, c) ring_write((ring), (ptr), (c) * sizeof(*(ptr)))
#define ring_read_array(ring, ptr, c) ring_read((ring), (ptr), (c) * sizeof(*(ptr)))

u64 ring_write_nowrap(RingBuffer& ring, void* src, u64 src_size, u64 align = 0);
u64 ring_read_nowrap(RingBuffer& ring, void* dst, u64 read_size);
#define ring_write_nowrap_struct(ring, ptr) ring_write_nowrap((ring), (ptr), sizeof(*(ptr)), alingof(*(ptr)))
#define ring_read_nowrap_struct(ring, ptr) ring_read_nowrap((ring), (ptr), sizeof(*(ptr)))
#define ring_write_nowrap_array(ring, ptr, c) ring_write_nowrap((ring), (ptr), (c) * sizeof(*(ptr)), alignof(*(ptr)))
#define ring_read_nowrap_array(ring, ptr, c) ring_read_nowrap((ring), (ptr), (c) * sizeof(*(ptr)))

template<typename T>
struct Slice {
  T* data;
  union {
    u64 count;
    u64 size;
  };
  T& operator[](u64 idx) {
    Assert(idx < count);
    return data[idx];
  }
  Slice(T* data_, u64 count_) { data = data_; count = count_; }
  Slice() = default;
};
template<typename T> Slice<T> slice(Slice<T> a, u64 li, u64 hi)      { Assert(li <= hi && hi <= a.count); return Slice(a.data + li, hi - li); }
template<typename T> Slice<T> slice_n(Slice<T> a, u64 off, u64 size) { Assert(off+size <= a.count); return Slice(a.data + off, size); }
template<typename T> Slice<T> slice_prefix(Slice<T> a, u64 n)        { Assert(n <= a.count); return Slice(a.data, n); }
template<typename T> Slice<T> slice_postfix(Slice<T> a, u64 n)       { Assert(n <= a.count); return Slice(a.data + (a.count - n), n); }
template<typename T> Slice<T> slice_skip(Slice<T> a, u64 n)          { Assert(n <= a.count); return Slice(a.data + n, a.count - n); }
template<typename T> Slice<T> slice_chop(Slice<T> a, u64 n)          { Assert(n <= a.count); return Slice(a.data, a.count - n); }
template<typename T, u64 N> Slice<T> slice(T (&a)[N])                { return Slice(a, N); }
template<typename T> Slice<u8> slice_to_bytes(Slice<T> s)            { return Slice((u8*)s.data, s.count * sizeof(T)); }
template<typename T> Slice<u8> slice_struct_to_bytes(T* s)           { return Slice((u8*)s, sizeof(T)); }
template<typename T> u64 slice_size(Slice<T> s)                      { return s.count * sizeof(T); }
template<typename To, typename From> Slice<To> slice_reinterpret(Slice<From> s) {
  Assert((s.count * sizeof(From)) % sizeof(To) == 0);
  return Slice((To*)s.data, (s.count*sizeof(From)) / sizeof(To));
}

struct String {
  u8* str;
  u64 size;
  String() = default;
  NO_DEBUG String(const char* str_);
};

struct String64 {
  u8 str[64];
  u32 size;
  operator String();
};

struct HotReloadData {
  void* ctx;
  String lib;
};




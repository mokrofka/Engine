#pragma once
#include "stdint.h"
#include "stdarg.h"
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

#define null 0
#define NoFlags 0
#define var auto

#define Swap(a, b) \
  {                \
    auto temp = a; \
    a = b;         \
    b = temp;      \
  }

#define intern static
#define global static
#define local  static

#define U8_MAX  0xFF
#define U16_MAX 0xFFFF
#define U32_MAX 0xFFFFFFFF
#define U64_MAX 0xFFFFFFFFFFFFFFFF
#define INVALID_ID     U32_MAX
#define INVALID_ID_U16 U16_MAX
#define INVALID_ID_U8  U8_MAX

#define KB(n)         ((u64)(n) << 10)
#define MB(n)         ((u64)(n) << 20)
#define GB(n)         ((u64)(n) << 30)
#define TB(n)         ((u64)(n) << 40)
#define Thousand(n)   ((n) * 1000)
#define Million(n)    ((n) * 1000000)
#define Billion(n)    ((n) * 1000000000ull)
#define BytesToKB(x)  ((f64)(x) / 1024)
#define BytesToMB(x)  ((f64)BytesToKB(x) / 1024)
#define BytesToGB(x)  ((f64)BytesToMB(x) / 1024)

#define SecToMs(x)  ((x) * Thousand(1))
#define SecToUs(x)  ((x) * Million (1))
#define SecToNs(x)  ((x) * Billion (1))
#define MsToSec(x)  ((f64)(x) / Thousand(1))
#define MsToUs(x)   ((x) * Thousand(1))
#define MsToNs(x)   ((x) * Million (1))
#define UsToSec(x)  ((f64)(x) / Million (1))
#define UsToMs(x)   ((f64)(x) / Thousand(1))
#define UsToNs(x)   ((x) * Thousand(1))
#define NsToSec(x)  ((f64)(x) / Billion (1))
#define NsToMs(x)   ((f64)(x) / Million (1))
#define NsToUs(x)   ((f64)(x) / Thousand(1))

#define OffsetOf(T,m)                (u64)(&((T*)0)->m)
#define MemberFromOffset(T,ptr,off)  (T)(Offset(ptr, off))
#define CastFromMember(T,m,ptr)      (T*)(OffsetBack(ptr, OffsetOf(T,m)))
#define MemberIndexOf(T,mT,m)        (OffsetOf(T,m) / sizeof(mT))

#define MemZero(x,z)          MemSet(x,0,z)
#define MemZeroStruct(x)      MemZero(x,sizeof(*(x)))
#define MemZeroArray(x,c)     MemZero(x,sizeof(*(x)) * (c))
#define MemCopyStruct(d,s)    MemCopy(d,s,sizeof(*(d)))
#define MemCopyArray(d,s,c)   MemCopy(d,s,sizeof(*(d)) * (c))
#define MemMatchStruct(a,b)   MemMatch(a,b,sizeof(*(a)))
#define MemMatchArray(a,b,c)  MemMatch(a,b,sizeof(*(a)) * (c))

#define AlignUp(x,a)       (((x) + (a) - 1) & ~((a) - 1))
#define AlignDown(x,a)     ((x) & ~((a) - 1))
#define AlignPadUp(x,a)    (-(x) & ((a) - 1))
#define AlignPadDown(x,a)  ((x) & ((a) - 1))
#define IsPow2(x)          ((((x) - 1) & (x)) == 0)
#define IsAligned(x,a)     ((((a) - 1) & (x)) == 0)
#define Offset(x,a)        ((u8*)(x) + (a))
#define OffsetBack(x,a)    ((u8*)(x) - (a))
#define MemDiff(x,a)       ((u8*)(x) - (u8*)(a))
#define PtrMatch(a,b)      ((u8*)(a) == (u8*)(b))

#define Min(a,b)               (((a) < (b)) ? (a) : (b))
#define Max(a,b)               (((a) > (b)) ? (a) : (b))
#define Max3(a,b,c)            Max(Max(a,b),c)
#define Min3(a,b,c)            Min(Min(a,b),c)
#define ClampTop(x,a)          Min(x,a)
#define ClampBot(x,a)          Max(x,a)
#define Clamp(a,x,b)           (((x) < (a)) ? (a) : ((x) > (b)) ? (b) : (x))
#define ReverseClamp(a,x,b)    (((x) < (a)) ? (b) : ((b) < (x)) ? (a) : (x))
#define Wrap(a,x,b)            ReverseClamp(a,x,b)
#define Sqr(x)                 ((x)*(x))
#define Cube(x)                ((x)*(x)*(x))
#define Sign(x)                ((x) < 0 ? -1 : (x) > 0 ? 1 : 0)
#define Abs(x)                 ((x) < 0 ? -(x) : (x))
#define IsInsideIncl(a,x,b)    (((a) <= (x)) && ((x) <= (b)))
#define IsInsideExcl(a,x,b)    (((a) < (x)) && ((x) < (b)))
#define IsInsideBound(x,a)     ((0 <= (x)) && ((x) < a))
#define IsInsideBounds(a,x,b)  (((a) <= (x)) && ((x) < (b)))
#define ModPow2(x,b)           ((x) & ((b) - 1))
#define DivPow2(x,b)           ((x) >> ctz(b))
#define CeilIntDiv(x,b)        (((x) + (b) - 1) / (b))
#define RoundUp(x,a)           (CeilIntDiv(x,a) * (a))
#define RoundDown(x,a)         ((x) / (y) * (a))
#define Compose64Bit(a,b)      (((u64)(a) << 32) | (u64)(b))

#define ArrayCount(x)  (sizeof((x)) / sizeof((x)[0]))
#define ElemSize(x)    (sizeof((x)[0]))
#define Assign(a,b)    (*((u8**)(&(a))) = (u8*)(b))
#define As(T)          *(T*)
#define cast(a)        (a)
#define Loop(i, c)     for (i32 i = 0; i < c; ++i)
#define _Stringify(S)  #S
#define Stringify(S)   _Stringify(S)
#define _Glue(A,B)     A##B
#define Glue(A,B)      _Glue(A,B)

#define Bit(x)               (1 << (x))
#define BitHas(x, pos)       ((x) & (1 << (pos)))
#define FlagSet(x, f)        ((x) | (f))
#define FlagClear(x, f)      ((x) & ~(f))
#define FlagToggle(x, f)     ((x) ^ (f))
#define FlagHas(x, f)        (((x) & (f)) == (f))
#define FlagEquals(x, f)     ((x) == (f))
#define FlagIntersects(x,f)  (((x) & (f)) > 0)

#define quick_sort(ptr, count, element_size, cmp_function) qsort((ptr), (count), (element_size), (int (*)(const void *, const void *))(cmp_function))

#define CheckNil(nil,p) ((p) == 0 || (p) == nil)
#define SetNil(nil,p) ((p) = nil)
//- rjf: singly-linked, doubly-headed lists (queues)
#define SLLQueuePush_NZ(nil,f,l,n,next) (CheckNil(nil,f) ?                                 \
                                        ((f)=(l)=(n), SetNil(nil, (n)->next)) :            \
                                        ((l)->next=(n), (l)=(n), SetNil(nil, (n)->next)))

#define SLLQueuePush(f,l,n) SLLQueuePush_NZ(0,f,l,n,next)

#define Local(T, x, fn) \
  local T x; \
  local b32 Glue(is_init, __LINE__); \
  if (!Glue(is_init, __LINE__)) { \
    x = fn; \
    Glue(is_init, __LINE__) = true; \
  }

template<typename F>
struct _Defer {
  F f;
  ~_Defer() { f(); }
};
#define defer(code) auto Glue(_defer_, __LINE__) = _Defer([&](){ code; })
#define DeferLoop(begin, end) for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define IfDeferLoop(begin, end) for (b32 _once = (begin); _once; _once = false, (end))

template <typename T>
struct Ret {
  T v;
  b32 err;
};
template <>
struct Ret<void> {
  b32 err;
};
template <typename T>
inline T _result_return(Ret<T> res) { 
  return res.v; 
}
inline void _result_return(Ret<void> res) {
}
#define Try(expr) ({ auto r = expr; if (r.err) return {.err = r.err}; _result_return(r); })

#if BUILD_DEBUG
  #define DebugDo(...) __VA_ARGS__
#else
  #define DebugDo(...)
#endif

#define C_LINKAGE_BEGIN extern "C"{
#define C_LINKAGE_END }
#define C_LINKAGE extern "C"

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
// Compiler

#define Likely(expr)           Expect(expr,1)
#define Unlikely(expr)         Expect(expr,0)

#if COMPILER_CLANG
  #define NO_DEBUG           __attribute__((nodebug))
  #define INLINE             NO_DEBUG inline __attribute__((always_inline))
  #define DebugBreak()       __builtin_debugtrap()
  #define Expect(expr, val)  __builtin_expect((expr), (val))

  #define MemSet(d,byte,z)      __builtin_memset(d,byte,z)
  #define MemCopy(d,s,z)        __builtin_memcpy(d,s,z)
  #define MemMatch(a,b,z)       (__builtin_memcmp((a), (b), (z)) == 0)

  INLINE u32 count_bits_set(u64 val)   { return __builtin_popcountll(val); }
  INLINE u32 clz(u64 val)              { return __builtin_clzll(val); }
  INLINE u32 ctz(u64 val)              { return __builtin_ctzll(val); }

  #define NO_ASAN __attribute__((no_sanitize("address")))
#else
  #error Compiler not supported.
#endif

////////////////////////////////////////////////////////////////////////
// OS

#define PAGE_SIZE 4096

#if OS_WINDOWS
  #if HOTRELOAD_BUILD
    #define shared_function C_LINKAGE __declspec(dllexport)
    #ifdef KEXPORT
      #define KAPI __declspec(dllexport)
    #else
      #define KAPI __declspec(dllimport)
    #endif
  #else
    #define KAPI
    #define shared_function
  #endif
#elif OS_LINUX
  #if HOTRELOAD_BUILD
    #define shared_function C_LINKAGE __attribute__((visibility("default")))
    #ifdef KEXPORT
      #define KAPI __attribute__((visibility("default")))
    #else
      #define KAPI
    #endif
  #else
    #define KAPI
    #define shared_function
  #endif
#else
  #error OS not supported.
#endif

////////////////////////////////////////////////////////////////////////
// ARCH

#if ARCH_X64
  #define CpuTimerNow() __rdtsc()
#else 
  #error Architecture not supported
#endif

inline u32 most_significant_bit(u64 size) {
  return 63 - clz(size);
}

struct Range {
  u64 offset; 
  u64 size; 
};



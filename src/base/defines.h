#pragma once

#include "stdint.h"
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

#define Swap(a, b) \
  {                \
    auto temp = a; \
    a = b;         \
    b = temp;      \
  }

#define intern static
#define global static
#define local  static

#define KB(n)         (((u64)(n)) << 10)
#define MB(n)         (((u64)(n)) << 20)
#define GB(n)         (((u64)(n)) << 30)
#define TB(n)         (((u64)(n)) << 40)
#define Thousand(n)   ((n)*1000)
#define Million(n)    ((n)*1000000)
#define Billion(n)    ((n)*1000000000ull)
#define BytesToKB(x)  (x / 1024.0f)
#define BytesToMB(x)  (BytesToKB(x) / 1024.0f)
#define BytesToGB(x)  (BytesToMB(x) / 1024.0f)

#define SecToMs(x)    (x * Thousand(1))
#define SecToUs(x)    (x * Million(1))
#define SecToNs(x)    (x * Billion(1))
#define MsToSec(x)    (x / Thousand(1))
#define MsToUs(x)     (x * Thousand(1))
#define MsToNs(x)     (x * Million(1))
#define UsToSec(x)    (x / Million(1))
#define UsToMs(x)     (x / Thousand(1))
#define UsToNs(x)     (x * Thousand(1))
#define NsToSec(x)    (x / Billion(1))
#define NsToMs(x)     (x / Million(1))
#define NsToUs(x)     (x / Thousand(1))

#define Member(T,m)                 (((T*)0)->m)
#define OffsetOf(T,m)               (u64)(&Member(T,m))
#define MemberFromOffset(T,ptr,off) (T)((((u8*)ptr)+(off)))
#define CastFromMember(T,m,ptr)     (T*)(((u8*)ptr) - OffsetOf(T,m))

#define MemZero(d,s)          MemSet(d,0,s)
#define MemZeroStruct(x)      MemZero((x),sizeof(*(x)))
#define MemZeroArray(x)       MemZero((x),sizeof(x))
#define MemZeroTyped(d,c)     MemZero((d),sizeof(*(d))*(c))

#define MemCopyStruct(d, s)   MemCopy((d), (s), sizeof(*(d)))
#define MemCopyArray(d, s)    MemCopy((d), (s), sizeof(x))
#define MemCopyTyped(d, s, c) MemCopy((d), (s), sizeof(*(d)) * (c))

#define MemMatchStruct(a,b)   MemMatch((a),(b),sizeof(*(a)))
#define MemMatchArray(a,b)    MemMatch((a),(b),sizeof(a))

#define AlignUp(x,a)          (((x) + (a) - 1)&(~((a) - 1)))
#define AlignDown(x,a)        ((x)&(~((a) - 1)))
#define AlignPadUp(x,a)       ((0-(x)) & ((a) - 1))
#define AlignPadDown(x, a)    ((x) & ((a) - 1))
#define IsPow2(x)             ((((x) - 1)&(x)) == 0)
#define IsAligned(x, a)       ((((a) - 1)&(x)) == 0)
#define Offset(x, a)          ((u8*)(x) + (a))
#define OffsetBack(x, a)      ((u8*)(x) - (a))
#define MemDiff(from, to)     ((u8*)(from) - (u8*)(to))
#define PtrMatch(a, y)        ((u8*)(a) == (u8*)(y))
#define AlignUpTo(x, a)       (((x) + ((a) - 1)) - (((x) + ((a) - 1)) % (a)))

#define Min(a,b)                      (((a)<(b))?(a):(b))
#define Max(a,b)                      (((a)>=(b))?(a):(b))
#define Max3(a,b,c)                   Max(Max(a, b), c)
#define Min3(a,b,c)                   Min(Min(a, b), c)
#define ClampTop(a,x)                 Min(a,x)
#define ClampBot(x,b)                 Max(x,b)
#define Clamp(a,x,b)                  (((x)<(a))?(a):((x)>(b))?(b):(x))
#define ReverseClamp(a,x,b)           (((x)<(a))?(b):((b)<(x))?(a):(x))
#define Wrap(a,x,b)                   ReverseClamp(a,x,b)
#define ArrayCount(x)                 (sizeof(x) / sizeof((x)[0]))
#define ElemSize(x)                   (sizeof(x[0]))
#define Sqr(x)                        (x*x)
#define Cube(x)                       (x*x*x)
#define Sign(x)                       ((x) < 0 ? -1 : (x) > 0 ? 1 : 0)
#define Abs(x)                        ((x) < 0 ? -(x) : (x))
#define Compose64Bit(a,b)             (((u64)a << 32) | (u64)b)
#define CeilIntDiv(a,b)               (((a) + (b) - 1)/(b))
#define IsBetween(lower, x, upper)    (((lower) <= (x)) && ((x) <= (upper)))
#define Assign(a,b)                   (*((void**)(&(a))) = (void*)(b))
#define As(T)                         *(T*)
#define Transmute(T)                  *(T*)&
#define cast(a)                       (a)
#define Glue(A,B)                     A##B
#define Stringify(S)                  #S
#define Loop(i, c)                    for (i32 i = 0; i < c; ++i)
#define IndexOf(type, mtype, member)  (OffsetOf(type, member) / sizeof(mtype))
#define TrunctPow2(a, b)              ((u64)(a) & ((u64)(b) - 1))
#define u32DivPow2(a, b)              (a >> ctz32(b))
#define u64DivPow2(a, b)              (a >> ctz64(b))

#define Bit(x)                 (1 << (x))
#define HasBit(x, pos)         ((x) & (1 << (pos)))
#define FlagSet(x, f)          ((x) | (f))
#define FlagClear(x, f)        ((x) & ~(f))
#define FlagToggle(x, f)       ((x) ^ (f))
#define FlagExists(x, f)       (((x) & (f)) == (f))
#define FlagEquals(x, f)       ((x) == (f))
#define FlagIntersects(x,f)    (((x) & (f)) > 0)

#define quick_sort(ptr, count, element_size, cmp_function) qsort((ptr), (count), (element_size), (int (*)(const void *, const void *))(cmp_function))

#define CheckNil(nil,p) ((p) == 0 || (p) == nil)
#define SetNil(nil,p) ((p) = nil)
//- rjf: singly-linked, doubly-headed lists (queues)
#define SLLQueuePush_NZ(nil,f,l,n,next) (CheckNil(nil,f) ?                                 \
                                        ((f)=(l)=(n), SetNil(nil, (n)->next)) :            \
                                        ((l)->next=(n), (l)=(n), SetNil(nil, (n)->next)))

#define SLLQueuePush(f,l,n) SLLQueuePush_NZ(0,f,l,n,next)

#define U8_MAX  0xFF
#define U16_MAX 0xFFFF
#define U32_MAX 0xFFFFFFFF
#define U64_MAX 0xFFFFFFFFFFFFFFFF

#define INVALID_ID     U32_MAX
#define INVALID_ID_U16 U16_MAX
#define INVALID_ID_U8  U8_MAX

template<typename F>
struct ImplDefer {
	F f;
	ImplDefer(F f_) : f(f_) {}
	~ImplDefer() { f(); }
};
template<typename F>
ImplDefer<F> MakeDefer(F f) {
	return ImplDefer<F>(f);
}
#define _CONCAT(a, b) Glue(a, b)
#define Defer(code) auto _CONCAT(_defer_, __LINE__) = MakeDefer([&](){code;})
#define DeferLoop(begin, end) for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define IfDeferLoop(begin, end) for (b32 _once = (begin); _once; _once = false, (end))

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

#if BUILD_DEBUG
  #define DebugDo(...) __VA_ARGS__
#else
  #define DebugDo(...)
#endif

#define IfDo(flag, ...)  _IfDo(flag, __VA_ARGS__)
#define _IfDo(flag, ...)  IfDo_##flag(__VA_ARGS__)

#define IfDo_1(code) code
#define IfDo_0(code)

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
// Compiler Specific

#define Likely(expr)           Expect(expr,1)
#define Unlikely(expr)         Expect(expr,0)

#if COMPILER_MSVC
  #define NO_DEBUG
  #define INLINE             __forceinline
  #define DebugBreak()       __debugbreak();
  #define Expect(expr, val)  (expr)

  #include <memory.h>
  #define MemSet(d, byte, c)  memset((d), (byte), (c))
  #define MemCopy(d, s, c)    memcpy((d), (s), (c))
  #define MemMatch(a, b, c)  (memcmp((a), (b), (c)) == 0)

  #if __SANITIZE_ADDRESS__
    #define ASAN_ENABLED 1
    #define NO_ASAN __declspec(no_sanitize_address)
  #else
    #define NO_ASAN
  #endif

#elif COMPILER_CLANG
  #define NO_DEBUG           __attribute__((nodebug))
  #define INLINE             NO_DEBUG inline __attribute__((always_inline))
  #define DebugBreak()       __builtin_debugtrap()
  #define Expect(expr, val)  __builtin_expect((expr), (val))

  #define MemSet(d, byte, c)    __builtin_memset((d), (byte), (c))
  #define MemCopy(d, s, c)      __builtin_memcpy((d), (s), (c))
  #define MemMatch(a, b, c)    (__builtin_memcmp((a), (b), (c)) == 0)

  INLINE u32 count_bits_set32(u32 val) { return __builtin_popcount(val); }
  INLINE u32 count_bits_set64(u64 val) { return __builtin_popcountll(val); }
  INLINE u32 ctz32(u32 val)            { return __builtin_ctz(val); }
  INLINE u32 clz32(u32 val)            { return __builtin_clz(val); }
  INLINE u32 ctz64(u64 val)            { return __builtin_ctzll(val); }
  INLINE u32 clz64(u64 val)            { return __builtin_clzll(val); }

  #define NO_ASAN __attribute__((no_sanitize("address")))

#endif

////////////////////////////////////////////////////////////////////////
// OS Specific

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
#endif

////////////////////////////////////////////////////////////////////////
// ARCH Specific

#if ARCH_X64
  #define ReadTimestampCounter() __rdtsc() 

#else 
  #define ReadTimestampCounter()

#endif

#define DEFAULT_CAPACITY      8
#define DEFAULT_RESIZE_FACTOR 2

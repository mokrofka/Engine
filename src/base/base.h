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
// Compiler

#if COMPILER_CLANG
  #define NO_DEBUG __attribute__((nodebug))
  #define INLINE   NO_DEBUG inline __attribute__((always_inline))
  #define NO_ASAN  __attribute__((no_sanitize("address")))
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
const u64 INVALID_ID     = U32_MAX;
const u64 INVALID_ID_U16 = U16_MAX;
const u64 INVALID_ID_U8  = U8_MAX;
const u64 PAGE_SIZE = 4096;

template<typename T> void Swap(T& a, T& b) {
  T temp = a;
  a = b;
  b = temp;
}

constexpr u64 KB(u64 x) { return x << 10; }
constexpr u64 MB(u64 x) { return x << 20; }
constexpr u64 GB(u64 x) { return x << 30; }
constexpr u64 TB(u64 x) { return x << 40; }
constexpr u64 Thousand(u64 x) { return x * 1000; }
constexpr u64 Million(u64 x)  { return x * 1000000; }
constexpr u64 Billion(u64 x)  { return x * 1000000000; }
KAPI f32 BytesToKB(u64 x);
KAPI f32 BytesToMB(u64 x);
KAPI f32 BytesToGB(u64 x);

KAPI u64 cpu_timer_now();
KAPI void DebugBreak();

////////////////////////////////////////////////////////////////////////
// Memory

#define OffsetOf(T,m) (u64)(&((T*)0)->m)

KAPI void MemSet(void *d, i32 byte, u64 size);
KAPI void MemZero(void *d, u64 size);
KAPI void MemCopy(void* d, void* s, u64 size);
KAPI b32  MemMatch(void* a, void* b, u64 size);

template<typename T> void MemZeroStruct(T* x)              { MemZero(x, sizeof(*x)); };
template<typename T> void MemZeroArray(T* x, u64 c)        { MemZero(x, sizeof(*x)) * c; };
template<typename T> void MemCopyStruct(T* d, T* s)        { MemCopy(d, s, sizeof(*d)); }
template<typename T> void MemCopyArray(T* d, T* s, u64 c)  { MemCopy(d, s, sizeof(*d) * c); }
template<typename T> b32  MemMatchStruct(T* a, T* b)       { return MemMatch(a, b, sizeof(*a)); }
template<typename T> b32  MemMatchArray(T* a, T* b, u64 c) { return MemMatch(a, b, sizeof(*a) * c); }

KAPI u64 AlignUp(u64 x, u64 a);
KAPI u64 AlignDown(u64 x, u64 a);
KAPI u64 AlignPadUp(u64 x, u64 a);
KAPI u64 AlignPadDown(u64 x, u64 a);
KAPI b32 IsPow2(u64 x);
KAPI b32 IsAligned(u64 x, u64 a);
KAPI u8* Offset(void* x, u64 a);
KAPI u8* OffsetBack(void* x, u64 a);
KAPI u64 MemDiff(void* x, void* a);
KAPI b32 PtrMatch(void* a, void* b);

////////////////////////////////////////////////////////////////////////
// Bits

KAPI u32 clz(u64 val);
KAPI u32 ctz(u64 val);
KAPI u32 count_bits_set(u64 val);
KAPI u32 most_significant_bit(u64 size);

constexpr u64 Bit(u64 x) { return 1 << x; }
KAPI u64 BitHas(u64 x, u64 pos);
KAPI u64 FlagSet(u64 x, u64 f);
KAPI u64 FlagClear(u64 x, u64 f);
KAPI u64 FlagToggle(u64 x, u64 f);
KAPI b32 FlagHas(u64 x, u64 f);
KAPI b32 FlagEquals(u64 x, u64 f);
KAPI b32 FlagIntersects(u64 x, u64 f);

////////////////////////////////////////////////////////////////////////
// Common operations

template <typename T> T Max(T a, T b)                { return a > b ? a : b; }
template <typename T> T Min(T a, T b)                { return a < b ? a : b; }
template <typename T> T Max(T a, T b, T c)           { return Max(Max(a,b), c); }
template <typename T> T Min(T a, T b, T c)           { return Min(Min(a,b), c); }
template <typename T> T ClampTop(T x, T a)           { return Min(x, a); };
template <typename T> T ClampBot(T x, T a)           { return Max(x, a); };
template <typename T> T Clamp(T a, T x, T b)         { return (x < a) ? a : (x > b) ? b : x; };
template <typename T> T Reverse_clamp(T a, T x, T b) { return (x < a) ? b : (b < x) ? a : x; };
template <typename T> T Wrap(T a, T x, T b)          { return (x < a) ? b : (b < x) ? a : x; };
template <typename T> T Square(T x)                  { return x * x; };
template <typename T> T Cube(T x)                    { return x * x * x; };
template <typename T> T Sign(T x)                    { return (x < 0) ? -1 : (x > 0) ? 1 : 0; };
template <typename T> T Abs(T x)                     { return (x < 0) ? -x : x; };

template <typename T> b32 IsInRangeIncl(T a, T x, T b) { return (a <= x) && (x <= b); };
template <typename T> b32 IsInRangeExcl(T a, T x, T b) { return (a < x) && (x < b); };
template <typename T> b32 IsInRange(T a, T x, T b)     { return (a <= x) && (x < b); };
KAPI u64 ModPow2(u64 x, u64 b);
KAPI u64 DivPow2(u64 x, u64 b);
KAPI u64 CeilIntDiv(u64 x, u64 b);
KAPI u64 RoundUp(u64 x, u64 a);
KAPI u64 RoundDown(u64 x, u64 a);
KAPI u64 Compose64Bit(u64 a, u64 b);

////////////////////////////////////////////////////////////////////////
// Shenanigans

#define ArrayCount(x)  (sizeof((x)) / sizeof((x)[0]))
#define Assign(a,b)    (*((u8**)(&(a))) = (u8*)(b))
#define As(T)          *(T*)
#define Loop(i, c)     for (i32 i = 0; i < c; ++i)
#define _Stringify(S)  #S
#define Stringify(S)   _Stringify(S)
#define _Glue(A,B)     A##B
#define Glue(A,B)      _Glue(A,B)

////////////////////////////////////////////////////////////////////////
// Link list

#define SLLQueuePush(f,l,n) (f == null) ? \
                            (f = l = n, (n)->next = null) : \
                            ((l)->next = n, l = n, (n)->next = null)

////////////////////////////////////////////////////////////////////////
// Defer

template<typename F>
struct _Defer {
  F f;
  ~_Defer() { f(); }
};
#define defer(code) auto Glue(_defer_, __LINE__) = _Defer([&](){ code; })
#define DeferLoop(begin, end) for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define IfDeferLoop(begin, end) for (b32 _once = (begin); _once; _once = false, (end))

////////////////////////////////////////////////////////////////////////
// Error handling

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

struct Buffer {
  u8* data;
  u64 size;
};

struct Range {
  u64 offset;
  u64 size;
};



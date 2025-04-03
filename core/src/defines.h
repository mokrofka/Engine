#pragma once

// Unsigned i32 types
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

// Signed i32 types
typedef char      i8;
typedef short     i16;
typedef int	      i32;
typedef long long i64;

// Floating point types
typedef float  f32;
typedef double f64;

// Boolean types
typedef char  b8;
typedef short b16;
typedef int   b32;

// char
typedef unsigned char uchar;

#define null 0

#define Swap(A, B) \
  {                \
    auto temp = A; \
    A = B;         \
    B = temp;      \
  }

#define internal static
#define global   static
#define local    static

#define KB(n)  (((u64)(n)) << 10)
#define MB(n)  (((u64)(n)) << 20)
#define GB(n)  (((u64)(n)) << 30)
#define TB(n)  (((u64)(n)) << 40)
#define Thousand(n)   ((n)*1000)
#define Million(n)    ((n)*1000000)
#define Billion(n)    ((n)*1000000000)

#define Min(A,B) (((A)<(B))?(A):(B))
#define Max(A,B) (((A)>(B))?(A):(B))
#define Max3(a, b, c) Max(Max(a, b), c)
#define Min3(a, b, c) Min(Min(a, b), c)

#define ClampTop(A,X) Min(A,X)
#define ClampBot(X,B) Max(X,B)
#define Clamp(A,X,B) (((X)<(A))?(A):((X)>(B))?(B):(X))

#define ReverseClamp(a,x,b) (((x)<(a))?(b):((b)<(x))?(a):(x))
#define Wrap(a,x,b) ReverseClamp(a,x,b)

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define IntFromPtr(ptr) ((u64)(ptr))

#define Member(T,m)                 (((T*)0)->m)
#define OffsetOf(T,m)               IntFromPtr(&Member(T,m))
#define MemberFromOffset(T,ptr,off) (T)((((U8 *)ptr)+(off)))
#define CastFromMember(T,m,ptr)     (T*)(((U8*)ptr) - OffsetOf(T,m))

#define MemZero(s,z)       _memory_zero(s,z)
#define MemZeroStruct(s)   MemZero((s),sizeof(*(s)))
#define MemZeroArray(a)    MemZero((a),sizeof(a))
#define MemZeroTyped(m,c)  MemZero((m),sizeof(*(m))*(c))

#define MemCopy(d, s, c)         _memory_copy((d), (s), (c))
#define MemCopyStruct(d, s)      _memory_copy((d), (s), sizeof(*(d)))
#define MemCopyTyped(d, s, c)    _memory_copy((d), (s), sizeof(*(d)) * (c))
#define MemSet(d, byte, c)       _memory_set((d), (byte), (c))
#define MemCompare(a, b, size)   _memory_compare((a), (b), (size))

#define U32_MAX 4294967295U
#define U64_MAX 18446744073709551615ULL

#define Sqr(x) ((x)*(x))
#define Sign(x) ((x) < 0 ? -1 : (x) > 0 ? 1 : 0)
#define Abs(x) ((x) < 0 ? -(x) : (x))

#define BIT(x) (1 << (x))

#define Glue(A,B) A##B
#define Stringify(S) #S

#define DeferLoop(begin, end) for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define Assign(to, from) (to = (decltype(to))from)

#ifdef KEXPORT
#define KAPI __declspec(dllexport)
#else
#define KAPI __declspec(dllimport)
#endif

#define C_LINKAGE_BEGIN extern "C"{
#define C_LINKAGE_END }
#define C_LINKAGE extern "C"

#define INLINE static inline

#define INVALID_ID 4294967295U

struct String {
  u8* str;
  u64 size;
};
struct Arena;

#define str_lit(S) str((u8*)(S), sizeof(S) - 1)

KAPI String str(u8* str, u64 size);

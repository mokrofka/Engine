#pragma once

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef char      i8;
typedef short     i16;
typedef int	      i32;
typedef long long i64;

typedef float  f32;
typedef double f64;

typedef char  b8;
typedef int   b32;

typedef unsigned char uchar;
typedef u64 PtrInt;
typedef void VoidProc(void);

#define null 0

#define Swap(a, b) \
  {                \
    auto temp = a; \
    a = b;         \
    b = temp;      \
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
#define BytesToKB(x) (x / 1024.f)
#define BytesToMB(x) (BytesToKB(x) / 1024.f)
#define BytesToGB(x) (BytesToMB(x) / 1024.f)

#define Min(a,b) (((a)<(b))?(a):(b))
#define Max(a,b) (((a)>(b))?(a):(b))
#define Max3(a, b, c) Max(Max(a, b), c)
#define Min3(a, b, c) Min(Min(a, b), c)

#define ClampTop(a,x) Min(a,x)
#define ClampBot(x,b) Max(x,b)
#define Clamp(a,x,b) (((x)<(a))?(a):((x)>(b))?(b):(x))
#define ReverseClamp(a,x,b) (((x)<(a))?(b):((b)<(x))?(a):(x))
#define Wrap(a,x,b) ReverseClamp(a,x,b)

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define ElemSize(a) (sizeof(a[0]))

#define Member(T,m)                 (((T*)0)->m)
#define OffsetOf(T,m)               PtrInt(&Member(T,m))
#define MemberFromOffset(T,ptr,off) (T)((((u8 *)ptr)+(off)))
#define CastFromMember(T,m,ptr)     (T*)(((u8*)ptr) - OffsetOf(T,m))
#define Offset(x, y)                (u8*)(x) + (y)
#define PtrMatch(x, y)              ((u8*)(x) == (u8*)(y))

#define MemZero(d,s)       __builtin_memset(d,0,s)
#define MemZeroStruct(a)   MemZero((a),sizeof(*(a)))
#define MemZeroArray(a)    MemZero((a),sizeof(a))
#define MemZeroTyped(d,c)  MemZero((d),sizeof(*(d))*(c))

#define MemCopy(d, s, c)         __builtin_memcpy((d), (s), (c))
#define MemCopyStruct(d, s)      MemCopy((d), (s), sizeof(*(d)))
#define MemCopyTyped(d, s, c)    MemCopy((d), (s), sizeof(*(d)) * (c))
#define MemSet(d, byte, c)       __builtin_memset((d), (byte), (c))

#define MemMatch(a, b, size)     __builtin_memcmp((a), (b), (size))
#define MemMatchStruct(a,b)      MemMatch((a),(b),sizeof(*(a)))
#define MemMatchArray(a,b)       MemMatch((a),(b),sizeof(a))

#define AlignPow2(x,b)     (((x) + (b) - 1)&(~((b) - 1)))
#define AlignDownPow2(x,b) ((x)&(~((b) - 1)))
#define AlignPadPow2(x,b)  ((0-(x)) & ((b) - 1))
#define IsPow2(x)    ((((x) - 1)&(x)) == 0)

#define Sqr(x) ((x)*(x))
#define Sign(x) ((x) < 0 ? -1 : (x) > 0 ? 1 : 0)
#define Abs(x) ((x) < 0 ? -(x) : (x))
#define Compose64Bit(a,b)  (((u64)a << 32) | (u64)b)
#define CeilIntDiv(a,b) (((a) + (b) - 1)/(b))
#define IsBetween(lower, x, upper) (((lower) <= (x)) && ((x) <= (upper)))
#define Assign(a,b) *((void**)(&(a))) = (void*)(b)
#define Transmute(T) *(T*)&

#define Bit(x) (1 << (x))
#define SetBit(x, c) ((x) |= Bit(c))
#define ClearBit(x, c) ((x) &= ~Bit(c))
#define ToggleBit(x, c) ((x) ^= Bit(c))
#define HasBit(x, c) (((x) & Bit(c)) != 0)
#define LowestBit(bitset) __builtin_ctz(bitset)

#define Glue(A,B) A##B
#define Stringify(S) #S

#define Loop(i, c) for (int i = 0; i < c; ++i)

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
#define __CONCAT(a, b) Glue(a, b)
#define Defer(code) auto __CONCAT(_defer_, __LINE__) = MakeDefer([&](){code;})
#define DeferLoop(begin, end) for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define IfDeferLoop(begin_call, end_call) for (b32 _once = (begin_call); _once; _once = false, (end_call))

#ifdef MONOLITHIC_BUILD
  #define KAPI
  #define ExportAPI
#else
#ifdef KEXPORT
  #define KAPI __declspec(dllexport)
#else
  #define KAPI __declspec(dllimport)
#endif
#define ExportAPI __declspec(dllexport)
#endif

#define C_LINKAGE_BEGIN extern "C"{
#define C_LINKAGE_END }
#define C_LINKAGE extern "C"

#define INLINE __forceinline

#define U64_MAX 18446744073709551615ull
#define U32_MAX 4294967295u
#define U16_MAX 65535
#define U8_MAX  255

#define INVALID_ID     U32_MAX
#define INVALID_ID_U16 U16_MAX
#define INVALID_ID_U8  U8_MAX

#define ALLOC_HEADER_GUARD   0xA110C8
#define DEALLOC_HEADER_GUARD 0xDE1E7E
#define ALLOC_GUARD          0xA1
#define DEALLOC_GUARD        0xDE

#ifdef GUARD_MEMORY
  #define FillAlloc(ptr, size)  MemSet(ptr, ALLOC_GUARD, size)
  #define FillAllocStruct(ptr)  MemSet(ptr, ALLOC_GUARD, sizeof(*(ptr)))
  #define FillDealoc(ptr, size) MemSet(ptr, DEALLOC_GUARD, size)
  #define FillDealocStruct(ptr) MemSet(ptr, DEALLOC_GUARD, sizeof(*(ptr)))
#else
  #define FillAlloc(ptr, size)
  #define FillAllocStruct(ptr)
  #define FillDealoc(ptr, size)
  #define FillDealocStruct(ptr)
#endif

struct Arena;

#define quick_sort(ptr, count, element_size, cmp_function) qsort((ptr), (count), (element_size), (int (*)(const void *, const void *))(cmp_function))


#define Main                     \
  void entry_point();            \
  int main() {                   \
    os_entry_point(entry_point); \
  } void

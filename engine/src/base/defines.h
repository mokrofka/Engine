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
#define MaxEntities KB(10)

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
#define IntFromPtr(a) ((u64)(a))

#define Member(T,m)                 (((T*)0)->m)
#define OffsetOf(T,m)               IntFromPtr(&Member(T,m))
#define MemberFromOffset(T,ptr,off) (T)((((u8 *)ptr)+(off)))
#define CastFromMember(T,m,ptr)     (T*)(((u8*)ptr) - OffsetOf(T,m))

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
#define IsPow2(x)          ((x)!=0 && ((x)&((x)-1))==0)
#define IsPow2OrZero(x)    ((((x) - 1)&(x)) == 0)

#define Sqr(x) ((x)*(x))
#define Sign(x) ((x) < 0 ? -1 : (x) > 0 ? 1 : 0)
#define Abs(x) ((x) < 0 ? -(x) : (x))
#define Compose64Bit(a,b)  (((u64)a << 32) | (u64)b)
#define CeilIntDiv(a,b) (((a) + (b) - 1)/(b))
#define IsBetween(lower, x, upper) (((lower) <= (x)) && ((x) <= (upper)))
#define Assign(a,b) *((void**)(&(a))) = (void*)(b)
#define GetProcAddr(x,l,s) Assign((x), os_lib_get_proc((l), (s)))

#define Bit(x) (1 << (x))
#define SetBit(x, c) ((x) |= Bit(c))
#define ClearBit(x, c) ((x) &= ~Bit(c))
#define ToggleBit(x, c) ((x) ^= Bit(c))
#define HasBit(x, c) (((x) & Bit(c)) != 0)

#define Glue(A,B) A##B
#define Stringify(S) #S

#define DeferLoop(begin, end) for (int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))
#define Loop(i, c) for (int i = 0; i < c; ++i)
#define LoopC(i, c) for (int i = 0, _end = (c); i < _end; ++i)

#define Func(a) struct a { static

#ifdef KEXPORT
  #define KAPI __declspec(dllexport)
#else
  #define KAPI __declspec(dllimport)
#endif
#define ExportAPI __declspec(dllexport)

#define C_LINKAGE_BEGIN extern "C"{
#define C_LINKAGE_END }
#define C_LINKAGE extern "C"

#define INLINE __forceinline
#if _DEBUG
#define DebugDo(x) (x)
#endif

#define U64_MAX 18446744073709551615ull
#define U32_MAX 4294967295u
#define U16_MAX 65535
#define U8_MAX  255

#define INVALID_ID     U32_MAX
#define INVALID_ID_U16 U16_MAX
#define INVALID_ID_U8  U8_MAX

#define ZERO_MEMORY
#ifdef ZERO_MEMORY
  #define MemClear(ptr, size) MemZero(ptr, size)
  #define MemClearStruct(ptr) MemZero(ptr, sizeof(*(ptr)))
#else
  #define MemClear(ptr, size)
  #define MemClearStruct(ptr)
#endif

// #define UI_Window(begin) DeferLoop(begin, ImGui::End())
struct String {
  u8* str;
  u32 size;
  INLINE operator bool() {
    return size;
  }
  // INLINE operator char*() {
  //   return (char*)str;
  // }
};

struct Arena;

#define quick_sort(ptr, count, element_size, cmp_function) qsort((ptr), (count), (element_size), (int (*)(const void *, const void *))(cmp_function))

//- rjf: linked list macro helpers
#define CheckNil(nil,p) ((p) == 0 || (p) == nil)
#define SetNil(nil,p) ((p) = nil)

//- rjf: doubly-linked-lists
#define DLLInsert_NPZ(nil,f,l,p,n,next,prev) (CheckNil(nil,f) ? \
((f) = (l) = (n), SetNil(nil,(n)->next), SetNil(nil,(n)->prev)) :\
CheckNil(nil,p) ? \
((n)->next = (f), (f)->prev = (n), (f) = (n), SetNil(nil,(n)->prev)) :\
((p)==(l)) ? \
((l)->next = (n), (n)->prev = (l), (l) = (n), SetNil(nil, (n)->next)) :\
(((!CheckNil(nil,p) && CheckNil(nil,(p)->next)) ? (0) : ((p)->next->prev = (n))), ((n)->next = (p)->next), ((p)->next = (n)), ((n)->prev = (p))))
#define DLLPushBack_NPZ(nil,f,l,n,next,prev) DLLInsert_NPZ(nil,f,l,l,n,next,prev)
#define DLLPushFront_NPZ(nil,f,l,n,next,prev) DLLInsert_NPZ(nil,l,f,f,n,prev,next)
#define DLLRemove_NPZ(nil,f,l,n,next,prev) (((n) == (f) ? (f) = (n)->next : (0)),\
((n) == (l) ? (l) = (l)->prev : (0)),\
(CheckNil(nil,(n)->prev) ? (0) :\
((n)->prev->next = (n)->next)),\
(CheckNil(nil,(n)->next) ? (0) :\
((n)->next->prev = (n)->prev)))

//- rjf: singly-linked, doubly-headed lists (queues)
#define SLLQueuePush_NZ(nil,f,l,n,next) (CheckNil(nil,f)?\
((f)=(l)=(n),SetNil(nil,(n)->next)):\
((l)->next=(n),(l)=(n),SetNil(nil,(n)->next)))
#define SLLQueuePushFront_NZ(nil,f,l,n,next) (CheckNil(nil,f)?\
((f)=(l)=(n),SetNil(nil,(n)->next)):\
((n)->next=(f),(f)=(n)))
#define SLLQueuePop_NZ(nil,f,l,next) ((f)==(l)?\
(SetNil(nil,f),SetNil(nil,l)):\
((f)=(f)->next))

//- rjf: singly-linked, singly-headed lists (stacks)
#define SLLStackPush_N(f,n,next) ((n)->next=(f), (f)=(n))
#define SLLStackPop_N(f,next) ((f)=(f)->next)

//- rjf: doubly-linked-list helpers
#define DLLInsert_NP(f,l,p,n,next,prev) DLLInsert_NPZ(0,f,l,p,n,next,prev)
#define DLLPushBack_NP(f,l,n,next,prev) DLLPushBack_NPZ(0,f,l,n,next,prev)
#define DLLPushFront_NP(f,l,n,next,prev) DLLPushFront_NPZ(0,f,l,n,next,prev)
#define DLLRemove_NP(f,l,n,next,prev) DLLRemove_NPZ(0,f,l,n,next,prev)
#define DLLInsert(f,l,p,n) DLLInsert_NPZ(0,f,l,p,n,next,prev)
#define DLLPushBack(f,l,n) DLLPushBack_NPZ(0,f,l,n,next,prev)
#define DLLPushFront(f,l,n) DLLPushFront_NPZ(0,f,l,n,next,prev)
#define DLLRemove(f,l,n) DLLRemove_NPZ(0,f,l,n,next,prev)

//- rjf: singly-linked, doubly-headed list helpers
#define SLLQueuePush_N(f,l,n,next) SLLQueuePush_NZ(0,f,l,n,next)
#define SLLQueuePushFront_N(f,l,n,next) SLLQueuePushFront_NZ(0,f,l,n,next)
#define SLLQueuePop_N(f,l,next) SLLQueuePop_NZ(0,f,l,next)
#define SLLQueuePush(f,l,n) SLLQueuePush_NZ(0,f,l,n,next)
#define SLLQueuePushFront(f,l,n) SLLQueuePushFront_NZ(0,f,l,n,next)
#define SLLQueuePop(f,l) SLLQueuePop_NZ(0,f,l,next)

//- rjf: singly-linked, singly-headed list helpers
#define SLLStackPush(f,n) SLLStackPush_N(f,n,next)
#define SLLStackPop(f) SLLStackPop_N(f,next)


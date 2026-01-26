#include "base.h"

////////////////////////////////////////////////////////////////////////
// Basic

f32 BytesToKB(u64 x) { return (f32)x / 1024; };
f32 BytesToMB(u64 x) { return BytesToKB(x) / 1024; };
f32 BytesToGB(u64 x) { return BytesToMB(x) / 1024; };

u64 cpu_timer_now() { return __rdtsc(); }
void DebugBreak()   { __builtin_debugtrap(); }

////////////////////////////////////////////////////////////////////////
// Memory

void MemSet(void *d, i32 byte, u64 size)  { __builtin_memset(d, byte, size); }
void MemZero(void *d, u64 size)           { MemSet(d, 0, size); }
void MemCopy(void* d, void* s, u64 size)  { __builtin_memcpy(d, s, size); }
b32  MemMatch(void* a, void* b, u64 size) { return (__builtin_memcmp(a, b, size) == 0); }

u64 AlignUp(u64 x, u64 a)      { return (x + a - 1) & ~(a - 1); }
u64 AlignDown(u64 x, u64 a)    { return x & ~(a - 1); }
u64 AlignPadUp(u64 x, u64 a)   { return -x & (a - 1); }
u64 AlignPadDown(u64 x, u64 a) { return x & (a - 1); }
b32 IsPow2(u64 x)              { return ((x - 1) & x) == 0; }
b32 IsAligned(u64 x, u64 a)    { return ((a - 1) & x) == 0; }
u8* Offset(void* x, u64 a)     { return (u8*)x + a; }
u8* OffsetBack(void* x, u64 a) { return (u8*)x - a; }
u64 MemDiff(void* x, void* a)  { return (u8*)x - (u8*)a; }
b32 PtrMatch(void* a, void* b) { return (u8*)a == (u8*)b; }

////////////////////////////////////////////////////////////////////////
// Bits

u32 clz(u64 val)                   { return __builtin_clzll(val); }
u32 ctz(u64 val)                   { return __builtin_ctzll(val); }
u32 count_bits_set(u64 val)        { return __builtin_popcountll(val); }
u32 most_significant_bit(u64 size) { return 63 - clz(size); }

u64 BitHas(u64 x, u64 pos)       { return x & (1 << pos); }
u64 FlagSet(u64 x, u64 f)        { return x | f; }
u64 FlagClear(u64 x, u64 f)      { return x & ~f; }
u64 FlagToggle(u64 x, u64 f)     { return x ^ f; }
b32 FlagHas(u64 x, u64 f)        { return (x & f) == (f); }
b32 FlagEquals(u64 x, u64 f)     { return x == f; }
b32 FlagIntersects(u64 x, u64 f) { return (x & f) > 0; }

////////////////////////////////////////////////////////////////////////
// Common operations

KAPI u64 ModPow2(u64 x, u64 b)      { return x & (b - 1); }
KAPI u64 DivPow2(u64 x, u64 b)      { return x >> ctz(b); }
KAPI u64 CeilIntDiv(u64 x, u64 b)   { return (x + b - 1) / b; }
KAPI u64 RoundUp(u64 x, u64 a)      { return CeilIntDiv(x, a) * a; }
KAPI u64 RoundDown(u64 x, u64 a)    { return x / a * a; }
KAPI u64 Compose64Bit(u64 a, u64 b) { return (a << 32) | b; }

////////////////////////////////////////////////////////////////////////
// Types

u64 range_size(Range r) {
  return  r.size - r.offset;
}


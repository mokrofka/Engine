#include "base.h"

////////////////////////////////////////////////////////////////////////
// Basic

f32 BytesToKB(u64 x) { return (f32)x / 1024; };
f32 BytesToMB(u64 x) { return BytesToKB(x) / 1024; };
f32 BytesToGB(u64 x) { return BytesToMB(x) / 1024; };

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

u64 ModPow2(u64 x, u64 b)      { return x & (b - 1); }
u64 DivPow2(u64 x, u64 b)      { return x >> ctz(b); }
u64 CeilIntDiv(u64 x, u64 b)   { return (x + b - 1) / b; }
u64 RoundUp(u64 x, u64 a)      { return CeilIntDiv(x, a) * a; }
u64 RoundDown(u64 x, u64 a)    { return x / a * a; }
u64 Compose64Bit(u64 a, u64 b) { return (a << 32) | b; }
u32 next_pow2(u32 v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}
u32 prev_pow2(u32 n) {
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	return n - (n >> 1);
}

////////////////////////////////////////////////////////////////////////
// Asserts

void Trap()      { __builtin_trap(); }
void DebugTrap() { __builtin_debugtrap(); }

global u64 _cpu_frequency;

u64 cpu_timer_now() { return __rdtsc(); }
u64 cpu_frequency() { return _cpu_frequency; }

#include "../os/os_core.h"
void estimate_cpu_frequency() {
  u64 os_freq = os_timer_frequency();
  u64 cpu_start = cpu_timer_now();
  u64 os_start = os_timer_now();
  u64 milliseconds = 1;
  u64 os_end = 0;
  u64 os_elapsed = 0;
  u64 os_wait_time = os_freq * milliseconds / 1000;
  while (os_elapsed < os_wait_time) {
    os_end = os_timer_now();
    os_elapsed = os_end - os_start;
  }
  u64 cpu_end = cpu_timer_now();
  u64 cpu_elapsed = cpu_end - cpu_start;
  u64 cpu_freq = 0;
  if (cpu_elapsed) {
    cpu_freq = os_freq * cpu_elapsed / os_elapsed;
  }
  _cpu_frequency = cpu_freq;
}

////////////////////////////////////////////////////////////////////////
// Types

void ring_write(RingBuffer& ring, void *src, u64 src_size) {
  Assert(src_size <= (ring.size - (ring.write_pos - ring.read_pos)));
  u64 offset = ModPow2(ring.write_pos, ring.size);
  u64 first = Min(ring.size - offset, src_size);
  u64 second = src_size - first;
  MemCopy(ring.base + offset, src, first);
  if (second) {
    MemCopy(ring.base, Offset(src, first), second);
  }
  ring.write_pos += src_size;
}

void ring_read(RingBuffer& ring, void *dst, u64 dst_size) {
  Assert(dst_size <= (ring.write_pos - ring.read_pos));
  u64 offset = ModPow2(ring.read_pos, ring.size);
  u64 first = Min(ring.size - offset, dst_size);
  u64 second = dst_size - first;
  MemCopy(dst, ring.base+offset, first);
  if (second) {
    MemCopy(Offset(dst, first), ring.base, second);
  }
  ring.read_pos += dst_size;
}


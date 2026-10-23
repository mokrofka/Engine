#include "base.h"
#include "os_core_linux.cpp"
#include "os_core_win32.cpp"
#include "os_gfx_linux_wayland.cpp"
#include "os_gfx_linux_x11.cpp"

GlobalVar f32 time_dt;
GlobalVar f32 time_now;
GlobalVar u32 current_frame;
GlobalVar u64 cpu_frequency;
GlobalVar ThreadPool thread_st;
GlobalVar ProfState prof_st;
global_var thread_local u64 _seed = HASH_DEFAULT_SEED;
global_var u32 _next_thread_id;
global_var thread_local TCTX tctx;
#if MEM_TRACK
global_var MemTrackState mem_track_st;
#endif

u64 u64_pow10[] = {
	1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
	1000000000ull, 10000000000ull, 100000000000ull, 1000000000000ull,
	10000000000000ull, 100000000000000ull, 1000000000000000ull,
	10000000000000000ull, 100000000000000000ull, 1000000000000000000ull,
	10000000000000000000ull
};

f64 f64_pow10[23] = {
	1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11,
	1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22,
};

f32 BytesToKB(u64 x) { return (f32)x / 1024; };
f32 BytesToMB(u64 x) { return BytesToKB(x) / 1024; };
f32 BytesToGB(u64 x) { return BytesToMB(x) / 1024; };

////////////////////////////////////////////////////////////////////////
// Memory
void MemSet(void *d, i32 byte, u64 size)  { __builtin_memset(d, byte, size); }
void MemZero(void *d, u64 size)           { MemSet(d, 0, size); }
void MemCopy(void* d, void* s, u64 size)  { __builtin_memcpy(d, s, size); }
b32  MemMatch(void* a, void* b, u64 size) { return (__builtin_memcmp(a, b, size) == 0); }

u64 align_up(u64 x, u64 a)             { return (x + a - 1) & ~(a - 1); }
u64 align_down(u64 x, u64 a)           { return x & ~(a - 1); }
u64 align_pad_up(u64 x, u64 a)         { return -x & (a - 1); }
u64 align_pad_down(u64 x, u64 a)       { return x & (a - 1); }
b32 is_aligned(u64 x, u64 a)           { return ((a - 1) & x) == 0; }
u8* align_up_ptr(void* x, u64 a)       { return (u8*)align_up(u64(x), a); }
u8* align_down_ptr(void* x, u64 a)     { return (u8*)align_down(u64(x), a); }
u8* align_pad_up_ptr(void* x, u64 a)   { return (u8*)align_pad_up(u64(x), a); }
u8* align_pad_down_ptr(void* x, u64 a) { return (u8*)align_pad_down(u64(x), a); }
b32 is_aligned_ptr(void* x, u64 a)     { return  is_aligned(u64(x), a); }
b32 is_pow2(u64 x)                     { return ((x - 1) & x) == 0; }
u64 ptr_diff(void* a, void* b)         { return (u8*)a - (u8*)a; }
b32 ptr_match(void* a, void* b)        { return (u8*)a == (u8*)b; }
u8* Offset(void* x, u64 a)             { return (u8*)x + a; }
u8* OffsetBack(void* x, u64 a)         { return (u8*)x - a; }

////////////////////////////////////////////////////////////////////////
// Bits
u32 clz(u64 v)                     { return __builtin_clzll(v); }
u32 clz32(u32 v)																	  { return __builtin_clz(v); }
u32 ctz(u64 v)                     { return __builtin_ctzll(v); }
u32 ctzu32(u32 v)																	 { return __builtin_ctz(v); }
u32 count_ones(u64 v)              { return __builtin_popcountll(v); }
u32 most_significant_bit(u32 v)    { return 31 - clz32(v); }
u64 most_significant_bit(u64 v)    { return 63 - clz(v); }
u32 remove_lowest_bit(u64 v)       { return v & (v - 1);}
u32 remove_highest_bit(u32 v)      { return v ^ (1 << most_significant_bit(v)); }
u64 remove_highest_bit(u64 v)      { return v ^ (1 << most_significant_bit(v)); }
b32 bit_has(u64 x, u64 pos)        { return (x >> pos) & 1; }
u64 flag_clear(u64 x, u64 f)       { return x & ~f; }
u64 flag_toggle(u64 x, u64 f)      { return x ^ f; }
b32 flag_has(u64 x, u64 f)         { return (x & f) == (f); }
b32 flag_any(u64 x, u64 f)         { return x & f; }

////////////////////////////////////////////////////////////////////////
// Common operations
u64 mod_pow2(u64 x, u64 b)   { return x & (b - 1); }
u64 div_pow2(u64 x, u64 b)   { return x >> ctz(b); }
u64 div_ceil(u64 x, u64 b)   { return (x + b - 1) / b; }
u64 round_up(u64 x, u64 a)   { return div_ceil(x, a) * a; }
u64 round_down(u64 x, u64 a) { return x / a * a; }
u64 compose_64(u32 a, u32 b) { return ((u64)a << 32) | b; }
u32 next_pow2(u32 x) {
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x;
}
u32 prev_pow2(u32 x) {
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x - (x >> 1);
}

////////////////////////////////////////////////////////////////////////
// Asserts
void Trap()      { __builtin_trap(); }
void DebugTrap() { __builtin_debugtrap(); }

////////////////////////////////////////////////////////////////////////
// Types
RingBuffer ring_make(void* base, u64 size) {
	RingBuffer res = {
		.base = (u8*)base,
		.size = size,
	};
	return res;
}

u64 ring_write(RingBuffer& ring, void* src, u64 src_size) {
	Assert(ring.size >= src_size);
	u64 offset = ring.write_pos % ring.size;
	u64 first = Min(ring.size - offset, src_size);
	u64 second = src_size - first;
	MemCopy(ring.base + offset, src, first);
	if(second) {
		MemCopy(ring.base, Offset(src, first), second);
	}
	ring.write_pos += src_size;
	return offset;
}

u64 ring_read(RingBuffer& ring, void* dst, u64 dst_size) {
	Assert(ring.size >= dst_size);
	u64 offset = ring.read_pos % ring.size;
	u64 first = Min(ring.size - offset, dst_size);
	u64 second = dst_size - first;
	MemCopy(dst, ring.base+offset, first);
	if(second) {
		MemCopy(Offset(dst, first), ring.base, second);
	}
	ring.read_pos += dst_size;
	return offset;
}

u64 ring_write_nowrap(RingBuffer& ring, void* src, u64 src_size) {
	Assert(ring.size >= src_size);
	u64 offset = ring.write_pos % ring.size;
	u64 tail = ring.size - offset;
	b32 wrap = src_size > tail;
	if(wrap) {
		ring.write_pos += tail;
		offset = 0;
	}
	MemCopy(ring.base + offset, src, src_size);
	ring.write_pos += src_size;
	return offset;
}

u64 ring_read_nowrap(RingBuffer& ring, void* dst, u64 dst_size) {
	Assert(ring.size >= dst_size);
	u64 offset = ring.read_pos % ring.size;
	u64 tail = ring.size - offset;
	b32 wrap = dst_size > tail;
	if(wrap) {
		ring.read_pos += tail;
		offset = 0;
	}
	MemCopy(dst, ring.base+offset, dst_size);
	ring.read_pos += dst_size;
	return offset;
}

u8* Restrict _coroutine_var(Coroutine& co, u32 size) {
	Assert(co.stack_pointer + size < CoroutineStackSize);
	u8* res = co.stack + co.stack_pointer;
	co.stack_pointer += size;
	return res;
}

////////////////////////////////////////////////////////////////////////
// Simd
void cpu_relax() { __builtin_ia32_pause(); }

///////////////////////////////////
// xmmintrin.h
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __min_vector_width__(128)))
#define __DEFAULT_FN_ATTRS_CONSTEXPR

static __inline__ __m128 __DEFAULT_FN_ATTRS _mm_loadu_ps(const float* __p) {
	struct __loadu_ps {
		__m128_u __v;
	} __attribute__((__packed__, __may_alias__));
	return ((const struct __loadu_ps*)__p)->__v;
}
static __inline__ void __DEFAULT_FN_ATTRS _mm_storeu_ps(float *__p, __m128 __a) {
	struct __storeu_ps {
		__m128_u __v;
	} __attribute__((__packed__, __may_alias__));
	((struct __storeu_ps*)__p)->__v = __a;
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_set1_ps(float __w) {
	return __extension__(__m128){__w, __w, __w, __w};
}
static __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_setr_ps(float __z, float __y, float __x, float __w) {
	return __extension__(__m128){__z, __y, __x, __w};
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_setzero_ps(void) {
	return __extension__(__m128){0.0f, 0.0f, 0.0f, 0.0f};
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_add_ps(__m128 __a, __m128 __b) {
	return (__m128)((__v4sf)__a + (__v4sf)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_sub_ps(__m128 __a, __m128 __b) {
	return (__m128)((__v4sf)__a - (__v4sf)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_mul_ps(__m128 __a, __m128 __b) {
	return (__m128)((__v4sf)__a * (__v4sf)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_div_ps(__m128 __a, __m128 __b) {
	return (__m128)((__v4sf)__a / (__v4sf)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_xor_ps(__m128 __a, __m128 __b) {
	return (__m128)((__v4su)__a ^ (__v4su)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_and_ps(__m128 __a, __m128 __b) {
	return (__m128)((__v4su)__a & (__v4su)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_or_ps(__m128 __a, __m128 __b) {
	return (__m128)((__v4su)__a | (__v4su)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS _mm_cmplt_ps(__m128 __a, __m128 __b) {
	return (__m128)__builtin_ia32_cmpltps((__v4sf)__a, (__v4sf)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS _mm_cmple_ps(__m128 __a, __m128 __b) {
	return (__m128)__builtin_ia32_cmpleps((__v4sf)__a, (__v4sf)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS _mm_cmpgt_ps(__m128 __a, __m128 __b) {
	return (__m128)__builtin_ia32_cmpltps((__v4sf)__b, (__v4sf)__a);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS _mm_cmpge_ps(__m128 __a, __m128 __b) {
	return (__m128)__builtin_ia32_cmpleps((__v4sf)__b, (__v4sf)__a);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS _mm_cmpeq_ps(__m128 __a, __m128 __b) {
	return (__m128)__builtin_ia32_cmpeqps((__v4sf)__a, (__v4sf)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS _mm_cmpneq_ps(__m128 __a, __m128 __b) {
	return (__m128)__builtin_ia32_cmpneqps((__v4sf)__a, (__v4sf)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_min_ps(__m128 __a, __m128 __b) {
	return __builtin_ia32_minps((__v4sf)__a, (__v4sf)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_max_ps(__m128 __a, __m128 __b) {
	return __builtin_ia32_maxps((__v4sf)__a, (__v4sf)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_andnot_ps(__m128 __a, __m128 __b) {
	return (__m128)(~(__v4su)__a & (__v4su)__b);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS _mm_sqrt_ps(__m128 __a) {
	return __builtin_elementwise_sqrt(__a);
}
static __inline__ int __DEFAULT_FN_ATTRS_CONSTEXPR _mm_movemask_ps(__m128 __a) {
	return __builtin_ia32_movmskps((__v4sf)__a);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_unpacklo_ps(__m128 __a, __m128 __b) {
  return __builtin_shufflevector((__v4sf)__a, (__v4sf)__b, 0, 4, 1, 5);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_unpackhi_ps(__m128 __a, __m128 __b) {
	return __builtin_shufflevector((__v4sf)__a, (__v4sf)__b, 2, 6, 3, 7);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_movelh_ps(__m128 __a, __m128 __b) {
	return __builtin_shufflevector((__v4sf)__a, (__v4sf)__b, 0, 1, 4, 5);
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_movehl_ps(__m128 __a, __m128 __b) {
  return __builtin_shufflevector((__v4sf)__a, (__v4sf)__b, 6, 7, 2, 3);
}

///////////////////////////////////
// smmintrin.h
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_blendv_ps(__m128 __V1, __m128 __V2, __m128 __M) {
	return (__m128)__builtin_ia32_blendvps((__v4sf)__V1, (__v4sf)__V2, (__v4sf)__M);
}

#define _MM_FROUND_TO_NEAREST_INT 0x00
#define _MM_FROUND_TO_NEG_INF 0x01
#define _MM_FROUND_TO_POS_INF 0x02
#define _MM_FROUND_TO_ZERO 0x03
#define _MM_FROUND_CUR_DIRECTION 0x04

#define _MM_FROUND_RAISE_EXC 0x00
#define _MM_FROUND_NO_EXC 0x08

#define _MM_FROUND_NINT (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_NEAREST_INT)
#define _MM_FROUND_FLOOR (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_NEG_INF)
#define _MM_FROUND_CEIL (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_POS_INF)
#define _MM_FROUND_TRUNC (_MM_FROUND_RAISE_EXC | _MM_FROUND_TO_ZERO)
#define _MM_FROUND_RINT (_MM_FROUND_RAISE_EXC | _MM_FROUND_CUR_DIRECTION)
#define _MM_FROUND_NEARBYINT (_MM_FROUND_NO_EXC | _MM_FROUND_CUR_DIRECTION)

#define _mm_round_ps(X, M) ((__m128)__builtin_ia32_roundps((__v4sf)(__m128)(X), (M)))
#define _mm_floor_ps(X) _mm_round_ps((X), _MM_FROUND_FLOOR)

///////////////////////////////////
// emmintrin.h
static __inline__ __m128i __DEFAULT_FN_ATTRS_CONSTEXPR _mm_castps_si128(__m128 __a) {
	return (__m128i)__a;
}
static __inline__ __m128 __DEFAULT_FN_ATTRS_CONSTEXPR _mm_castsi128_ps(__m128i __a) {
	return (__m128)__a;
}
static __inline__ __m128i __DEFAULT_FN_ATTRS_CONSTEXPR _mm_set_epi32(int __i3, int __i2, int __i1, int __i0) {
  return __extension__(__m128i)(__v4si){__i0, __i1, __i2, __i3};
}
static __inline__ __m128i __DEFAULT_FN_ATTRS_CONSTEXPR _mm_set1_epi32(int __i) {
	return _mm_set_epi32(__i, __i, __i, __i);
}

f32x4 simd_load(void* p)                    { return {_mm_loadu_ps((f32*)p)}; }
void simd_store(f32x4 x, void* p)           { _mm_storeu_ps((f32*)p, x.p); }
f32x4 simd_splat(f32 x)                     { return {_mm_set1_ps(x)}; }
f32x4 simd_make(f32 x, f32 y, f32 z, f32 w) { return {_mm_setr_ps(x,y,z,w)}; }
f32x4 simd_zero()                           { return {.p = _mm_setzero_ps()}; }

f32x4 operator+(f32x4 a, f32x4 b) { return {_mm_add_ps(a.p, b.p)}; }
f32x4 operator-(f32x4 a, f32x4 b) { return {_mm_sub_ps(a.p, b.p)}; }
f32x4 operator-(f32x4 a)          { return {simd_zero() - a}; }
f32x4 operator*(f32x4 a, f32x4 b) { return {_mm_mul_ps(a.p, b.p)}; }
f32x4 operator/(f32x4 a, f32x4 b) { return {_mm_div_ps(a.p, b.p)}; }
f32x4 operator^(f32x4 a, f32x4 b) { return {_mm_xor_ps(a.p, b.p)}; }
f32x4 operator&(f32x4 a, f32x4 b) { return {_mm_and_ps(a.p, b.p)}; }
f32x4 operator|(f32x4 a, f32x4 b) { return {_mm_or_ps(a.p, b.p)}; }

f32x4 operator<(f32x4 a, f32x4 b)  { return {_mm_cmplt_ps(a.p, b.p)}; }
f32x4 operator<=(f32x4 a, f32x4 b) { return {_mm_cmple_ps(a.p, b.p)}; }
f32x4 operator>(f32x4 a, f32x4 b)  { return {_mm_cmpgt_ps(a.p, b.p)}; }
f32x4 operator>=(f32x4 a, f32x4 b) { return {_mm_cmpge_ps(a.p, b.p)}; }
f32x4 operator==(f32x4 a, f32x4 b) { return {_mm_cmpeq_ps(a.p, b.p)}; }
f32x4 operator!=(f32x4 a, f32x4 b) { return {_mm_cmpneq_ps(a.p, b.p)}; }

f32x4& operator+=(f32x4& a, f32x4 b) { a = a + b; return a; }
f32x4& operator-=(f32x4& a, f32x4 b) { a = a - b; return a; }
f32x4& operator*=(f32x4& a, f32x4 b) { a = a * b; return a; }
f32x4& operator/=(f32x4& a, f32x4 b) { a = a / b; return a; }
f32x4& operator^=(f32x4& a, f32x4 b) { a = a ^ b; return a; }
f32x4& operator&=(f32x4& a, f32x4 b) { a = a & b; return a; }
f32x4& operator|=(f32x4& a, f32x4 b) { a = a | b; return a; }

f32x4 simd_min(f32x4 a, f32x4 b)            { return {_mm_min_ps(a.p, b.p)}; }
f32x4 simd_max(f32x4 a, f32x4 b)            { return {_mm_max_ps(a.p, b.p)}; }
f32x4 simd_andnot(f32x4 a, f32x4 b)         { return {_mm_andnot_ps(a.p, b.p)}; };
f32x4 simd_sqrt(f32x4 a)                    { return {_mm_sqrt_ps(a.p)}; }
f32x4 simd_blend(f32x4 a, f32x4 m, f32x4 b) { return {_mm_blendv_ps(a.p,m.p,b.p)}; }
f32x4 simd_floor(f32x4 a)                   { return {_mm_floor_ps(a.p)}; }
u32 simd_movemask(f32x4 a)                  { return _mm_movemask_ps(a.p); }
f32x4 simd_unpacklo(f32x4 a, f32x4 b)       { return {_mm_unpacklo_ps(a.p, b.p)}; }
f32x4 simd_unpackhi(f32x4 a, f32x4 b)       { return {_mm_unpackhi_ps(a.p, b.p)}; }
f32x4 simd_movelh(f32x4 a, f32x4 b)         { return {_mm_movelh_ps(a.p, b.p)}; }
f32x4 simd_movehl(f32x4 a, f32x4 b)         { return {_mm_movehl_ps(a.p, b.p)}; }

f32x4 simd_cast_itof(f32x4 a) { return {_mm_castsi128_ps(a.p)}; }
f32x4 simd_cast_ftoi(f32x4 a) { return {_mm_castps_si128(a.p)}; }

const f32x4 f32x4SignMask = simd_cast_itof(f32x4(_mm_set1_epi32(U32_MAX-1)));

f32x4 simd_abs(f32x4 a)                     { return simd_andnot(f32x4(f32x4SignMask), a); }
f32x4 simd_sign_bit(f32x4 a)                { return a & f32x4(f32x4SignMask); };
f32x4 simd_sign_of(f32x4 a)                 { return simd_splat(1) | simd_sign_bit(a); };
f32x4 simd_clamp(f32x4 a, f32x4 x, f32x4 b) { return simd_min(simd_max(a, x), b); }
f32x4 simd_clamp01(f32x4 x)                 { return simd_clamp(simd_zero(), x, simd_splat(1)); }

void _log_output(LogLevel level, String fmt, ...) {
	Scratch scratch;
	String level_strings[] = {"[TRACE]: ", "[DEBUG]: ", "[INFO]:  ", "[WARN]:  ", "[ERROR]: "};
	VaList argc;
	va_start(argc, fmt);
	String formatted = push_strfv(scratch, fmt, argc);
	va_end(argc);
	String out_message = push_strf(scratch, "%s%s\n", level_strings[level-1], formatted);
	os_console_write(out_message, level);
}

void print(String fmt, ...) {
	Scratch scratch;
	VaList argc;
	va_start(argc, fmt);
	String formatted = push_strfv(scratch, fmt, argc);
	va_end(argc);
	String out_message = push_strf(scratch, "%s", formatted);
	os_console_write(out_message, 0);
}

void println(String fmt, ...) {
	Scratch scratch;
	VaList argc;
	va_start(argc, fmt);
	String formatted = push_strfv(scratch, fmt, argc);
	va_end(argc);
	String out_message = push_strf(scratch, "%s\n", formatted);
	os_console_write(out_message, 0);
}

u64 cpu_now()       { return __rdtsc(); }
void cpu_find_frequency() {
	u64 cpu_start = cpu_now();
	u64 start_ns = os_now_ns();
	u64 ns_elapsed = 0;
	while(ns_elapsed < Million(1)) {
		ns_elapsed = os_now_ns() - start_ns;
	}
	u64 cpu_elapsed = cpu_now() - cpu_start;
	cpu_frequency = Billion(1) / ns_elapsed * cpu_elapsed;
}

// struct CoroCtx {
//   void *stack_pointer;
// };

// struct Coro {
//   CoroCtx ctx;
//   Coro* caller;
//   void (*fn)(void* arg);
//   void* arg;
//   void* stack_mem;
//   size_t stack_size;
//   int finished;
// };

// thread_local Coro  g_thread_ctx;
// thread_local Coro *g_current;

// __attribute__((naked)) void coro_return_to_caller(CoroCtx* from, CoroCtx* to) {
//   asm volatile(R"(
//     push %rbp
//     push %rbx
//     push %r12
//     push %r13
//     push %r14
//     push %r15

//     mov %rsp, (%rdi)
//     mov (%rsi), %rsp

//     pop %r15
//     pop %r14
//     pop %r13
//     pop %r12
//     pop %rbx
//     pop %rbp

//     ret
//   )");
// }

// __attribute__((naked)) void coro_switch(CoroCtx* from, CoroCtx* to) {
//   asm volatile(R"(
//     push %rbp
//     push %rbx
//     push %r12
//     push %r13
//     push %r14
//     push %r15

//     mov %rsp, (%rdi)
//     mov (%rsi), %rsp

//     pop %r15
//     pop %r14
//     pop %r13
//     pop %r12
//     pop %rbx
//     pop %rbp

//     ret
//   )");
// }

// [[noreturn]] __attribute__((naked)) void coro_trampoline() {
//   asm volatile(R"(
//     mov %r12, %rdi
//     call coro_start
//   )");
// }

// Coro *coro_make(void (*fn)(void *arg), void *arg, size_t stack_size) {
//   Coro *co = (Coro*)calloc(1, sizeof(Coro));
//   co->fn         = fn;
//   co->arg        = arg;
//   co->stack_size = stack_size;
//   co->stack_mem  = malloc(stack_size);

//   uint8_t *sp = (uint8_t *)co->stack_mem + stack_size;
//   sp = (uint8_t *)((uintptr_t)sp & ~(uintptr_t)0xF); /* 16-byte align */

//   /*
//     * Hand-build a stack frame that looks exactly like what coro_switch()
//     * would find if this coroutine had been running and just pushed its
//     * six callee-saved registers. Laid out high -> low address, matching
//     * push order rbp,rbx,r12,r13,r14,r15 (so pop order r15..rbp restores
//     * correctly), with the trampoline's address sitting where `ret`
//     * expects a return address:
//     *
//     *   [ coro_trampoline ]   <- "return address"
//     *   [ rbp = 0         ]
//     *   [ rbx = 0         ]
//     *   [ r12 = co        ]   <- trampoline reads this as its argument
//     *   [ r13 = 0         ]
//     *   [ r14 = 0         ]
//     *   [ r15 = 0         ]   <- co->ctx.rsp points here
//     */
//   sp -= sizeof(void *); *(void **)sp = (void *)coro_trampoline;
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* rbp */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* rbx */
//   sp -= sizeof(void *); *(void **)sp = (void *)co; /* r12 -> self */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r13 */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r14 */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r15 */

//   co->ctx.stack_pointer = sp;
//   return co;
// }

// void coro_restart(Coro *co) {
//   co->finished = 0;

//   uint8_t* sp = (uint8_t*)co->stack_mem + co->stack_size;

//   sp = (uint8_t*)((uintptr_t)sp & ~(uintptr_t)0xF);

//   sp -= sizeof(void *); *(void **)sp = (void *)coro_trampoline;
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* rbp */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* rbx */
//   sp -= sizeof(void *); *(void **)sp = (void *)co; /* r12 -> self */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r13 */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r14 */
//   sp -= sizeof(void *); *(void **)sp = NULL;       /* r15 */

//   co->ctx.stack_pointer = sp;
// }

// void coro_destroy(Coro *co) {
//   if(!co) return;
//   free(co->stack_mem);
//   free(co);
// }

// Coro *coro_current(void) {
//   if(!g_current) g_current = &g_thread_ctx;
//   return g_current;
// }

// void coro_resume(Coro *co) {
//   co->caller = coro_current();
//   g_current  = co;
//   coro_switch(&co->caller->ctx, &co->ctx);
//   // Coro *prev = coro_current();

//   //   co->caller = prev;
//   //   g_current = co;

//   //   coro_switch(&prev->ctx, &co->ctx);
//   /* control returns here once `co` yields or finishes */
// }

// void coro_yield(void) {
//   Coro* co = coro_current();
//   g_current = co->caller;
//   coro_switch(&co->ctx, &co->caller->ctx);
//   /* control returns here once we're resumed again */
// }

// /* Called once, from coro_trampoline, the first time a coroutine runs. */
// C_LINKAGE void coro_start(Coro *co) {
//   co->fn(co->arg);
//   g_current = co->caller;
//   coro_switch(&co->ctx, &co->caller->ctx);
//   AssertAlways(0);
//   __builtin_unreachable(); /* a finished coroutine is never resumed again */
// }

// void example(void* ctx) {
//   u32* i = (u32*)ctx;
//   Loop(c, *i) {
//     // Info("from coroutine %i", c);
//     printf("example %i\n", *i);
//     coro_yield();
//   }
//   u32 a = 1;
// }

// void foo() {
//   u32 i = 2;
//   Coro* c = coro_make(example, &i, KB(8));
//   Info("start example");
//   Loop(i, 10) {
//     Info("From outside %i", i);
//     coro_restart(c);
//     coro_resume(c);
//   }

//   u32 a = 1;
// }



// template<i32 N> struct BitSetIter1 {
//   BitSetS<N>* bits;
//   u64 word_i;
//   u64 word;
//   operator bool() {
//     while(word == 0) {
//       if(word_i >= bitset_word_count(*bits))
//         return false;
//       word = bits->words[word_i];
//       ++word_i;
//     }
//     // u64 bit = ctz(word);
//     u64 word_i = word_i - 1;
//     // u64 idx = word_i * 64 + (64-bit);
//     word = remove_lowest_bit(word);
//     return true;
//   }
//   BitSetIter1& operator++() {}
//   // T& operator*() {
//   // }
// };

// template<i32 N> BitSetIter<N> bit_iter_begin(BitSetS<N>* bits) {
//   BitSetIter res = {
//     .bits = bits,
//   };
//   return res;
// }

// template<i32 N> b32 bit_iter_next(BitSetIter<N>* it, ThingId* out) {
//   while(it->word == 0) {
//     if(it->word_i >= bitset_word_count(*it->bits))
//       return false;
//     it->word = it->bits->words[it->word_i];
//     ++it->word_i;
//   }
//   u64 bit = ctz(it->word);
//   u64 word_i = it->word_i - 1;
//   u64 idx = word_i * 64 + (64-bit);
//   it->word = remove_lowest_bit(it->word);
//   *out = pool_get_handle(st->things, idx);
//   return true;
// }

// typedef u32 EntityId;
// struct Entity {
//   u32 slot;
//   // ...
// };

// Array<Entity, 1024> entities;
// Array<EntityId, 128> specific_entities;

// void push_to_specific(EntityId id) {
//   Entity* e = &entities[id];
//   u32 idx = specific_entities.count;
//   array_push(specific_entities, id);
//   e->slot = idx;
// }
// void remove_from_specific(EntityId id) {
//   Entity* e = &entities[id];

//   u32 idx = e->slot;
//   u32 last = specific_entities.count - 1;
//   EntityId moved = specific_entities[last];

//   specific_entities[idx] = moved;
//   --specific_entities.count;

//   entities[moved].slot = idx;
// }

// template <typename T> f64 benchmark(T func, u32 iterations) {
//   u64 start = cpu_timer_now();

//   Loop(i, iterations) {
//     func();
//   }

//   return tsc_to_ms(cpu_timer_now() - start);
// }


// struct Node {
//   ThingId id;
//   u32 next;
//   u32 prev;
// };

// struct List {
//   u32 first;
//   u32 last;
// };

// Poolu32<Node, 128> pool;

// void list_push(List& l, ThingId id) {
//   u32 idx = pool_push(pool, {id});
//   idll_list_push_back(pool.data, l, idx);
// }
// void list_remove(List& l, u32 idx) {
//   idll_list_remove(pool.data, l, idx);
//   pool_remove(pool, idx);
// }

// void foo() {
//   List x_list = {};
//   // list_push(pool, x_list, ThingId{1});
//   list_push(x_list, ThingId{1});
//   list_push(x_list, ThingId{2});
//   list_push(x_list, ThingId{3});

//   {
//     Scratch scratch;
//     struct Node {
//       u32 i;
//       Node* next;
//       Node* prev;
//     };
//     struct List {
//       Node* first;
//       Node* last;
//     };
//     List list = {};
//     Loop(i, 10) {
//       Node* n = push_struct(scratch, Node);
//       n->i = i;
//       dll_list_push_back(list, n);
//     }
//     LoopNode (it, list.first) {
//       Info("%i", it->i);
//     }
//   }

//   LoopIter (it, pool_begin(pool, x_list.first)) {
//     Info("%i", (*it).id);
//   }
//   list_remove(x_list, x_list.first);
//   LoopIter (it, pool_begin(pool, x_list.first)) {
//     Info("%i", (*it).id);
//   }

//   // LoopINode (it, x_list.first, pool.data) {
//   //   Info("%i", it);
//   // }

//   // list_push<Node, 128, List>(pool, x_list, id);
//   // list_push(pool, x_list, ThingId{1});
//   // list_push(pool, x_list, {2});
//   // list_push(pool, x_list, {3});
// }

v2::v2(const v2u& v):x(v.x),y(v.y){}

f32 Sin(f32 x)                         { return __builtin_sinf(x); }
f32 Cos(f32 x)                         { return __builtin_cosf(x); }
f32 Tan(f32 x)                         { return __builtin_tanf(x); }
f32 Asin(f32 x)                        { return __builtin_asinf(x); }
f32 Acos(f32 x)                        { return __builtin_acosf(x); }
f32 Atan2(f32 y, f32 x)                { return __builtin_atan2f(y,x); }
f32 Sqrt(f32 x)                        { return __builtin_sqrtf(x); }
f32 Pow(f32 a, f32 b)                  { return __builtin_powf(a, b); }
f32 Floor(f32 x)                       { return __builtin_floorf(x); }
f32 Ceil(f32 x)                        { return __builtin_ceilf(x); }
f32 Round(f32 x)                       { return __builtin_roundf(x); }
f32 Mod(f32 a, f32 b)                  { return __builtin_fmodf(a, b); }
f32 Trunc(f32 x)																							{ return __builtin_truncf(x); }
f32 Exp(f32 x)                         { return __builtin_expf(x); }
f32 Loge(f32 x)                        { return __builtin_logf(x); }
f32 Log2(f32 x)                        { return __builtin_log2f(x); }
f32 Log10(f32 x)                       { return __builtin_log10f(x); }
void Sincos(f32 rad, f32* s, f32* c)   { __builtin_sincosf(rad, s, c); }

f32 sind(f32 x) { return Sin(deg2rad(x)); }
f32 cosd(f32 x) { return Cos(deg2rad(x)); }

f32 deg2rad(f32 degrees) { return degrees * PI / 180.0f; }
f32 rad2deg(f32 radians) { return radians * 180.0f / PI; }
f32 ceil_to(f32 x, f32 step)  { return Ceil(x / step) * step; }
f32 floor_to(f32 x, f32 step) { return Floor(x / step) * step; }
f32 round_to(f32 x, f32 step) { return Round(x / step) * step; }

f64 mod_trunc(f64 x, f64 m) { return x - Trunc(x / m) * m; };
f32 mod_floor(f32 x, f32 m) { return x - Floor(x / m) * m; }
f32 frac(f32 x) { return x - Floor(x); }

////////////////////////////////////////////////////////////////////////
// Color
v4 rgba_from_u32(u32 hex) {
	v4 res = v4(
		((hex >> 0) & 0xff) / 255.0f,
		((hex >> 8) & 0xff) / 255.0f,
		((hex >> 16) & 0xff) / 255.0f,
		((hex >> 24) & 0xff) / 255.0f);
	return res;
}
u32 u32_from_rgba(v4 rgba) {
	u32 res =
		((u32)((u8)(rgba.x * 255.0f))) << 0 |
		((u32)((u8)(rgba.y * 255.0f))) << 8 |
		((u32)((u8)(rgba.z * 255.0f))) << 16 |
		((u32)((u8)(rgba.w * 255.0f))) << 24;
	return res;
}

////////////////////////////////////////////////////////////////////////
// Hash

#define SplitmixK 0x9e3779b97f4a7c15ull
u64 mix64(u64 x) {
	x ^= x >> 30;
	x *= 0xbf58476d1ce4e5b9;
	x ^= x >> 27;
	x *= 0x94d049bb133111eb;
	x ^= x >> 31;
	return x;
}
u64 wymix(u64 a, u64 b) {
	u128 r = u128(a) * b;
	return u64(r) ^ u64(r >> 64);
}

const u64 wyp0 = 0x2d358dccaa6c78a5ull, wyp1 = 0x8bb84b93962eacc9ull, wyp2 = 0x4b33a62ed433d4a3ull, wyp3 = 0x4d5a2da51de1aa47ull;
u64 wyhash2(u64 a, u64 b) {
	a^=wyp0;
	b^=wyp1;
	u128 r = u128(a)*b;
	return wymix(u64(r)^wyp0, u64(r >> 64)^wyp1);
}
u64 wyhash3(u64 a, u64 b, u64 c) {
	a ^= wyp0;
	b ^= wyp1;
	c ^= wyp2;
	u64 t = wymix(a, b);
	u64 r = wymix(t, c);
	return r;
}
u64 wyhash4(u64 a, u64 b, u64 c, u64 d) {
	a ^= wyp0;
	b ^= wyp1;
	c ^= wyp2;
	d ^= wyp3;
	u64 t1 = wymix(a, b);
	u64 t2 = wymix(c, d);
	u64 r = wymix(t1, t2);
	return r;
}

// wyhash (https://github.com/wangyi-fudan/wyhash)
u64 wyr8(u8* p) { u64 v; MemCopy(&v, p, 8); return v; }
u64 wyr4(u8* p) { u32 v; MemCopy(&v, p, 4); return v; }
u64 wyr3(u8* p, u64 k) { return (u64(p[0]) << 16) | (u64(p[k >> 1]) << 8) | p[k - 1]; }
u64 hash_bytes(void* data, u64 size, u64 seed) {
	u8* p = (u8*)data;
	seed ^= wymix(seed ^ wyp0, wyp1);
	u64 a, b;
	if(size <= 16) {
		if(size >= 4) {
			a = (wyr4(p) << 32) | wyr4(p + ((size >> 3) << 2));
			b = (wyr4(p + size - 4) << 32) | wyr4(p + size - 4 - ((size >> 3) << 2));
		} else if(size > 0) {
			a = wyr3(p, size);
			b = 0;
		} else {
			a = b = 0;
		}
	} else {
		u64 i = size;
		if(i > 48) {
			u64 see1 = seed, see2 = seed;
			do {
				seed = wymix(wyr8(p) ^ wyp1, wyr8(p + 8) ^ seed);
				see1 = wymix(wyr8(p + 16) ^ wyp2, wyr8(p + 24) ^ see1);
				see2 = wymix(wyr8(p + 32) ^ wyp3, wyr8(p + 40) ^ see2);
				p += 48;
				i -= 48;
			}while(i > 48);
			seed ^= see1 ^ see2;
		}
		while(i > 16) {
			seed = wymix(wyr8(p) ^ wyp1, wyr8(p + 8) ^ seed);
			i -= 16;
			p += 16;
		}
		a = wyr8(p + i - 16);
		b = wyr8(p + i - 8);
	}
	a ^= wyp1;
	b ^= seed;
	u128 r = u128(a) * b;
	return wymix(u64(r) ^ wyp0 ^ size, u64(r >> 64) ^ wyp1);
}

u64 hash(u64 x) 																					{ return mix64(x+1); }
u64 hash(u64 a, u64 b)															{ return wyhash2(a,b); }
u64 hash(u64 a, u64 b, u64 c)								{ return wyhash3(a,b,c); }
u64 hash(u64 a, u64 b, u64 c, u64 d) { return wyhash4(a,b,c,d); }
u64 hash(String x, u64 seed) 								{ return hash_bytes(x.str, x.size, seed); }

////////////////////////////////////////////////////////////////////////
// Random
u64 wyrand(u64* seed) {
	*seed += 0x2d358dccaa6c78a5ull;
	return wymix(*seed, *seed^0x8bb84b93962eacc9ull);
}
u64 wy2u0k(u64 r, u64 k) {
	return (u128(r) * k) >> 64;
}

u64 rand_u64()																														{ return wyrand(&_seed); }
u64 rand_u64(u64 max) 																						{ return wy2u0k(rand_u64(), max); }
u32 rand_u32()                              { return rand_u64(); }
u32 rand_u32(u32 max)                       { return rand_u64(max); }
u32 rand_u32(u32 min, u32 max)              { return rand_u32(max - min) + min; }
i32 rand_i32()                              { return rand_u32(); }
i32 rand_i32(i32 min, i32 max)              { return rand_u32(min, max); }
f32 rand_f32() 																										   { return (rand_u64() >> 40) * 0x1.0p-24f; }
f32 rand_f32_11()                           { return rand_f32() * 2.0f - 1.0f; }
f32 rand_f32(f32 max)																							{ return rand_f32() * max; }
f32 rand_f32(f32 min, f32 max)          				{ return rand_f32() * (max - min) + min ; }
f32 rand_f32_centered(f32 base, f32 radius) { return rand_f32(base-radius, base+radius); }
f32 rand_f32_signed(f32 magnitude)          { return rand_f32(-magnitude, magnitude); }
b32 rand_b32()                              { return rand_u64() & 1; }
void rand_seed(u64 seed)                				{ _seed = seed; }
v2 rand_in_disk(f32 r, f32 rad) {
	r = Sqrt(r);
	f32 a = 2 * PI * rad;
	f32 s,c; Sincos(a, &s, &c);
	return {r * c, r * s};
}
v3 rand_on_sphere(f32 r, f32 rad) {
	f32 z = 1 - 2 * r;
	r = Sqrt(1 - z * z);
	f32 a = 2 * PI * rad;
	f32 s,c; Sincos(a, &s, &c);
	return {r * c, r * s, z};
}

////////////////////////////////////////////////////////////////////////
// Misc

f32 safe_divn(f32 a, f32 b, f32 n) { return b == 0 ? n : a/b; }
f32 safe_div0(f32 a, f32 b)        { return safe_divn(a,b,0); }
f32 safe_div1(f32 a, f32 b)        { return safe_divn(a,b,1); }
i32 wrap_i32(i32 min, i32 x, i32 max) {
	i32 res = (x - min) % (max - min);
	return res + min;
}
f32 wrap_f32(f32 min, f32 x, f32 max) {
	f32 res = x - (max - min)*Floor(unlerp(min, x, max));
	return res;
}
f32 lerp(f32 a, f32 t, f32 b)       { return t*(b - a) + a; }
f64 lerp_f64(f64 a, f64 t, f64 b)   { return t*(b - a) + a; }
f32 lerp_clamp(f32 a, f32 t, f32 b) { return lerp(a, Clamp01(t), b); }
f32 unlerp(f32 a, f32 x, f32 b)     { return (x - a) / (b - a); }
f64 unlerp_f64(f64 a, f64 x, f64 b) { return (x - a) / (b - a); }
f32 remap(f32 x, f32 from_min, f32 from_max, f32 to_min, f32 to_max) { return lerp(to_min, unlerp(from_min, x, from_max), to_max); }
f32 remap(f32 x, f32 from_max, f32 to_max)                           { return x / from_max * to_max; }
f64 remap_f64(f64 x, f64 from_min, f64 from_max, f64 to_min, f64 to_max) { return lerp_f64(to_min, unlerp_f64(from_min, x, from_max), to_max); }
f64 remap_f64(f64 x, f64 from_max, f64 to_max)																											{ return x / from_max * to_max; }
f32 remap_clamp(f32 x, f32 from_min, f32 from_max, f32 to_min, f32 to_max) {
	return remap(Clamp(from_min, x, from_max), from_min, from_max, to_min, to_max);
}
f32 approach(f32 from, f32 to, f32 step) {
	f32 d = to - from;
	if(Abs(d) <= step) {
		return to;
	}
	return from + Sign(d)*step;
}

f32 exp_decay(f32 x, f32 target, f32 decay, f32 dt) {
	return target + (x - target) * Exp(-decay * dt);
}

f32 wrap_2pi(f32 a) {
	a -= Tau * Floor(a / Tau);
	return a;
}

f32 wrap_pi(f32 a) {
	a -= Tau * Floor((a + PI) / Tau);
	return a;
}

f32 angle_delta(f32 from, f32 to)     { return wrap_pi(to - from); }
f32 angle_delta_cw(f32 from, f32 to)  { return wrap_2pi(to - from); }
f32 angle_delta_ccw(f32 from, f32 to) { return wrap_2pi(from - to); }
f32 angle_delta_longest(f32 from, f32 to) {
	f32 d = wrap_pi(to - from);
	return d > 0 ? (d - 2*PI) : (d + 2*PI);
}
f32 angle_approach(f32 from, f32 to, f32 step) {
	f32 d = angle_delta(from, to);
	if(Abs(d) <= step)
		return to;
	return from + Sign(d)*step;
}

Polar2 cartesian_to_polar(v2 p) {
	Polar2 res = {
		v2_len(p),
		Atan2(p.y, p.x),
	};
	return res;
}
v2 polar_to_cartesian(Polar2 p) {
	f32 s,c; Sincos(p.theta, &s, &c);
	v2 res = {
		p.r * c,
		p.r * s,
	};
	return res;
}

v2 orbit_pos(Orbit2 o) {
	f32 s,c; Sincos(o.angle, &s,&c);
	return o.center + v2{c, s} * o.radius;
}
v3 orbit_pos(Orbit3 o) {
	f32 s,c; Sincos(o.angle, &s,&c);
	return o.center + (o.u * c + o.v * s) * o.radius;
}

v3 spherical_coord(f32 r, f32 theta, f32 phi) {
	f32 phi_s,phi_c; Sincos(phi, &phi_s,&phi_c);
	f32 theta_s,theta_c; Sincos(theta, &theta_s,&theta_c);
	v3 res = {
		r * phi_c * theta_s,
		r * phi_s,
		r * phi_c * theta_c,
	};
	return res;
}
v3 cylindrical_coord(f32 r, f32 theta, f32 z) {
	f32 s,c; Sincos(theta, &s,&c);
	v3 res = {
		r * c,
		r * s,
		z,
	};
	return res;
}

void plane_basis(v3 n, v3* u, v3* v) {
	n = v3_norm(n);
	v3 helper = Abs(n.y) < 0.99f ? v3{0, 1, 0} : v3{1, 0, 0}; // avoid parallel
	*u = v3_norm(v3_cross(helper, n));
	*v = v3_cross(n, *u);
}
v3 best_fit_norm(Slice<v3> v) {
	v3 res = {};
	v3* p = &v[v.count - 1];
	Loop(i, v.count) {
		v3* c = &v[i];
		res.x += (p->z + c->z) * (p->y - c->y);
		res.y += (p->x + c->x) * (p->z - c->z);
		res.z += (p->y + c->y) * (p->x - c->x);
		p = c;
	}
	return v3_norm(res);
}
f32 point_plane_signed_dist(v3 p, v3 n, f32 d) {
	return v3_dot(p, n) - d;
}

f32 v2_falloff(v2 p, v2 center, f32 radius, f32 max) {
	f32 r = v2_distance(p, center);
	if(r >= radius) return 0;
	return max * (1 - r / radius);
}
f32 v3_falloff(v3 p, v3 center, f32 radius, f32 max) {
	f32 r = v3_distance(p, center);
	if(r >= radius) return 0;
	return max * (1 - r / radius);
}

v3 v3_bezier2(v3 p0, v3 p1, v3 p2, f32 t) {
	v3 a = v3_lerp(p0, t, p1);
	v3 b = v3_lerp(p1, t, p2);
	return v3_lerp(a, t, b);
}
v3 v3_bezier3(v3 p0, v3 p1, v3 p2, v3 p3, f32 t) {
	v3 a = v3_lerp(p0, t, p1);
	v3 b = v3_lerp(p1, t, p2);
	v3 c = v3_lerp(p2, t, p3);
	v3 d = v3_lerp(a, t, b);
	v3 e = v3_lerp(b, t, c);
	return v3_lerp(d, t, e);
}

// v2 ndc_to_screen(v2 ndc) {
// 	v2 res = {
// 		(ndc.x/2 + 0.5f) * os_win_width(),
// 		(ndc.y/2 + 0.5f) * os_win_height()
// 	};
// 	return res;
// }
// v2 screen_to_ndc(v2 p) {
// 	v2 res = {
// 		p.x / os_win_width() * 2 - 1,
// 		p.y / os_win_height() * 2 - 1,
// 	};
// 	return res;
// }
v2 ndc_to_screen(v2 ndc) {
	v2 res = {
		(ndc.x*0.5f + 0.5f) * os_win_width(),
		(0.5f - ndc.y * 0.5f) * os_win_height()
	};
	return res;
}
v2 screen_to_ndc(v2 p) {
	v2 res = {
		p.x / os_win_width() * 2 - 1,
		1 - p.y / os_win_height() * 2.0f
	};
	return res;
}
b32 world_to_screen(m4x4 proj_view, v3 p, v2* out, f32* out_depth) {
	v4 clip = proj_view * v4(p, 1);
	if(clip.w <= 0) return false;                    // behind camera (w = -z_view)
	v3 ndc = clip.xyz / clip.w;
	if(ndc.z < 0 || ndc.z > 1) return false;         // outside near/far
	*out = ndc_to_screen(ndc.xy);
	if(out_depth) *out_depth = ndc.z;
	return true;                                      // may still be off-screen; test against viewport if needed
}
v3 screen_to_world(m4x4 inv_view_proj, v2 p, f32 ndc_z) {
	v2 n = screen_to_ndc(p);
	v4 w = inv_view_proj * v4(n.x, n.y, ndc_z, 1);
	return w.xyz / w.w;
}
Ray screen_to_ray(m4x4 inv_view_proj, v3 pos, v2 p) {
	v3 far_p = screen_to_world(inv_view_proj, p, 1.0f);
	return {pos, v3_norm(far_p - pos)}; // perspective
}
b32 ray_plane(Ray r, v3 plane_p, v3 plane_n, v3* out_hit) {
	f32 denom = v3_dot(plane_n, r.dir);
	if(Abs(denom) < FloatEpsilon) return false; // parallel
	f32 t = v3_dot(plane_n, plane_p - r.pos) / denom;
	if(t < 0) return false; // behind the ray
	*out_hit = r.pos + r.dir * t;
	return true;
}
b32 screen_to_ground(m4x4 inv_view_proj, v3 pos, v2 p, f32 ground_y, v3* out_hit) {
	return ray_plane(screen_to_ray(inv_view_proj, pos, p), v3(0, ground_y, 0), v3(0, 1, 0), out_hit);
}

////////////////////////////////////////////////////////////////////////
// Vector2
v2  operator+(v2 a, v2 b)          { return v2(a.x + b.x, a.y + b.y); }
v2  operator-(v2 a, v2 b)          { return v2(a.x - b.x, a.y - b.y); }
v2  operator*(v2 v, f32 scalar)    { return v2(v.x*scalar, v.y*scalar); }
v2  operator*(f32 scalar, v2 v)    { return v2(v.x*scalar, v.y*scalar); }
v2  operator/(v2 v, f32 scalar)    { return v2(v.x/scalar, v.y/scalar); }
v2  operator+=(v2& a, v2 b)        { return a = a + b; }
v2  operator-=(v2& a, v2 b)        { return a = a - b; }
v2  operator*=(v2& v, f32 scalar)  { return v = v*scalar; }
v2  operator/=(v2& v, f32 scalar)  { return v = v/scalar; }
b32 operator==(v2 a, v2 b)         { return (Abs(a.x - b.x) <= FloatEpsilon) && (Abs(a.y - b.y) <= FloatEpsilon); }
b32 operator!=(v2 a, v2 b)         { return !(a == b); }
v2  operator-(v2 v)                { return v2(-v.x, -v.y); }

b32 operator==(v2u a, v2u b)       { return (a.x == b.x) && (a.y == b.y); }
b32 operator!=(v2u a, v2u b)       { return !(a == b); }

v2i v2i_of_v2(v2 v)                 { return v2i(v.x, v.y); }
v2  v2_of_v3(v3 v)                  { return v2(v.x, v.y); }
v2  v2_of_v4(v4 v)                  { return v2(v.x, v.y); }
v3  v2_to_v3(v2 v, f32 a)           { return v3(v.x, v.y, a); }
v4  v2_to_v4(v2 v, f32 a, f32 b)    { return v4(v.x, v.y, a, b); }
v2  v2_of_v2i(v2i v)                { return v2(v.x, v.y); }
v2  v2_of_v2u(v2u v)                { return v2(v.x, v.y); }
v2  v2_abs(v2 v)                    { return v2(Abs(v.x), Abs(v.y)); }
v2  v2_min(v2 a, v2 b)              { return v2(Min(a.x,b.x), Min(a.y,b.y)); }
v2  v2_max(v2 a, v2 b)              { return v2(Max(a.x,b.x), Max(a.y,b.y)); }
v2  v2_clamp(v2 min, v2 v, v2 max)  { return v2(Clamp(min.x, v.x, max.x), Clamp(min.y, v.y, max.y)); }
v2  v2_sign(v2 v)                   { return v2(Sign(v.x), Sign(v.y)) ; }
v2  v2_invert(v2 v)                 { return v2(1/v.x, 1/v.y); }
v2b v2_greater(v2 a, v2 b)          { return v2b(a.x>b.x, a.y>b.y); }
v2b v2_less(v2 a, v2 b)             { return v2b(a.x<b.x, a.y<b.y); }
b32 v2_greater_all(v2 a, v2 b)      { return a.x>b.x && a.y>b.y; }
b32 v2_less_all(v2 a, v2 b)         { return a.x<b.x && a.y<b.y; }
b32 v2_greater_any(v2 a, v2 b)      { return a.x>b.x || a.y>b.y; }
b32 v2_less_any(v2 a, v2 b)         { return a.x<b.x || a.y<b.y; }
v2  v2_rand(v2 a, v2 b)         				{ return v2(rand_f32(a.x, b.x), rand_f32(a.y, b.y)); }
v2  v2_swizzle(v2 v, u32 x, u32 y) 	{ return v2(v.v[x], v.v[y]); }
v3  v2_swizzle(v2 v, u32 x, u32 y, u32 z) { return v3(v.v[x], v.v[y], v.v[z]); }

f32 v2_len(v2 v)                             			{ return Sqrt(v2_len_sqr(v)); }
f32 v2_len_sqr(v2 v)                         			{ return Square(v.x) + Square(v.y); }
v2  v2_norm(v2 v)                               { return v * (1/v2_len(v)); }
f32 v2_distance(v2 a, v2 b)                     { return v2_len(a - b); }
f32 v2_distance_sqr(v2 a, v2 b)                 { return v2_len_sqr(a - b); }
f32 v2_dot(v2 a, v2 b)                          { return a.x*b.x + a.y*b.y; }
f32 v2_cross(v2 a, v2 b)                        { return a.x*b.y - a.y*b.x; }
v2  v2_lerp(v2 a, f32 t, v2 b)                  { return t*(b - a) + a; }
v2  v2_hadamard(v2 a, v2 b)                     { return v2(a.x*b.x, a.y*b.y); }
v2  v2_hadamard_div(v2 a, v2 b)                 { return v2(a.x/b.x, a.y/b.y); }
v2  v2_project(v2 v, v2 dir)                    { return v2_dot(v, dir) / v2_len_sqr(dir) * dir; }
v2  v2_project_on_unit(v2 v, v2 dir)            { return v2_dot(v, dir) * dir; }
f32 v2_project_len(v2 v, v2 dir)      			       { return v2_dot(v, v2_norm(dir)); }
f32 v2_project_len_on_unit(v2 v, v2 dir)        { return v2_dot(v, dir); }
v2  v2_reject(v2 v, v2 dir)                     { return v - v2_project(v, dir); }
v2  v2_reject_on_unit(v2 v, v2 dir)             { return v - v2_project_on_unit(v, dir); }
v2  v2_reflect(v2 v, v2 norm)                   { return v - 2*v2_project(v, norm); }
v2  v2_reflect_on_unit(v2 v, v2 norm)           { return v - 2*v2_project_on_unit(v, norm); }
f32 v2_angle(v2 a, v2 b)                        { return Atan2(v2_cross(a, b), v2_dot(a, b)); }
f32 v2_angle(v2 v)                              { return Atan2(v.y, v.x); }
v2  v2_from_angle(f32 rad)                      { f32 s,c; Sincos(rad,&s,&c); return v2(s, c); }

v2  v2_flip_y(v2 v)                             { return v2(v.x, -v.y); }
v2  v2_remap_01_to_11(v2 pos, v2 range)         { return v2(2 * (pos.x/range.x) - 1, 2 * (pos.y/range.y) - 1); }
f32 v2_line_angle(v2 start, v2 end)             { v2 dir = end - start; return Atan2(dir.y, dir.x); }
v2  v2_cw90(v2 v)                               { return v2(-v.y, v.x); }
v2  v2_ccw90(v2 v)                              { return v2(v.y, -v.x); }
v2  v2_rotate(v2 v, f32 sine, f32 cosine)       { return v2(v.x*cosine - v.y*sine, v.x*sine + v.y*cosine); }
v2  v2_rotate(v2 v, f32 rad)                    { f32 s, c; Sincos(rad, &s, &c); return v2_rotate(v, s, c);}
v2  v2_rotate_around(v2 v, v2 pivot, f32 rad)   { return v2_rotate(v - pivot, rad) + pivot; }

v2 v2_approach(v2 current, v2 target, f32 step) {
	if(v2_len_sqr(current - target) <= Square(step)) return target;
	return current + v2_norm(target - current) * step;
}
v2 v2_clamp_len(f32 min, v2 v, f32 max) {
	v2 res = v;
	f32 len = v2_len_sqr(v);
	if(len > 0) {
		len = Sqrt(len);
		f32 scale = 1;
		if(len < min) scale = min/len;
		else if(len > min) scale = max/len;
		res *= scale;
	}
	return res;
}
v2 v2_bounce(v2 v, v2 normal, f32 restitution) { return v - (1 + restitution) * v2_project_on_unit(v, normal); }

////////////////////////////////////////////////////////////////////////
// Vector3
v3  operator+(v3 a, v3 b)          { return v3(a.x + b.x, a.y + b.y, a.z + b.z); }
v3  operator-(v3 a, v3 b)          { return v3(a.x - b.x, a.y - b.y, a.z - b.z); }
v3  operator*(v3 v, f32 scalar)    { return v3(v.x*scalar, v.y*scalar, v.z*scalar); }
v3  operator*(f32 scalar, v3 v)    { return v3(v.x*scalar, v.y*scalar, v.z*scalar); }
v3  operator/(v3 v, f32 scalar)    { return v3(v.x/scalar, v.y/scalar, v.z/scalar); }
v3  operator+=(v3& a, v3 b)        { return a = a + b; }
v3  operator-=(v3& a, v3 b)        { return a = a - b; }
v3  operator*=(v3& v, f32 scalar)  { return v = v * scalar; }
v3  operator/=(v3& v, f32 scalar)  { return v = v / scalar; }
b32 operator==(v3 a, v3 b)         { return (Abs(a.x - b.x) <= FloatEpsilon) && (Abs(a.y - b.y) <= FloatEpsilon) && (Abs(a.z - b.z) <= FloatEpsilon); }
b32 operator!=(v3 a, v3 b)         { return !(a == b); }
v3  operator-(v3 v)                { return v3(-v.x, -v.y, -v.z); }

b32 operator==(v3u a, v3u b)       { return a.x == b.x && a.y == b.y && a.z == b.z; }

v3  v3_abs(v3 v)                    { return v3(Abs(v.x), Abs(v.y), Abs(v.z)); }
v3  v3_min(v3 a, v3 b)              { return v3(Min(a.x,b.x), Min(a.y,b.y), Min(a.z,b.z)); }
v3  v3_max(v3 a, v3 b)              { return v3(Max(a.x,b.x), Max(a.y,b.y), Max(a.z,b.z)); }
v3  v3_clamp(v3 min, v3 v, v3 max)  { return v3(Clamp(min.x, v.x, max.x), Clamp(min.y, v.y, max.y), Clamp(min.z, v.z, max.z)); }
v3  v3_sign(v3 v)                   { return v3(Sign(v.x), Sign(v.y), Sign(v.z)); }
v3  v3_invert(v3 v)                 { return v3(1/v.x, 1/v.y, 1/v.z); }
b32 v3_equal(v3 a, v3 b)            { return a.x == b.x && a.y == b.y && a.z == b.z; }
v3b v3_greater(v3 a, v3 b)          { return v3b(a.x>b.x, a.y>b.y, a.z>b.z); }
v3b v3_less(v3 a, v3 b)             { return v3b(a.x<b.x, a.y<b.y, a.z<b.z); }
b32 v3_greater_all(v3 a, v3 b)      { return a.x>b.x && a.y>b.y && a.z>b.z; }
b32 v3_less_all(v3 a, v3 b)         { return a.x<b.x && a.y<b.y && a.z<b.z; }
b32 v3_greater_any(v3 a, v3 b)      { return a.x>b.x || a.y>b.y || a.z>b.z; }
b32 v3_less_any(v3 a, v3 b)         { return a.x<b.x || a.y<b.y || a.z<b.z; }
v3  v3_rand(v3 a, v3 b)        				 { return v3(rand_f32(a.x, b.x), rand_f32(a.y, b.y), rand_f32(a.z, b.z)); }
v2  v3_swizzle(v3 v, u32 x, u32 y)  { return v2(v.v[x], v.v[y]); }
v3  v3_swizzle(v3 v, u32 x, u32 y, u32 z) { return v3(v.v[x], v.v[y], v.v[z]); }

f32 v3_len(v3 v)                  			{ return Sqrt(v3_len_sqr(v)); }
f32 v3_len_sqr(v3 v)              			{ return Square(v.x) + Square(v.y) + Square(v.z); }
v3  v3_norm(v3 v)                    { return v * (1.0f/v3_len(v)); }
f32 v3_distance(v3 a, v3 b)          { return v3_len(a - b); }
f32 v3_distance_sqr(v3 a, v3 b)      { return v3_len_sqr(a - b); }
f32 v3_dot(v3 a, v3 b)               { return a.x*b.x + a.y*b.y + a.z*b.z; }
v3  v3_cross(v3 a, v3 b)             { return v3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }
v3  v3_lerp(v3 a, f32 t, v3 b)       { return t*(b - a) + a; }
v3  v3_hadamard(v3 a, v3 b)          { return v3(a.x*b.x, a.y*b.y, a.z*b.z); }
v3  v3_hadamard_div(v3 a, v3 b)      { return v3(a.x/b.x, a.y/b.y, a.z/b.z); }
v3  v3_project(v3 v, v3 dir)         { return v3_dot(v, dir) / v3_len_sqr(dir) * dir; }
v3  v3_project_on_norm(v3 v, v3 dir) { return v3_dot(v, dir) * dir; }
f32 v3_len_projection(v3 a, v3 b) 			{ return v3_dot(a, v3_norm(b)); }
v3  v3_reject(v3 v, v3 dir)          { return v - v3_project(v, dir); }
v3  v3_reject_on_norm(v3 v, v3 dir)  { return v - v3_project_on_norm(v, dir); }
v3  v3_reflect(v3 v, v3 n)           { return v - 2*n*v3_dot(v, n); }
f32 v3_angle(v3 a, v3 b)             { return Atan2(v3_len(v3_cross(a, b)), v3_dot(a, b)); }
v3  v3_bounce(v3 vel, v3 n, f32 restitution) { return vel - v3_project_on_norm(vel, n) * (1.0f + restitution); }

v3 v3_approach(v3 current, v3 target, f32 step) {
	if(v3_len_sqr(current - target) <= Square(step)) return target;
	return current + v3_norm(target - current) * step;
}
v3 v3_clamp_len(f32 min, v3 v, f32 max) {
	v3 res = v;
	f32 len = v3_len_sqr(v);
	if(len > 0.0f) {
		len = Sqrt(len);
		f32 scale = 1;
		if(len < min) scale = min/len;
		else if(len > min) scale = max/len;
		res *= scale;
	}
	return res;
}

v3 v3_pos_of_mat4(m4x4 mat)        { return v3(mat.v[3][0], mat.v[3][1], mat.v[3][2]); };
v3 v3_rotate_x(v3 v, f32 rad)      { v2 r = v2_rotate(v2(v.y, v.z), rad); return v3(v.x, r.v[0], r.v[1]); }
v3 v3_rotate_y(v3 v, f32 rad)      { v2 r = v2_rotate(v2(v.x, v.z), rad); return v3(r.v[0], v.y, r.v[1]); }
v3 v3_rotate_z(v3 v, f32 rad)      { v2 r = v2_rotate(v2(v.x, v.y), rad); return v3(r.v[0], r.v[1], v.z); }
v3 v3_rotate_around_pivot(v3 pos, v3 pivot, v4 q) {
	v3 l = pos - pivot;
	v3 rotated = quat_rotate(q, l);
	return pivot + rotated;
}

v3 v3_barycentric(v3 v, v3 a, v3 b, v3 c) {
	v3 v0 = b-a;
	v3 v1 = c-a;
	v3 v2 = v-a;
	f32 d00 = v3_dot(v0,v0);
	f32 d01 = v3_dot(v0,v1);
	f32 d11 = v3_dot(v1,v1);
	f32 d20 = v3_dot(v2,v0);
	f32 d21 = v3_dot(v2,v1);
	f32 denom = d00 * d11 - d01 * d01;
	v3 res;
	res.y = (d11 * d20 - d01 * d21) / denom;
	res.z = (d00 * d21 - d01 * d20) / denom;
	res.x = 1.0f - (res.z + res.y);
	return res;
}
v3 v3_perpendicular(v3 v) {
	v3 cardinal_axis = v3(1,0,0);
	f32 min = Abs(v.x);
	if(Abs(v.y) < min) {
		min = Abs(v.y);
		cardinal_axis = v3(0,1,0);
	}
	if(Abs(v.z) < min) {
		min = Abs(v.z);
		cardinal_axis = v3(0,0,1);
	}
	return v3_cross(v, cardinal_axis);
}
v3 v3_rotate_around_axis(v3 v, v3 axis, f32 rad) {
	axis = v3_norm(axis);
	v3 proj = v3_project_on_norm(v, axis);
	v3 reject = v3_reject_on_norm(v, axis);
	v3 cross = v3_cross(axis, v);
	f32 sine;
	f32 cosine;
	Sincos(rad, &sine, &cosine);
	return proj + reject*cosine + cross*sine;
}
v3 v3_refract(v3 v, v3 n, f32 r) {
	v3 res = {};
	f32 v_norm_len = v3_dot(v, n); // v and n are normalized
	f32 v_tangent_len_sqr = 1 - Square(v_norm_len);
	f32 r_tangent_len_sqr = Square(r)*(v_tangent_len_sqr); // Snell's law: n1sin(theta1) = n2sin(theta2), r = n1/n2, v_tangent_len = sin(theta), r_tangent = r*v_tangent
	f32 r_norm_len_sqr = 1 - r_tangent_len_sqr;
	if(r_norm_len_sqr < 0) {
		return v3_reflect(v, n);
	}
	f32 r_norm_len = Sqrt(r_norm_len_sqr);
	res = r*v - (r*v_norm_len + r_norm_len)*n;
	return res;
}

////////////////////////////////////////////////////////////////////////
// Vector4
v4  operator+(v4 a, v4 b)          { return v4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w); }
v4  operator-(v4 a, v4 b)          { return v4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w); }
v4  operator*(v4 v, f32 scalar)    { return v4(v.x*scalar, v.y*scalar, v.z*scalar, v.w*scalar); }
v4  operator*(f32 scalar, v4 v)    { return v4(v.x*scalar, v.y*scalar, v.z*scalar, v.w*scalar); }
v4  operator/(v4 v, f32 scalar)    { return v4(v.x/scalar, v.y/scalar, v.z/scalar, v.w/scalar); }
v4  operator+=(v4& a, v4 b)        { return a = a + b; }
v4  operator-=(v4& a, v4 b)        { return a = a - b; }
v4  operator*=(v4& v, f32 scalar)  { return v = v * scalar; }
v4  operator/=(v4& v, f32 scalar)  { return v = v / scalar; }
b32 operator==(v4 a, v4 b)         { return (Abs(a.x - b.x) <= FloatEpsilon) && (Abs(a.y - b.y) <= FloatEpsilon) && (Abs(a.z - b.z) <= FloatEpsilon) && (Abs(a.w - b.w) <= FloatEpsilon); }
b32 operator!=(v4 a, v4 b)         { return !(a == b); }
v4  operator-(v4 v)                { return v4(-v.x, -v.y, -v.z, -v.w); }

v4 v4_abs(v4 v)                    { return v4(Abs(v.x), Abs(v.y), Abs(v.z), Abs(v.w)); }
v4 v4_set_w(v4 v, f32 w)           { return v4(v.x, v.y, v.x, w); }
v4 v4_min(v4 a, v4 b)              { return v4(Min(a.x,b.x), Min(a.y,b.y), Min(a.z,b.z), Min(a.w,b.w)); }
v4 v4_max(v4 a, v4 b)              { return v4(Max(a.x,b.x), Max(a.y,b.y), Max(a.z,b.z), Max(a.w,b.w)); }
v4 v4_invert(v4 v)                 { return v4(1.0f/v.x,1.0f/v.y,1.0f/v.z,1.0f/v.w); }
b32 v4_equal(v4 a, v4 b)           { return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w; }

f32 v4_len(v4 v)                			{ return Sqrt(v4_len_sqr(v)); }
f32 v4_len_sqr(v4 v)        							{ return Square(v.x) + Square(v.y) + Square(v.z) + Square(v.w); }
v4  v4_norm(v4 v)                  { return v * (1.0f / v4_len(v)); }
f32 v4_distance(v4 a, v4 b)        { return v4_len(a - b); }
f32 v4_distance_sqr(v4 a, v4 b)    { return v4_len_sqr(a - b); }
f32 v4_dot(v4 a, v4 b)             { return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w; }
v4  v4_lerp(v4 a, f32 t, v4 b)     { return t*(b - a) + a; }
v4  v4_hadamard(v4 a, v4 b)        { return v4(a.x*b.x, a.y*b.y, a.z*b.z, a.w*b.w); }

Transform transform_mul(Transform a, Transform b) {
	Transform r;
	r.scale = v3(a.scale.x*b.scale.x);
	r.rot = quat_mul(a.rot, b.rot);
	r.pos = quat_rotate(a.rot, b.pos * b.scale.x) + a.pos;
	return r;
}

v3 transform_point(Transform t, v3 p) {
	p *= t.scale.x;
	p = quat_rotate(t.rot, p);
	p += t.pos;
	return p;
}

v3 transform_inverse_point(Transform t, v3 p) {
	p -= t.pos;
	p = quat_rotate(quat_conjugate(t.rot), p);
	p /= t.scale.x;
	return p;
}

v3 transform_direction(Transform t, v3 v) {
	return quat_rotate(t.rot, v * t.scale.x);
}

v3 transform_direction(m4x4 m, v3 v) {
	v3 res = {
		m.v[0][0]*v.x + m.v[0][1]*v.y + m.v[0][2]*v.z,
		m.v[1][0]*v.x + m.v[1][1]*v.y + m.v[1][2]*v.z,
		m.v[2][0]*v.x + m.v[2][1]*v.y + m.v[2][2]*v.z,
	};
	return res;
}

////////////////////////////////////////////////////////////////////////
// Quatornion
v3 quat_forward(v4 q) { return quat_rotate(q, v3_forward); }
v3 quat_back(v4 q)			 { return quat_rotate(q, v3_back); }
v3 quat_right(v4 q)   { return quat_rotate(q, v3_right);   }
v3 quat_up(v4 q)      { return quat_rotate(q, v3_up);      }

v4 quat_conjugate(v4 q) { return v4(-q.x, -q.y, -q.z, q.w); }
v4 quat_inverse(v4 q)   { return v4_norm(quat_conjugate(q)); }

v4 quat_axis_angle(v3 axis, f32 rad) {
	axis = v3_norm(axis);
	f32 s, c;
	Sincos(rad / 2, &s, &c);
	v4 res = {axis.x*s, axis.y*s, axis.z*s, c};
	return res;
}

v3 quat_rotate(v4 q, v3 v) {
	v3 t = 2 * v3_cross(q.xyz, v);
	return v + q.w*t + v3_cross(q.xyz, t); 
}

v4 quat_mul(v4 a, v4 b) {
	f32 scalar = a.w*b.w - v3_dot(a.xyz, b.xyz);
	v3 v = a.w*b.xyz + b.w*a.xyz + v3_cross(a.xyz, b.xyz);
	v4 res = v4(v, scalar);
	return res;
}

f32 quat_angle(v4 a, v4 b) {
	f32 d = Abs(v4_dot(a,b));
	d = Clamp11(d);
	return Acos(d) * 2.0f;
}

v4 quat_slerp(v4 a, f32 t, v4 b) {
	f32 c = v4_dot(a, b);
	if(c < 0.0f) { b = -b; c = -c; }
	if(c > 0.9995f) {
		return v4_norm(v4_lerp(a, t, b));
	}
	f32 s = Sqrt(1.0f - c * c);
	f32 omega = Atan2(s, c);
	f32 inv_s = 1.0f / s;
	f32 k0 = Sin((1.0f - t) * omega) * inv_s;
	f32 k1 = Sin(t * omega) * inv_s;
	return a * k0 + b * k1;
}

v4 quat_nlerp(v4 a, f32 t, v4 b) {
	if(v4_dot(a, b) < 0) {
		b = -b;
	}
	v4 res = v4_norm(v4_lerp(a, t, b));
	return res;
}

v4 quat_rotate_towards(v4 a, v4 b, f32 max_rad) {
	f32 d = v4_dot(a, b);
	if(d < 0.f) {
		b.x = -b.x;
		b.y = -b.y;
		b.z = -b.z;
		b.w = -b.w;
		d = -d;
	}
	if(d > 0.9995f) return b;
	f32 angle = Acos(d) * 2.f;
	f32 t = (max_rad >= angle) ? 1.f : max_rad / angle;
	return quat_slerp(a, t, b);
}

v4 quat_from_to(v3 a, v3 b) {
	a = v3_norm(a);
	b = v3_norm(b);
	f32 dot = v3_dot(a, b);
	if(dot > 0.9999f)
		return quat_identity;
	if(dot < -0.9999f) {
		// we do this since v3_cross(a, -a) == v3() and quat will be invalid
		v3 axis = v3_cross(v3(1, 0, 0), a);
		if(v3_dot(axis, axis) < 0.0001f) {
			axis = v3_cross(v3(0, 1, 0), a);
		}
		axis = v3_norm(axis);
		return quat_axis_angle(axis, PI);
	}
	v3 c = v3_cross(a, b);
	v4 res = v4_norm(v4(c.x, c.y, c.z, 1.0f + dot));
	return res;
}

v4 quat_from_euler(v3 e) {
	f32 cy, cx, cz;
	f32 sy, sx, sz;
	Sincos(e.y/2, &sy, &cy);
	Sincos(e.x/2, &sx, &cx);
	Sincos(e.z/2, &sz, &cz);
	v4 qy = v4(0, sy, 0, cy);
	v4 qx = v4(sx, 0, 0, cx);
	v4 qz = v4(0, 0, sz, cz);
	v4 res = quat_mul(qz, quat_mul(qx, qy));
	return v4_norm(res);
}

v4 quat_look_rotation(v3 dir, v3 up) {
	v3 f = v3_norm(dir);
	v3 r = v3_norm(v3_cross(up, f));
	v3 u = v3_cross(f, r);
	f32 m00=r.x, m01=r.y, m02=r.z;
	f32 m10=u.x, m11=u.y, m12=u.z;
	f32 m20=f.x, m21=f.y, m22=f.z;
	f32 tr = m00+m11+m22;
	v4 q = {};
	if(tr > 0.f) {
		f32 s = 0.5f / Sqrt(tr+1.f);
		q.x = (m12 - m21) * s;
		q.y = (m20 - m02) * s;
		q.z = (m01 - m10) * s;
		q.w = 0.25f / s;
	} else if(m00 > m11 && m00 > m22) {
		f32 s = 2.f * Sqrt(1.f + m00 - m11 - m22);
		q.x = 0.25f * s;
		q.y = (m01 + m10) / s;
		q.z = (m20 + m02) / s;
		q.w = (m12 - m21) / s;
	} else if(m11 > m22) {
		f32 s = 2.f * Sqrt(1.f + m11 - m00 - m22);
		q.x = (m01 + m10) / s;
		q.y = 0.25f * s;
		q.z = (m12 + m21) / s;
		q.w = (m20 - m02) / s;
	} else {
		f32 s = 2.f * Sqrt(1.f + m22 - m00 - m11);
		q.x = (m20 + m02) / s;
		q.y = (m12 + m21) / s;
		q.z = 0.25f * s;
		q.w = (m01 - m10) / s;
	}
	return v4_norm(q);
}

v3 quat_rotate_point(v4 q, v3 point, v3 pivot) {
	v3 relative = point - pivot;
	v3 rotated = quat_rotate(q, relative);
	return pivot + rotated;
}

////////////////////////////////////////////////////////////////////////
// m2x2
m2x2 operator*(m2x2 a, m2x2 b) {
	m2x2 res = {};
	Loop(y, 2) {
		Loop(x, 2) {
			res.v[y][x] = a.v[0][x] * b.v[y][0] +
																	a.v[1][x] * b.v[y][1];
		}
	}
	return res;
}
m2x2& operator*=(m2x2& a, m2x2 b) { return a = b * a; }

v2 operator*(m2x2 mat, v2 v) {
	v2 res = {
		mat.v[0][0]*v.x + mat.v[1][0]*v.y,
		mat.v[0][1]*v.x + mat.v[1][1]*v.y,
	};
	return res;
}

m2x2 m2x2_identity() {
	m2x2 res = {
		1, 0,
		0, 1,
	};
	return res;
}

m2x2 m2x2_rotate(f32 rad) {
	f32 s, c;
	Sincos(rad, &s, &c);
	m2x2 res = {
		c, s,
	-s, c,
	};
	return res;
}

m2x2 m2x2_scale(v2 scale) {
	m2x2 res = {
		scale.x, 0,
		0,       scale.y,
	};
	return res;
}

m2x2 m2x2_inverse(m2x2 m) {
	f32 det =
		m.v[0][0] * m.v[1][1] -
		m.v[1][0] * m.v[0][1];
	f32 inv_det = 1.0f / det;
	m2x2 res = {
		m.v[1][1], -m.v[0][1],
	-m.v[1][0],  m.v[0][0],
	};
	return m2x2_scale_all_elements(res, inv_det);
}

m2x2 m2x2_scale_all_elements(m2x2 mat, f32 scale) {
	Loop(y, 2) {
		Loop(x, 2) {
			mat.v[y][x] *= scale;
		}
	}
	return mat;
}

////////////////////////////////////////////////////////////////////////
// m3x2
m3x2 operator*(m3x2 a, m3x2 b) {
	m3x2 res;
	Loop(y, 2) {
		Loop(x, 2) {
			res.v[y][x] = a.v[0][x] * b.v[y][0] +
																	a.v[1][x] * b.v[y][1];
		}
	}
	res.v[2][0] = a.v[0][0] * b.v[2][0] +
															a.v[1][0] * b.v[2][1] + 
															a.v[2][0];
	res.v[2][1] = a.v[0][1] * b.v[2][0] +
															a.v[1][1] * b.v[2][1] + 
															a.v[2][0];
	return res;
}
m3x2& operator*=(m3x2& a, m3x2 b) { return a = b * a; }

v2 operator*(m3x2 mat, v2 v) {
	v2 res = {
		mat.v[0][0] * v.x + mat.v[1][0] * v.y + mat.v[2][0],
		mat.v[0][1] * v.x + mat.v[1][1] * v.y + mat.v[2][1],
	};
	return res;
}

m3x2 m3x2_identity() {
	m3x2 res = {
		1, 0,
		0, 1,
		0, 0,
	};
	return res;
}

m3x2 m3x2_translate(v2 pos) {
	m3x2 res = {
		1, 0,
		0, 1,
		pos.x, pos.y
	};
	return res;
}

m3x2 m3x2_scale(v2 scale) {
	m3x2 res = {
		scale.x, 0,
		0,       scale.y,
		0,       0,
	};
	return res;
}

m3x2 m3x2_rotate(f32 rad) {
	f32 s, c;
	Sincos(rad, &s, &c);
	m3x2 res = {
		c, s,
	-s, c,
		0, 0,
	};
	return res;
}

m3x2 m3x2_transform(v2 pos, v2 scale, f32 rad) {
	f32 s, c;
	Sincos(rad, &s, &c);
	m3x2 res = {
		c*scale.x, s*scale.y,
	-s*scale.x, c*scale.y,
		pos.x,     pos.y,
	};
	return res;
}

m3x2 m3x2_inverse(m3x2 a) {
	f32 det = a.v[0][0] * a.v[1][1]
									- a.v[1][0] * a.v[0][1];
	f32 inv_det = 1.0f / det;
	m3x2 r;
	r.v[0][0] =  a.v[1][1] * inv_det;
	r.v[0][1] = -a.v[0][1] * inv_det;
	r.v[1][0] = -a.v[1][0] * inv_det;
	r.v[1][1] =  a.v[0][0] * inv_det;
	r.v[2][0] = -(r.v[0][0] * a.v[2][0] +
															r.v[1][0] * a.v[2][1]);
	r.v[2][1] = -(r.v[0][1] * a.v[2][0] +
															r.v[1][1] * a.v[2][1]);
	return r;
}

////////////////////////////////////////////////////////////////////////
// m3x3
m3x3 operator*(m3x3 a, m3x3 b) {
	m3x3 res = {};
	Loop(j, 3) {
		Loop(i, 3) {
			res.v[j][i] = b.v[j][0] * a.v[0][i] +
																	b.v[j][1] * a.v[1][i] +
																	b.v[j][2] * a.v[2][i];
		}
	}
	return res;
}
m3x3& operator*=(m3x3& a, m3x3 b) { return a = b * a; }

v3 operator*(m3x3 mat, v3 v) {
	v3 res = {
		mat.v[0][0]*v.x + mat.v[0][1]*v.y + mat.v[0][2]*v.z,
		mat.v[1][0]*v.x + mat.v[1][1]*v.y + mat.v[1][2]*v.z,
		mat.v[2][0]*v.x + mat.v[2][1]*v.y + mat.v[2][2]*v.z,
	};
	return res;
}

m3x3 m3x3_identity() {
	m3x3 res = {
		1,0,0,
		0,1,0,
		0,0,1
	};
	return res;
}

m3x3 m3x3_translate(v2 pos) {
	m3x3 res = {
		1,     0,     0,
		0,     1,     0,
		pos.x, pos.y, 1
	};
	return res;
}

m3x3 m3x3_scale(v2 scale) {
	m3x3 res = {
		scale.x, 0,       0,
		0,       scale.y, 0,
		0,       0,       1
	};
	return res;
}

m3x3 m3x3_rotate(f32 rad) {
	f32 s, c;
	Sincos(rad, &s, &c);
	m3x3 res = {
		c, s, 0,
	-s, c, 0,
		0, 0, 1,
	};
	return res;
}

m3x3 m3x3_inverse(m3x3 m) {
	f32 a = m.v[0][0];
	f32 b = m.v[1][0];
	f32 c = m.v[2][0];

	f32 d = m.v[0][1];
	f32 e = m.v[1][1];
	f32 f = m.v[2][1];

	f32 g = m.v[0][2];
	f32 h = m.v[1][2];
	f32 i = m.v[2][2];

	f32 A = e*i - f*h;
	f32 B = c*h - b*i;
	f32 C = b*f - c*e;

	f32 D = f*g - d*i;
	f32 E = a*i - c*g;
	f32 F = c*d - a*f;

	f32 G = d*h - e*g;
	f32 H = b*g - a*h;
	f32 I = a*e - b*d;

	f32 det = a*A + b*D + c*G;
	f32 inv_det = 1.0f / det;

	m3x3 res = {
		A, D, G,
		B, E, H,
		C, F, I,
	};

	return m3x3_scale_all_elements(res, inv_det);
}

m3x3 m3x3_scale_all_elements(m3x3 mat, f32 scale) {
	Loop(row, 3) {
		Loop(col, 3) {
			mat.v[row][col] *= scale;
		}
	}
	return mat;
}

////////////////////////////////////////////////////////////////////////
// m4x3
m4x3 operator*(m4x3 a, m4x3 b) {
	m4x3 res;
	Loop(y, 3) {
		Loop(x, 3) {
			res.v[y][x] = a.v[0][x] * b.v[y][0] +
																	a.v[1][x] * b.v[y][1] +
																	a.v[2][x] * b.v[y][2];
		}
	}
	res.v[3][0] = a.v[0][0] * b.v[3][0] +
															a.v[1][0] * b.v[3][1] + 
															a.v[2][0] * b.v[3][2] + 
															a.v[3][0];
	res.v[3][1] = a.v[0][1] * b.v[3][0] +
															a.v[1][1] * b.v[3][1] + 
															a.v[2][1] * b.v[3][2] + 
															a.v[3][1];
	res.v[3][2] = a.v[0][2] * b.v[3][0] +
															a.v[1][2] * b.v[3][1] + 
															a.v[2][2] * b.v[3][2] + 
															a.v[3][2];
	return res;
}
m4x3& operator*=(m4x3& a, m4x3 b) { return a = b * a; }

v3 operator*(m4x3 mat, v3 v) {
	v3 res = {
		mat.v[0][0]*v.x + mat.v[1][0]*v.y + mat.v[2][0]*v.z + mat.v[3][0],
		mat.v[0][1]*v.x + mat.v[1][1]*v.y + mat.v[2][1]*v.z + mat.v[3][1],
		mat.v[0][2]*v.x + mat.v[1][2]*v.y + mat.v[2][2]*v.z + mat.v[3][2],
	};
	return res;
}

m4x3 m4x3_identity() {
	m4x3 res = {
		1,0,0,
		0,1,0,
		0,0,1,
		0,0,0,
	};
	return res;
}

m4x3 m4x3_translate(v3 pos) {
	m4x3 res = {
		1,     0,     0,
		0,     1,     0,
		0,     0,     1,
		pos.x, pos.y, pos.z,
	};
	return res;
}

m4x3 m4x3_scale(v3 scale) {
	m4x3 res = {
		scale.x, 0,       0,
		0,       scale.y, 0,
		0,       0,       scale.z,
		0,       0,       0,
	};
	return res;
}

m4x3 m4x3_scale_translate(v3 scale, v3 pos) {
	m4x3 res = {
		scale.x, 0,       0,
		0,       scale.y, 0,
		0,       0,       scale.z,
		pos.x,   pos.y,   pos.z,
	};
	return res;
}

m4x3 m4x3_transform(v3 scale, v3 pos, v4 q) {
	m4x3 r = m4x3_from_quat(q);
	r.v[0][0] *= scale.x;
	r.v[0][1] *= scale.x;
	r.v[0][2] *= scale.x;
	r.v[1][0] *= scale.y;
	r.v[1][1] *= scale.y;
	r.v[1][2] *= scale.y;
	r.v[2][0] *= scale.z;
	r.v[2][1] *= scale.z;
	r.v[2][2] *= scale.z;
	r.v[3][0] = pos.x;
	r.v[3][1] = pos.y;
	r.v[3][2] = pos.z;
	return r;
}

m4x3 m4x3_from_quat(v4 q) {
	f32 x=q.x, y=q.y, z=q.z, w=q.w;
	f32 xx=x*x, yy=y*y, zz=z*z;
	f32 xy=x*y, xz=x*z, yz=y*z;
	f32 wx=w*x, wy=w*y, wz=w*z;
	m4x3 res = {
		1-2*(yy+zz), 2*(xy+wz),   2*(xz-wy),
		2*(xy-wz),   1-2*(xx+zz), 2*(yz+wx),
		2*(xz+wy),   2*(yz-wx),   1-2*(xx+yy),
		0,           0,           0,
	};
	return res;
}

////////////////////////////////////////////////////////////////////////
// m4x4
// NOTE: camera is looking at -z so forward is negative
v3 m4x4_view_forward(m4x4 matrix) {
	v3 res = {
		-matrix.v[0][2],
		-matrix.v[1][2],
		-matrix.v[2][2],
	};
	res = v3_norm(res);
	return res;
}

v3 m4x4_view_backward(m4x4 matrix) {
	v3 res = {
		matrix.v[0][2],
		matrix.v[1][2],
		matrix.v[2][2],
	};
	res = v3_norm(res);
	return res;
}

v3 m4x4_view_up(m4x4 matrix) {
	v3 up = {
		matrix.v[0][1],
		matrix.v[1][1],
		matrix.v[2][1],
	};
	up = v3_norm(up);
	return up;
}

v3 m4x4_view_down(m4x4 matrix) {
	v3 down = {
		-matrix.v[0][1],
		-matrix.v[1][1],
		-matrix.v[2][1],
	};
	down = v3_norm(down);
	return down;
}

v3 m4x4_view_right(m4x4 matrix) {
	v3 left = {
		matrix.v[0][0],
		matrix.v[1][0],
		matrix.v[2][0],
	};
	left = v3_norm(left);
	return left;
}

v3 m4x4_view_left(m4x4 matrix) {
	v3 right = {
		-matrix.v[0][0],
		-matrix.v[1][0],
		-matrix.v[2][0],
	};
	right = v3_norm(right);
	return right;
}

m4x4 operator*(m4x4 a, m4x4 b) {
	m4x4 res = {};
	Loop(y, 4) {
		Loop(x, 4) {
			res.v[y][x] = a.v[0][x] * b.v[y][0] +
																	a.v[1][x] * b.v[y][1] +
																	a.v[2][x] * b.v[y][2] +
																	a.v[3][x] * b.v[y][3];
		}
	}
	return res;
}
m4x4& operator*=(m4x4& a, m4x4 b) { return a = b * a; }

v4 operator*(m4x4 mat, v4 v) {
	v4 res = {
		mat.v[0][0]*v.x + mat.v[1][0]*v.y + mat.v[2][0]*v.z + mat.v[3][0]*v.w,
		mat.v[0][1]*v.x + mat.v[1][1]*v.y + mat.v[2][1]*v.z + mat.v[3][1]*v.w,
		mat.v[0][2]*v.x + mat.v[1][2]*v.y + mat.v[2][2]*v.z + mat.v[3][2]*v.w,
		mat.v[0][3]*v.x + mat.v[1][3]*v.y + mat.v[2][3]*v.z + mat.v[3][3]*v.w,
	};
	return res;
}

v3 operator*(m4x4 mat, v3 v) {
	v3 res = {
		mat.v[0][0]*v.x + mat.v[1][0]*v.y + mat.v[2][0]*v.z + mat.v[3][0],
		mat.v[0][1]*v.x + mat.v[1][1]*v.y + mat.v[2][1]*v.z + mat.v[3][1],
		mat.v[0][2]*v.x + mat.v[1][2]*v.y + mat.v[2][2]*v.z + mat.v[3][2],
	};
	return res;
}

m4x4 m4x4_translate(v3 pos) {
	m4x4 res = {
		1,     0,     0,     0,
		0,     1,     0,     0,
		0,     0,     1,     0,
		pos.x, pos.y, pos.z, 1
	};
	return res;
}

m4x4 m4x4_scale(v3 scale) {
	m4x4 res = {
		scale.x, 0,       0,       0,
		0,       scale.y, 0,       0,
		0,       0,       scale.z, 0,
		0,       0,       0,       1
	};
	return res;
}

m4x4 m4x4_scale_translate(v3 scale, v3 pos) {
	m4x4 res = {
		scale.x, 0,       0,       0,
		0,       scale.y, 0,       0,
		0,       0,       scale.z, 0,
		pos.x,   pos.y,   pos.z,   1
	};
	return res;
}

m4x4 m4x4_transform(v3 pos, v3 scale, v4 q) {
	m4x4 r = m4x4_from_quat(q);
	r.v[0][0] *= scale.x;
	r.v[0][1] *= scale.x;
	r.v[0][2] *= scale.x;
	r.v[1][0] *= scale.y;
	r.v[1][1] *= scale.y;
	r.v[1][2] *= scale.y;
	r.v[2][0] *= scale.z;
	r.v[2][1] *= scale.z;
	r.v[2][2] *= scale.z;
	r.v[3][0] = pos.x;
	r.v[3][1] = pos.y;
	r.v[3][2] = pos.z;
	r.v[3][3] = 1;
	return r;
}

m4x4 m4x4_transform(Transform trans) { return m4x4_transform(trans.pos, trans.scale, trans.rot); }

m4x4 m4x4_mul_scalar(m4x4 mat, f32 scale) {
	Loop(y, 4) {
		Loop(x, 4) {
			mat.v[y][x] *= scale;
		}
	}
	return mat;
}

m4x4 m4x4_rotate_x(f32 rad) {
	f32 sin, cos;
	Sincos(rad, &sin, &cos);
	m4x4 mat = {
		1,   0,    0,   0,
		0, cos,  sin,   0,
		0,-sin,  cos,   0,
		0,   0,    0,   1,
	};
	return mat;
}

m4x4 m4x4_rotate_y(f32 rad) {
	f32 sin, cos;
	Sincos(rad, &sin, &cos);
	m4x4 mat = {
		cos, 0,  -sin, 0,
		0,   1,   0,   0,
		sin, 0,   cos, 0,
		0,   0,   0,   1
	};
	return mat;
}

m4x4 m4x4_rotate_z(f32 rad) {
	f32 sin, cos;
	Sincos(rad, &sin, &cos);
	m4x4 mat = {
		cos, sin, 0,   0,
	-sin, cos, 0,   0,
		0,   0,   1,   0,
		0,   0,   0,   1
	};
	return mat;
}

m4x4 m4x4_rotate_around_axis(v3 axis, f32 rad) {
	axis = v3_norm(axis);
	f32 sine;
	f32 cosine;
	Sincos(rad, &sine, &cosine);
	f32 x = axis.x, y = axis.y, z = axis.z;
	f32 t = 1.0f - cosine;
	m4x4 res = {
		x*x*t + cosine, y*x*t + z*sine, z*x*t - y*sine, 0,
		x*y*t - z*sine, y*y*t + cosine, z*y*t + x*sine, 0,
		x*z*t + y*sine, y*z*t - x*sine, z*z*t + cosine, 0,
		0,              0,              0,              1,
	};
	return res;
}

m4x4 m4x4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
	// m4x4 res = {
	// 	2/(right-left),             0,                          0,                 0,
	// 	0,                          2/(top-bottom),             0,                 0,
	// 	0,                          0,                          1/(far-near),      0,
	// 	-(right+left)/(right-left), -(top+bottom)/(top-bottom), -near/(far-near),  1
	// };
	// NOTE: This flips Z value so camera is looking at -Z direction
	m4x4 res = {
		2/(right-left),             0,                          0,                0,
		0,                          2/(top-bottom),             0,                0,
		0,                          0,                          1/(near-far),     0,
		-(right+left)/(right-left), -(top+bottom)/(top-bottom), near/(near-far),  1
	};
	return res;
}

m4x4 m4x4_orthographic_inverse(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far) {
	m4x4 res = {
		(right-left)/2, 0,       						 	0,   					0,
		0,              (top-bottom)/2,  0,   					0,
		0,              0,       						 	near-far, 0,
		(right+left)/2, (top+bottom)/2, -near,  			1
	};
	return res;
}

// NOTE: I use camera looking at -Z direction since I found it's more intuitive
m4x4 m4x4_perspective(f32 fov_rad, f32 aspect, f32 near, f32 far) {
	f32 fov = Tan(fov_rad/2.0f);
	// m4x4 res = {
	// 	1/(fov*aspect), 0,     0,                      0,
	// 	0,              1/fov, 0,                      0,
	// 	0,              0,     far/(far-near),         1,
	// 	0,              0,     (-far*near)/(far-near), 0
	// };
	// NOTE: This flips Z value so camera is looking at -Z direction
	m4x4 res = {
		1/(fov*aspect), 0,     0,                      0,
		0,              1/fov, 0,                      0,
		0,              0,     far/(near-far),        -1,
		0,              0,     (far*near)/(near-far),  0
	};
	return res;
}

m4x4 m4x4_perspective_inverse(f32 fov_rad, f32 aspect, f32 near, f32 far) {
	f32 fov = Tan(fov_rad/2.0f);
	m4x4 res = {
		fov*aspect, 0,    0,  0,
		0,          fov,  0,  0,
		0,          0,    0,  (near-far)/(far*near),
		0,          0,   -1,  1/near
	};
	return res;
}

// NOTE: we negate Z direction since camera is looking at -Z direction
m4x4 m4x4_look_at(v3 pos, v3 target, v3 up) {
	v3 z = -v3_norm(target - pos);
	v3 x = v3_norm(v3_cross(up, z));
	v3 y = v3_cross(z, x);
	m4x4 res = {
		x.x, y.x, z.x, 0,
		x.y, y.y, z.y, 0,
		x.z, y.z, z.z, 0,
	-v3_dot(x, pos), -v3_dot(y, pos), -v3_dot(z, pos), 1
	};
	return res;
}

m4x4 m4x4_look_at_inverse(v3 pos, v3 target, v3 up) {
	v3 z = -v3_norm(target - pos);
	v3 x = v3_norm(v3_cross(up, z));
	v3 y = v3_cross(z, x);
	m4x4 res = {
		x.x,   x.y,   x.z,   0,
		y.x,   y.y,   y.z,   0,
		z.x,   z.y,   z.z,   0,
		pos.x, pos.y, pos.z, 1
	};
	return res;
}

m4x4 m4x4_transpose(m4x4 mat) {
	m4x4 res = {
		mat.v[0][0], mat.v[1][0], mat.v[2][0], mat.v[3][0],
		mat.v[0][1], mat.v[1][1], mat.v[2][1], mat.v[3][1],
		mat.v[0][2], mat.v[1][2], mat.v[2][2], mat.v[3][2],
		mat.v[0][3], mat.v[1][3], mat.v[2][3], mat.v[3][3],
	};
	return res;
}

m4x4 m4x4_inverse(m4x4 m) {
	f32 coef00 = m.v[2][2] * m.v[3][3] - m.v[3][2] * m.v[2][3];
	f32 coef02 = m.v[1][2] * m.v[3][3] - m.v[3][2] * m.v[1][3];
	f32 coef03 = m.v[1][2] * m.v[2][3] - m.v[2][2] * m.v[1][3];
	f32 coef04 = m.v[2][1] * m.v[3][3] - m.v[3][1] * m.v[2][3];
	f32 coef06 = m.v[1][1] * m.v[3][3] - m.v[3][1] * m.v[1][3];
	f32 coef07 = m.v[1][1] * m.v[2][3] - m.v[2][1] * m.v[1][3];
	f32 coef08 = m.v[2][1] * m.v[3][2] - m.v[3][1] * m.v[2][2];
	f32 coef10 = m.v[1][1] * m.v[3][2] - m.v[3][1] * m.v[1][2];
	f32 coef11 = m.v[1][1] * m.v[2][2] - m.v[2][1] * m.v[1][2];
	f32 coef12 = m.v[2][0] * m.v[3][3] - m.v[3][0] * m.v[2][3];
	f32 coef14 = m.v[1][0] * m.v[3][3] - m.v[3][0] * m.v[1][3];
	f32 coef15 = m.v[1][0] * m.v[2][3] - m.v[2][0] * m.v[1][3];
	f32 coef16 = m.v[2][0] * m.v[3][2] - m.v[3][0] * m.v[2][2];
	f32 coef18 = m.v[1][0] * m.v[3][2] - m.v[3][0] * m.v[1][2];
	f32 coef19 = m.v[1][0] * m.v[2][2] - m.v[2][0] * m.v[1][2];
	f32 coef20 = m.v[2][0] * m.v[3][1] - m.v[3][0] * m.v[2][1];
	f32 coef22 = m.v[1][0] * m.v[3][1] - m.v[3][0] * m.v[1][1];
	f32 coef23 = m.v[1][0] * m.v[2][1] - m.v[2][0] * m.v[1][1];
	
	v4 fac0 = { coef00, coef00, coef02, coef03 };
	v4 fac1 = { coef04, coef04, coef06, coef07 };
	v4 fac2 = { coef08, coef08, coef10, coef11 };
	v4 fac3 = { coef12, coef12, coef14, coef15 };
	v4 fac4 = { coef16, coef16, coef18, coef19 };
	v4 fac5 = { coef20, coef20, coef22, coef23 };
	
	v4 vec0 = { m.v[1][0], m.v[0][0], m.v[0][0], m.v[0][0] };
	v4 vec1 = { m.v[1][1], m.v[0][1], m.v[0][1], m.v[0][1] };
	v4 vec2 = { m.v[1][2], m.v[0][2], m.v[0][2], m.v[0][2] };
	v4 vec3 = { m.v[1][3], m.v[0][3], m.v[0][3], m.v[0][3] };
	
	v4 inv0 = (v4_hadamard(vec1, fac0) - v4_hadamard(vec2, fac1)) + v4_hadamard(vec3, fac2);
	v4 inv1 = (v4_hadamard(vec0, fac0) - v4_hadamard(vec2, fac3)) + v4_hadamard(vec3, fac4);
	v4 inv2 = (v4_hadamard(vec0, fac1) - v4_hadamard(vec1, fac3)) + v4_hadamard(vec3, fac5);
	v4 inv3 = (v4_hadamard(vec0, fac2) - v4_hadamard(vec1, fac4)) + v4_hadamard(vec2, fac5);
	
	v4 sign_a = { +1, -1, +1, -1 };
	v4 sign_b = { -1, +1, -1, +1 };
	
	m4x4 inverse;
	Loop(i, 4) {
		inverse.v[0][i] = inv0.v[i] * sign_a.v[i];
		inverse.v[1][i] = inv1.v[i] * sign_b.v[i];
		inverse.v[2][i] = inv2.v[i] * sign_a.v[i];
		inverse.v[3][i] = inv3.v[i] * sign_b.v[i];
	}
	
	v4 row0 = { inverse.v[0][0], inverse.v[1][0], inverse.v[2][0], inverse.v[3][0] };
	v4 m0 = { m.v[0][0], m.v[0][1], m.v[0][2], m.v[0][3] };
	v4 dot0 = v4_hadamard(m0, row0);
	f32 dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);
	
	f32 one_over_det = 1 / dot1;
	
	return m4x4_mul_scalar(inverse, one_over_det);
}

m4x4 m4x4_from_quat(v4 q) {
	f32 x=q.x, y=q.y, z=q.z, w=q.w;
	f32 xx=x*x, yy=y*y, zz=z*z;
	f32 xy=x*y, xz=x*z, yz=y*z;
	f32 wx=w*x, wy=w*y, wz=w*z;
	m4x4 res = {
		1-2*(yy+zz), 2*(xy+wz),   2*(xz-wy),   0,
		2*(xy-wz),   1-2*(xx+zz), 2*(yz+wx),   0,
		2*(xz+wy),   2*(yz-wx),   1-2*(xx+yy), 0,
		0,           0,           0,           1,
	};
	return res;
}

m4x4 m4x4_from_euler(v3 e) {
	f32 sx,cx; Sincos(e.x, &sx,&cx);
	f32 sy,cy; Sincos(e.y, &sy,&cy);
	f32 sz,cz; Sincos(e.z, &sz,&cz);
	m4x4 r = {};
	r.v[0][0] = cz * cy - sz * sx * sy;
	r.v[0][1] = -sz * cx;
	r.v[0][2] = cz * sy + sz * sx * cy;
	r.v[1][0] = sz * cy + cz * sx * sy;
	r.v[1][1] = cz * cx;
	r.v[1][2] = sz * sy - cz * sx * cy;
	r.v[2][0] = -cx * sy;
	r.v[2][1] = sx;
	r.v[2][2] = cx * cy;
	r.v[3][3] = 1.0f;
	return r;
}

m4x4 m4x4_from_m4x3(m4x3 m) {
	m4x4 res = {
		m.v[0][0], m.v[0][1], m.v[0][2], 0,
		m.v[1][0], m.v[1][1], m.v[1][2], 0,
		m.v[2][0], m.v[2][1], m.v[2][2], 0,
		m.v[3][0], m.v[3][1], m.v[3][2], 1,
	};
	return res;
}

m4x4 m4x4_inverse_rigid(m4x4 m) {
	m4x4 r;
	r.v[0][0] = m.v[0][0];
	r.v[0][1] = m.v[1][0];
	r.v[0][2] = m.v[2][0];

	r.v[1][0] = m.v[0][1];
	r.v[1][1] = m.v[1][1];
	r.v[1][2] = m.v[2][1];

	r.v[2][0] = m.v[0][2];
	r.v[2][1] = m.v[1][2];
	r.v[2][2] = m.v[2][2];

	r.v[3][0] = -(r.v[0][0]*m.v[3][0] + r.v[1][0]*m.v[3][1] + r.v[2][0]*m.v[3][2]);
	r.v[3][1] = -(r.v[0][1]*m.v[3][0] + r.v[1][1]*m.v[3][1] + r.v[2][1]*m.v[3][2]);
	r.v[3][2] = -(r.v[0][2]*m.v[3][0] + r.v[1][2]*m.v[3][1] + r.v[2][2]*m.v[3][2]);

	r.v[3][3] = 1;
	return r;
}

////////////////////////////////////////////////////////////////////////
// Range Ops

///////////////////////////////////
// Dim1
Rng1u rng1u_shift(Rng1u r, u32 x)       { return Rng1u{r.min + x, r.max + x}; }
Rng1u rng1u_pad(Rng1u r, u32 x)         { return Rng1u(r.min - x, r.max + x); }
u32 rng1u_center(Rng1u r)               { return (r.min+r.max)/2; }
b32 rng1u_contains(Rng1u r, u32 x)      { return (r.min <= x && x < r.max); }
u32 rng1u_dim(Rng1u r)                  { return r.max - r.min; }
Rng1u rng1u_union(Rng1u a, Rng1u b)     { return Rng1u(Min(a.min, b.min), Max(a.max, b.max)); }
Rng1u rng1u_intersect(Rng1u a, Rng1u b) { return Rng1u(Max(a.min, b.min), Min(a.max, b.max)); }
u32 rng1u_clamp(Rng1u r, u32 x)         { return Clamp(r.min, x, r.max); }

Rng1i rng1i_shift(Rng1i r, i32 x)       { return Rng1i(r.min + x, r.max + x); }
Rng1i rng1i_pad(Rng1i r, i32 x)         { return Rng1i(r.min - x, r.max + x); }
i32 rng1i_center(Rng1i r)               { return (r.min+r.max)/2; }
b32 rng1i_contains(Rng1i r, i32 x)      { return (r.min <= x && x < r.max); }
i32 rng1i_dim(Rng1i r)                  { return r.max - r.min; }
Rng1i rng1i_union(Rng1i a, Rng1i b)     { return Rng1i(Min(a.min, b.min), Max(a.max, b.max)); }
Rng1i rng1i_intersect(Rng1i a, Rng1i b) { return Rng1i(Max(a.min, b.min), Min(a.max, b.max)); }
i32 rng1i_clamp(Rng1i r, i32 x)         { return Clamp(r.min, x, r.max); }

Rng1u64 rng1u64_shift(Rng1u64 r, u64 x)         { return Rng1u64(r.min + x, r.max + x); }
Rng1u64 rng1u64_pad(Rng1u64 r, u64 x)           { return Rng1u64(r.min - x, r.max + x); }
u64 rng1u64_center(Rng1u64 r)                   { return (r.min+r.max)/2; }
b32 rng1u64_contains(Rng1u64 r, u64 x)          { return (r.min <= x && x < r.max); }
u64 rng1u64_dim(Rng1u64 r)                      { return r.max - r.min; }
Rng1u64 rng1u64_union(Rng1u64 a, Rng1u64 b)     { return Rng1u64(Min(a.min, b.min), Max(a.max, b.max)); }
Rng1u64 rng1u64_intersect(Rng1u64 a, Rng1u64 b) { return Rng1u64(Max(a.min, b.min), Min(a.max, b.max)); }
u64 rng1u64_clamp(Rng1u64 r, u64 x)             { return Clamp(r.min, x, r.max); }

Rng1 rng1_shift(Rng1 r, f32 x)         { return Rng1(r.min + x, r.max + x); }
Rng1 rng1_pad(Rng1 r, f32 x)           { return Rng1(r.min - x, r.max + x); }
f32 rng1_center(Rng1 r)                { return (r.min+r.max)/2; }
b32 rng1_contains(Rng1 r, f32 x)       { return (r.min <= x && x < r.max); }
f32 rng1_dim(Rng1 r)                   { return r.max - r.min; }
Rng1 rng1_union(Rng1 a, Rng1 b)        { return Rng1(Min(a.min, b.min), Max(a.max, b.max)); }
Rng1 rng1_intersect(Rng1 a, Rng1 b)    { return Rng1(Max(a.min, b.min), Min(a.max, b.max)); }
b32 rng1_overlaps(Rng1 a, Rng1 b)      { return a.min < b.max && b.min < a.max; }
f32 rng1_clamp(Rng1 r, f32 x)          { return Clamp(r.min, x, r.max); }

f32 rng1_lerp(Rng1 r, f32 t)         					{ return lerp(r.min, t, r.max); } 
f32 rng1_unlerp(Rng1 r, f32 x)       					{ return unlerp(r.min, x, r.max); }
f32 rng1_remap(f32 x, Rng1 from, Rng1 to) { return remap(x, from.min,from.max, to.min,to.max); }
Rng1 rng1_subrng(Rng1 r, Rng1 sub)   					{ return Rng1(r.min + sub.min, r.min+sub.min + rng1_dim(sub)); }
Rng1 rng1_subrng01(Rng1 r, Rng1 sub) 					{ f32 w = rng1_dim(r); return Rng1(r.min + w*sub.min, r.min + w*sub.max); }
f32 rng1_align_center(Rng1 r, f32 size) 		{ return r.min + (rng1_dim(r) - size) * 0.5f; }

///////////////////////////////////
// Dim2
f32 rng2u_width(Rng2u r) { return r.x1 - r.x0; }
f32 rng2u_height(Rng2u r) { return r.y1 - r.y0; }

f32 rng2_width(Rng2 r)                 { return r.x1 - r.x0; }
f32 rng2_height(Rng2 r)                { return r.y1 - r.y0; }
v2 rng2_top_right(Rng2 r)														{ return r.min + v2(rng2_width(r), 0); }
v2 rng2_bottom_left(Rng2 r)									   { return r.min + v2(0, rng2_height(r)); }

Rng2 rng2_shift(Rng2 r, v2 x)          { return Rng2(r.min + x, r.max + x); }
Rng2 rng2_pad(Rng2 r, f32 x)           { return Rng2(r.min - v2(x), r.max + v2(x)); }
Rng2 rng2_pad(Rng2 r, v2 x)									   { return Rng2(r.min - x, r.max + x); }
v2 rng2_center(Rng2 r)                 { return v2((r.min + r.max)/2); }
b32 rng2_contains(Rng2 r, v2 x)        { return (r.min.x <= x.x && x.x < r.max.x && r.min.y <= x.y && x.y < r.max.y); }
v2 rng2_dim(Rng2 r)                    { return v2(r.max.x - r.min.x, r.max.y - r.min.y); }
Rng2 rng2_union(Rng2 a, Rng2 b)        { return Rng2(v2(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y)), v2(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y))); }
Rng2 rng2_intersection(Rng2 a, Rng2 b) { return Rng2(v2(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y)), v2(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y))); }
b32 rng2_intersect(Rng2 a, Rng2 b)     { return (a.min.x < b.max.x && b.min.x < a.max.x) && (a.min.y < b.max.y && b.min.y < a.max.y); }
v2 rng2_clamp(Rng2 r, v2 x)            { return v2(Clamp(r.min.x, x.x, r.max.x), Clamp(r.min.y, x.y, r.max.y)); }
Rng1 rng2_rng_x(Rng2 r)													   { return Rng1(r.min.x,r.max.x); }
Rng1 rng2_rng_y(Rng2 r)													   { return Rng1(r.min.y,r.max.y); }

Rng2 rng2_lerp(Rng2 a, f32 t, Rng2 b) { return Rng2(v2_lerp(a.min, t, b.min), v2_lerp(a.max, t, b.max)); }
v2 rng2_remap(v2 p, Rng2 from, Rng2 to) {
	return v2(
		rng1_remap(p.x, rng2_rng_x(from), rng2_rng_x(to)),
		rng1_remap(p.y, rng2_rng_y(from), rng2_rng_y(to))
	);
}
v2 rng2_remap(v2 p, v2 from, Rng2 to) {
	return v2(
		rng1_remap(p.x, Rng1(0,from.x), rng2_rng_x(to)),
		rng1_remap(p.y, Rng1(0,from.y), rng2_rng_y(to))
	);
}
Rng2 rng2_remap_rng(Rng2 r, Rng2 from, Rng2 to) { return Rng2(rng2_remap(r.min, from, to), rng2_remap(r.max, from, to)); }
Rng2 rng2_remap_rng(Rng2 r, v2 from, Rng2 to) { return Rng2(rng2_remap(r.min, from, to), rng2_remap(r.max, from, to)); }
Rng2 rng2_make(v2 min, v2 size)             { return Rng2(min, min+size); }
Rng2 rng2_make_centered(v2 pos, v2 halfdim) { return Rng2(pos - halfdim, pos + halfdim); }
Rng2 rng2_scale_centered(Rng2 r, v2 scale)  { v2 halfdim = rng2_dim(r)/2; v2 c = rng2_center(r); return Rng2(c - v2_hadamard(halfdim, scale), c + v2_hadamard(halfdim, scale)); }
Rng2 rng2_scale(Rng2 r, v2 scale)           { return Rng2(v2_hadamard(r.min, scale), v2_hadamard(r.max, scale)); }
Rng2 rng2_subrng_x(Rng2 r, Rng1 sub)        { return Rng2(v2(r.min.x + sub.min, r.min.y), v2(r.min.x+sub.min + rng1_dim(sub), r.max.y)); }
Rng2 rng2_subrng_y(Rng2 r, Rng1 sub)        { return Rng2(v2(r.min.x, r.min.y + sub.min), v2(r.min.x, r.max.y+sub.min + rng1_dim(sub))); }
Rng2 rng2_subrng_x01(Rng2 r, Rng1 sub)      { f32 w = rng2_dim(r).x; return Rng2(v2(r.min.x + w*sub.min, r.min.y), v2(r.min.x + w*sub.max, r.max.y)); }
Rng2 rng2_subrng_y01(Rng2 r, Rng1 sub)      { f32 w = rng2_dim(r).y; return Rng2(v2(r.min.x, r.min.y + w*sub.min), v2(r.min.x, r.max.y + w*sub.max)); }
Rng2 rng2_cut_left(Rng2* r, f32 amount) {
	f32 x = Min(r->min.x + amount, r->max.x);
	Rng2 cut = Rng2(r->min, v2(x, r->max.y));
	r->min.x = x;
	return cut;
}
Rng2 rng2_cut_right(Rng2* r, f32 amount) {
	f32 x = Max(r->max.x - amount, r->min.x);
	Rng2 cut = Rng2(v2(x, r->min.y), r->max);
	r->max.x = x;
	return cut;
}
Rng2 rng2_cut_top(Rng2* r, f32 amount) {
	f32 y = Min(r->min.y + amount, r->max.y);
	Rng2 cut = Rng2(r->min, v2(r->max.x, y));
	r->min.y = y;
	return cut;
}
Rng2 rng2_cut_bottom(Rng2* r, f32 amount) {
	f32 y = Max(r->max.y - amount, r->min.y);
	Rng2 cut = Rng2(v2(r->min.x, y), r->max);
	r->max.y = y;
	return cut;
}
void rng2_split_x(Rng2 r, f32 t, Rng2* left, Rng2* right) {
	f32 x = rng1_lerp({r.min.x, r.max.x}, t);
	*left = Rng2(r.min, v2(x,r.max.y));
	*right = Rng2(v2(x,r.min.y), r.max);
}
void rng2_split_y(Rng2 r, f32 t, Rng2* top, Rng2* bottom) {
	f32 y = rng1_lerp({r.min.y, r.max.y}, t);
	*top = Rng2(r.min, v2(r.max.x,y));
	*bottom = Rng2(v2(r.min.x,y), r.max);
}
Rng2 rng2_col(Rng2 r, i32 idx, i32 count) {
	f32 col_w = rng2_dim(r).x / count;
	v2 min = r.min+v2(col_w*idx,0);
	v2 max = v2(min.x+col_w, r.max.y);
	return Rng2(min, max);
}
Rng2 rng2_row(Rng2 r, i32 idx, i32 count) {
	f32 col_h = rng2_dim(r).y / count;
	v2 min = r.min+v2(0, col_h*idx);
	v2 max = v2(r.max.x, min.y+col_h);
	return Rng2(min, max);
}
Rng2 rng2_grid_cell(Rng2 r, i32 row, i32 col, i32 rows, i32 cols) {
 Rng2 row_rect = rng2_row(r, row, rows);
 return rng2_col(row_rect, col, cols);
}
Rng2 rng2_align_dim_at_center(Rng2 r, v2 size) { v2 c = rng2_center(r); v2 half = size/2; return Rng2(c - half, c + half); }
Rng2 rng2_aspect_fit(Rng2 r, Rng2 fit) {
	v2 rd = rng2_dim(r), fd = rng2_dim(fit);
	f32 scale = Min(rd.x / fd.x, rd.y / fd.y);
	v2 size = v2(fd.x * scale, fd.y * scale);
	return rng2_align_dim_at_center(r, size);
}
Rng2 rng2_top(Rng2 r, f32 height) {
	Rng2 res = {r.min, {r.max.x, r.min.y + height}};
	return res;
}
Rng2 rng2_bottom(Rng2 r, f32 height) {
	Rng2 res = {{r.min.x, r.max.y - height}, r.max};
	return res;
}
Rng2 rng2_left(Rng2 r, f32 width) {
	Rng2 res = {r.min, {r.min.x + width, r.max.y}};
	return res;
}
Rng2 rng2_right(Rng2 r, f32 width) {
	Rng2 res = {{r.max.x - width, r.min.y}, r.max};
	return res;
}

// NOTE: define prefix, postfix, skip, chop operations?

///////////////////////////////////
// Dim3
Rng3 rng3_shift(Rng3 r, v3 x)    { return Rng3(r.min + x, r.max + x); }
Rng3 rng3_pad(Rng3 r, f32 x)     { return Rng3(r.min - v3(x), r.max + v3(x)); }
v3 rng3_center(Rng3 r)           { return v3((r.min.x + r.max.x)/2, (r.min.y + r.max.y)/2, (r.min.z + r.max.z)/2); }
b32 rng3_contains(Rng3 r, v3 x)  { return (r.min.x <= x.x && x.x < r.max.x && r.min.y <= x.y && x.y < r.max.y && r.min.z <= x.z && x.z < r.max.z); }
v3 rng3_dim(Rng3 r)              { return v3(r.max.x - r.min.x, r.max.y - r.min.y, r.max.z - r.min.z); }
Rng3 rng3_union(Rng3 a, Rng3 b) {
	return Rng3(v3(Min(a.min.x, b.min.x), Min(a.min.y, b.min.y), Min(a.min.z, b.min.z)),
													v3(Max(a.max.x, b.max.x), Max(a.max.y, b.max.y), Max(a.max.z, b.max.z)));
}
Rng3 rng3_intersection(Rng3 a, Rng3 b) {
	return Rng3(v3(Max(a.min.x, b.min.x), Max(a.min.y, b.min.y), Max(a.min.z, b.min.z)),
													v3(Min(a.max.x, b.max.x), Min(a.max.y, b.max.y), Min(a.max.z, b.max.z)));
}
b32 rng3_intersect(Rng3 a, Rng3 b) { return (a.min.x < b.max.x && b.min.x < a.max.x) && (a.min.y < b.max.y && b.min.y < a.max.y) && (a.min.z < b.max.z && b.min.z < a.max.z); }
v3 rng3_clamp(Rng3 r, v3 x) { return v3(Clamp(r.min.x, x.x, r.max.x), Clamp(r.min.y, x.y, r.max.y), Clamp(r.min.z, x.z, r.max.z)); }

Rng3 rng3_make(v3 min, v3 size)             { return Rng3(min, min+size); }
Rng3 rng3_make_centered(v3 pos, v3 halfdim) { return Rng3(pos - halfdim, pos + halfdim); }
Rng3 rng3_scale_centered(Rng3 r, f32 scale) { v3 halfdim = rng3_dim(r)/2; v3 c = rng3_center(r); return Rng3(c - halfdim*scale, c + halfdim*scale); }
Rng3 rng3_scale_centered(Rng3 r, v3 scale)  { v3 halfdim = rng3_dim(r)/2; v3 c = rng3_center(r); return Rng3(c - v3_hadamard(halfdim, scale), c + v3_hadamard(halfdim, scale)); }
Rng3 rng3_scale(Rng3 r, f32 scale)          { return Rng3(r.min*scale, r.max*scale); }
Rng3 rng3_scale(Rng3 r, v3 scale)           { return Rng3(v3_hadamard(r.min, scale), v3_hadamard(r.max, scale)); }
f32 rng3_radius(Rng3 r)																					{ return v3_len(rng3_dim(r)/2); }
Rng3 rng3_empty()																											{ return Rng3(v3(F32_MIN), v3(F32_MAX)); }
void rng3_push(Rng3& r, v3 p)															{
	if(p.x < r.min.x) r.min.x = p.x;
	if(p.x > r.max.x) r.max.x = p.x;
	if(p.y < r.min.x) r.min.y = p.y;
	if(p.y > r.max.x) r.max.y = p.y;
	if(p.z < r.min.x) r.min.z = p.z;
	if(p.z > r.max.x) r.max.z = p.z;
}
Rng3 rng3_transform(Rng3 r, m4x4 m) {
	v3 t = v3(m.v[3][0], m.v[3][1], m.v[3][2]);
	Rng3 out = {t,t};
	Loop(i, 3) {
		Loop(j, 3) {
			f32 a = m.v[i][j] * r.min.v[j];
			f32 b = m.v[i][j] * r.max.v[j];
			out.min.v[i] += Min(a, b);
			out.max.v[i] += Max(a, b);
		}
	}
	return out;
}

f32 smoothstep(f32 t)   { return t*t*(3-2*t); }
f32 smootherstep(f32 t) { return t * t * t * (t * (t * 6 - 15) + 10); }

////////////////////////////////////////////////////////////////////////
// Ease
f32 ease_quad_in(f32 t)     { return Square(t); }
f32 ease_quad_out(f32 t)    { return -(t * (t - 2)); }
f32 ease_quad_in_out(f32 t) {
	if(t < 0.5f)
		return 2 * t * t;
	else
		return -2*t*t + 4*t - 1;
}

f32 ease_cube_in(f32 t)     { return Cube(t); }
f32 ease_cube_out(f32 t)    { f32 f = t - 1; return Cube(f) + 1; }
f32 ease_cube_in_out(f32 t) {
	if(t < 0.5f)
		return 4 * t * t * t;
	else {
		f32 f = 2*t - 2;
		return 0.5f * f * f * f + 1;
	}
}

f32 ease_quart_in(f32 t)     { return Cube(t) * t; }
f32 ease_quart_out(f32 t)    { f32 f = (t - 1); return Cube(f) * (1 - t) + 1; }
f32 ease_quart_in_out(f32 t) {
	if(t < 0.5f)
		return 8 * t * t * t * t;
	else {
		f32 f = t - 1;
		return -8 * f * f * f * f + 1;
	}
}

f32 ease_quint_in(f32 t)     { return Cube(t) * Square(t); }
f32 ease_quint_out(f32 t)    { f32 f = (t - 1); return Cube(f) * Square(f) + 1; }
f32 ease_quint_in_out(f32 t) {
	if(t < 0.5f)
		return 16 * t * t * t * t * t;
	else {
		f32 f = 2 * t - 2;
		return 0.5f * f * f * f * f * f + 1;
	}
}

f32 ease_sin_in(f32 t) { return Sin((t - 1) * PI/2) + 1; }
f32 ease_sin_out(f32 t) { return Sin(t * PI/2); }
f32 ease_sin_in_out(f32 t) { return (1 - Cos(t * PI)) / 2; }

f32 ease_circ_in(f32 t) { return 1 - Sqrt(1 - Square(t)); }
f32 ease_circ_out(f32 t) { return Sqrt((2 - t) * t); }
f32 ease_circ_in_out(f32 t) {
	if(t < 0.5f) {
		return 0.5f * (1 - Sqrt(1 - 4 * (t * t)));
	} else {
		return 0.5f * (Sqrt(-((2 * t) - 3) * ((2 * t) - 1)) + 1);
	}
}

f32 ease_exp_in(f32 t)  { return (t == 0.0f) ? t : Pow(2, 10 * (t - 1)); }
f32 ease_exp_out(f32 t) { return (t == 1.0f) ? t : 1 - Pow(2, -10 * t); }
f32 ease_exp_in_out(f32 t) {
	if(t == 0.0f || t == 1.0f) return t;
	if(t < 0.5f) {
		return 0.5f * Pow(2, (20 * t) - 10);
	} else {
		return -0.5f * Pow(2, (-20 * t) + 10) + 1;
	}
}

f32 ease_elastic_in(f32 t) { return Sin(13 * PI/2 * t) * Pow(2, 10 * (t - 1));}
f32 ease_elastic_out(f32 t) {	return Sin(-13 * PI/2 * (t + 1)) * Pow(2, -10 * t) + 1;}
f32 ease_elastic_in_out(f32 t) {
	if(t < 0.5f) {
		return 0.5f * Sin(13 * PI/2 * (2 * t)) * Pow(2, 10 * ((2 * t) - 1));
	} else {
		return 0.5f * (Sin(-13 * PI/2 * ((2 * t - 1) + 1)) * Pow(2, -10 * (2 * t - 1)) + 2);
	}
}

f32 ease_back_in(f32 t) { return t * t * t - t * Sin(t * PI); }
f32 ease_back_out(f32 t) { f32 f = (1 - t); return 1 - (f * f * f - f * Sin(f * PI));	}
f32 ease_back_in_out(f32 t) {
	if(t < 0.5f) {
		f32 f = 2 * t;
		return 0.5f * (f * f * f - f * Sin(f * PI));
	} else {
		f32 f = (1 - (2 * t - 1));
		return 0.5f * (1 - (f * f * f - f * Sin(f * PI))) + 0.5f;
	}
}

f32 ease_bounce_in(f32 t) { return 1 - ease_bounce_out(1 - t); }
f32 ease_bounce_out(f32 t) {
	if(t < 4 / 11.0f) {
		return (121 * t * t) / 16.0f;
	} else if(t < 8 / 11.0f) {
		return (363 / 40.0f * t * t) - (99 / 10.0f * t) + 17 / 5.0f;
	} else if(t < 9 / 10.0) {
		return (4356 / 361.0f * t * t) - (35442 / 1805.0f * t) + 16061 / 1805.0f;
	} else {
		return (54 / 5.0f * t * t) - (513 / 25.0f * t) + 268 / 25.0f;
	}
}
f32 ease_bounce_in_out(f32 t) {
	if(t < 0.5f) {
		return 0.5f * ease_bounce_in(t * 2);
	} else {
		return 0.5f * ease_bounce_out(t * 2 - 1) + 0.5f;
	}
}

const u32 MEM_ALLOC_HEADER_GUARD   = 0xA110C8;
const u32 MEM_DEALLOC_HEADER_GUARD = 0xDE1E7E;
const u32 MEM_ALLOC_GUARD          = 0xA1;
const u32 MEM_DEALLOC_GUARD        = 0xDE;
const u32 MEM_ALLOC_TAIL_GUARD     = 0xdeedbeef;

#if MEM_GUARD
	#define MemGuardAlloc(d, z)   MemSet(d, MEM_ALLOC_GUARD, z)
	#define MemGuardDealloc(d, z) MemSet(d, MEM_DEALLOC_GUARD, z)
#else
	#define MemGuardAlloc(d, c)
	#define MemGuardDealloc(d, c)
#endif

const u32 ARENA_LIST_BLOCK_SIZE = KB(64);

////////////////////////////////////////////////////////////////////////
// Mem track
#if MEM_TRACK

void mem_track_end() { ArrayZero(mem_track_st.infos_alocs_per_frame); }
AllocatorInfoList mem_track_info() { return mem_track_st.roots; }

AllocatorInfo* mem_track_get_info(Allocator alloc) {
	switch(alloc.type) {
		default: return null;
		case AllocatorType_Arena: return ((Arena*)alloc.ctx)->info;
		case AllocatorType_Alloc: return ((Alloc*)alloc.ctx)->info;
	}
}

AllocatorInfo* mem_track_make(Allocator parent_alloc, AllocatorType type, String name, String file, u32 line) {
	if(ptr_match(parent_alloc.ctx, &tctx.arenas[0]) || ptr_match(parent_alloc.ctx, &tctx.arenas[1])) {
		return null;
	}
	AllocatorInfo* info = mem_track_st.free;
	AllocatorInfo* parent = mem_track_get_info(parent_alloc);
	if(info) {
		sll_stack_pop(mem_track_st.free);
	} else {
		info = &mem_track_st.infos[mem_track_st.allocated_infos_count++];
	}
	MemZeroStruct(info);
	
	info->type = type;
	info->thread_idx = tctx_id();
	str_copy(info->name, name);
	str_copy(info->file, file);
	info->line = line;
	info->parent = parent;

	LockScope(mem_track_st.mutex);
	if(parent) {
		dll_push_back(parent->first, parent->last, info);
		parent->child_count++;
	} else {
		dll_push_back(mem_track_st.roots.first, mem_track_st.roots.last, info);
		mem_track_st.roots.count++;
	}
	return info;
}

void mem_track_destroy(AllocatorInfo* info) {
	if(info) {
		LockScope(mem_track_st.mutex);
		if(info->parent) {
			dll_remove(info->parent->first, info->parent->last, info);
			info->parent->child_count--;
		} else {
			dll_remove(mem_track_st.roots.first, mem_track_st.roots.last, info);
			mem_track_st.roots.count--;
		}
	}
}

void mem_track_on_alloc(AllocatorInfo* info, u64 size) {
	if(info) {
		atomic_add(&info->pos, size);
		atomic_inc(&info->allocs_count);
		atomic_inc(&mem_track_st.infos_alocs_per_frame[info - mem_track_st.infos]);
	}
}

void mem_track_on_free(AllocatorInfo* info, u64 size) {
	if(info) {
		atomic_sub(&info->pos, size);
		atomic_inc(&info->frees_count);
	}
}

void mem_track_on_commit(AllocatorInfo* info, u64 size) {
	if(info) {
		if(info->parent) {
			atomic_add(&info->parent->children_size, size);
		}
		atomic_add(&info->cap, size);
	}
}

void mem_track_reset(AllocatorInfo* info) {
	info->pos = 0;
	info->children_size = 0;
	info->cap = 0;
	info->allocs_count = 0;
	info->frees_count = 0;
}

void destroy(AllocatorInfo* info) {
	LoopNode (child, info->first) {
		destroy(child);
	}
	mem_track_destroy(info);
}

void reset(AllocatorInfo* info) {
	LoopNode (child, info->first) {
		destroy(child);
	}
	mem_track_reset(info);
}

#else
AllocatorInfoList mem_track_info() { return {}; }
void mem_track_init() {}
void mem_track_end() {}
#endif

////////////////////////////////////////////////////////////////////////
// Arena
Arena::operator Allocator() { return {.type = AllocatorType_Arena, .ctx = this}; }

Arena _arena_make(ArenaParams params) {
	u64 reserve_size = params.reserve_size;
	u8* base = os_reserve(reserve_size);
	Arena result = {
		.base = base,
		.cap = reserve_size,
		.lock = params.lock,
	};
#if MEM_TRACK
	result.info = mem_track_make({}, AllocatorType_Arena, params.name, params.file, params.line);
#endif
	return result;
}

void arena_destroy(Arena& arena) {
	os_release(arena.base, arena.cap);
#if MEM_TRACK
	destroy(arena.info);
#endif
}

void arena_clear(Arena& arena) { 
	AsanPoisonMemRegion(arena.base, arena.cmt);
#if MEM_TRACK
	mem_track_on_free(arena.info, arena.pos);
	if(arena.info->first) {
		reset(arena.info->first);
	}
#endif
	arena.pos = 0;
}

u8* arena_alloc(Arena* arena, u64 size, u64 align) {
	if(arena->lock) os_mutex_lock(arena->mutex);
	u64 pos = align_up(arena->pos, align);
	u64 pad = pos - arena->pos;
	if(pos + size > arena->cmt) {
		u64 commit_size = align_up(pad + size, ARENA_DEFAULT_COMMIT_SIZE);
		Assert((pos + commit_size) <= arena->cap && "Arena is out of memory");
		os_commit(Offset(arena->base, arena->cmt), commit_size);
		MemGuardDealloc(Offset(arena->base, arena->cmt), commit_size);
		AsanPoisonMemRegion(Offset(arena->base, arena->cmt), commit_size);
		arena->cmt += commit_size;
#if MEM_TRACK
		mem_track_on_commit(arena->info, commit_size);
#endif
	}
	AsanUnpoisonMemRegion(Offset(arena->base, arena->pos), size + pad);
	MemGuardAlloc(Offset(arena->base, arena->pos), size + pad);
	u8* result = Offset(arena->base, pos);
	arena->pos = pos + size;
#if MEM_TRACK
	mem_track_on_alloc(arena->info, size);
#endif
	if(arena->lock) os_mutex_unlock(arena->mutex);
	return result;
}

u8* arena_alloc_zero(Arena* arena, u64 size, u64 align) {
	u8* result = arena_alloc(arena, size, align);
	MemZero(result, size);
	return result;
}

u8* arena_realloc(Arena* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
	u8* result = arena_alloc(arena, new_size, align);
	MemCopy(result, ptr, old_size);
	return result;
}

u8* arena_realloc_zero(Arena* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
	u8* result = arena_alloc(arena, new_size, align);
	MemCopy(result, ptr, old_size);
	MemZero(Offset(result, old_size), new_size - old_size);
	return result;
}

Temp temp_begin(Arena* arena) {
	return Temp{arena, arena->pos};
};
void temp_end(Temp temp) {
	temp.arena->pos = temp.pos;
#if MEM_TRACK
	temp.arena->info->pos = temp.arena->pos;
#endif
}

////////////////////////////////////////////////////////////////////////
// ArenaList
ArenaList::operator Allocator() { return {.type = AllocatorType_ArenaList, .ctx = this}; }

ArenaList alloc_arena_list_make(Allocator alloc) {
	ArenaList res = {
		.alloc = alloc,
	};
	return res;
}

void alloc_arena_list_clear(ArenaList& arena) {
	for(ArenaBlock* b = arena.first;; b = b->next) {
		u8* base = Offset(b, sizeof(ArenaBlock));
		MemGuardDealloc(base, b->pos);
		AsanPoisonMemRegion(base, ARENA_LIST_BLOCK_SIZE);
		b->pos = 0;
		if(b == arena.current)
			break;
	}
	arena.current = arena.first;
}

ArenaBlock* arena_list_new_block(ArenaList* arena) {
	ArenaBlock* b = (ArenaBlock*)mem_alloc(arena->alloc, sizeof(ArenaBlock) + ARENA_LIST_BLOCK_SIZE);
	*b = {
		.cap = ARENA_LIST_BLOCK_SIZE,
	};
	u8* base = Offset(b, sizeof(ArenaBlock));
	MemGuardDealloc(base, ARENA_LIST_BLOCK_SIZE);
	AsanPoisonMemRegion(base, ARENA_LIST_BLOCK_SIZE);
	return b;
}

u8* arena_list_alloc(ArenaList* arena, u64 size, u64 align) {
	Assert(align_up(size, align) <= KB(64));
	if(!arena->current) {
		arena->first = arena->current = arena_list_new_block(arena);
	}
	u8* base = Offset(arena->current, sizeof(ArenaBlock));
	u64 pos = align_up((u64)base + arena->current->pos, align) - (u64)base;
	if(pos + size > arena->current->cap) {
		if(arena->current->next) {
			arena->current = arena->current->next;
		} else {
			arena->current = arena->current->next = arena_list_new_block(arena);
		}
		base = Offset(arena->current, sizeof(ArenaBlock));
		pos = align_up((u64)base, align) - (u64)base;
	}
	u64 pad = pos - arena->current->pos;
	AsanUnpoisonMemRegion(Offset(base, arena->current->pos), size + pad);
	MemGuardAlloc(Offset(base, arena->current->pos), size + pad);
	u8* result = Offset(base, pos);
	arena->current->pos = pos + size;
	return result;
}

u8* arena_list_alloc_zero(ArenaList* arena, u64 size, u64 align) {
	u8* result = arena_list_alloc(arena, size, align);
	MemZero(result, size);
	return result;
}

u8* arena_list_realloc(ArenaList* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
	u8* result = arena_list_alloc(arena, new_size, align);
	MemCopy(result, ptr, old_size);
	return result;
}

u8* arena_list_realloc_zero(ArenaList* arena, void* ptr, u64 old_size, u64 new_size, u64 align) {
	u8* result = arena_list_alloc(arena, new_size, align);
	MemCopy(result, ptr, old_size);
	MemZero(Offset(result, old_size), new_size - old_size);
	return result;
}

////////////////////////////////////////////////////////////////////////
// Segregated pow2 list
struct AllocHeader {
#if BUILD_DEBUG
	u32 head_guard;
	u32 size;
#endif
	u32 off;
};

Alloc::operator Allocator() { return {.type = AllocatorType_Alloc, .ctx = this};}

Alloc _alloc_make(Allocator alloc, AllocParams params) {
	Alloc res = {
		.alloc = alloc,
		.lock = params.lock,
	};
#if MEM_TRACK
	res.info = mem_track_make(alloc, AllocatorType_Alloc, params.name, params.file, params.line);
#endif
	return res;
}

void alloc_destroy(Alloc alloc) {
#if MEM_TRACK
	mem_track_destroy(alloc.info);
#endif
}

u8* intern_alloc(Alloc* alloc, u64 size) {
	Assert(size > 7);
	u64 pow2_size = next_pow2(size);
	u64 pool_idx = ctz(pow2_size) - ctz(8);
	MemNode* p = alloc->pools[pool_idx].next;
	if(p == null) {
		u8* buf = mem_alloc(alloc->alloc, pow2_size);
#if MEM_TRACK
		mem_track_on_commit(alloc->info, pow2_size);
		mem_track_on_alloc(alloc->info, pow2_size);
#endif
		return buf;
	}
	sll_stack_pop(alloc->pools[pool_idx].next);
	AsanUnpoisonMemRegion(p, pow2_size);
	MemGuardAlloc(p, pow2_size);
#if MEM_TRACK
	mem_track_on_alloc(alloc->info, size);
#endif
	return (u8*)p;
}

void intern_free(Alloc* alloc, void* ptr, u64 size) {
	Assert(size > 7);
	u64 pow2_size = next_pow2(size);
	u64 pool_idx = ctz(pow2_size) - ctz(8);
	MemNode* p = (MemNode*)ptr;
	sll_stack_push(alloc->pools[pool_idx].next, p);
	MemGuardDealloc(Offset(p, sizeof(void*)), pow2_size - sizeof(void*));
	AsanPoisonMemRegion(Offset(p, sizeof(void*)), pow2_size - sizeof(void*));
#if MEM_TRACK
	mem_track_on_free(alloc->info, size);
#endif
}

// mem block: align_pad -> header -> mem
u8* intern_alloc_align(Alloc* alloc, u64 size, u64 align) {
	u64 alloc_size = align+sizeof(AllocHeader)+size;
	u8* raw = intern_alloc(alloc, alloc_size);
	u8* user = align_up_ptr(raw+sizeof(AllocHeader), align);
	AllocHeader* h = OffsetBackStruct(user, AllocHeader);
	h->off = user - raw;
	return user;
}

void intern_free_align(Alloc* alloc, void*ptr, u64 size) {
	AllocHeader* h = OffsetBackStruct(ptr, AllocHeader);
	u8* raw = OffsetBack(ptr, h->off);
	u64 raw_size = size + ptr_diff(ptr, raw);
	intern_free(alloc, raw, raw_size);
}

// mem block: align_pad -> header -> mem -> u32 tail guard
u8* alloc_alloc(Alloc* alloc, u64 size, u64 align) {
	if(alloc->lock) os_mutex_lock(alloc->mutex);
	u8* res = null;
#if BUILD_DEBUG
	u64 alloc_size = size + sizeof(u32);
	res = intern_alloc_align(alloc, alloc_size, align);
	AllocHeader* h = OffsetBackStruct(res, AllocHeader);
	h->head_guard = MEM_ALLOC_HEADER_GUARD;
	h->size = size;
	*OffsetAs(res, u32, size) = MEM_ALLOC_TAIL_GUARD;
#else
	res = intern_alloc_align(alloc, size, align);
#endif
	if(alloc->lock) os_mutex_unlock(alloc->mutex);
	return res;
}

u8* alloc_alloc_zero(Alloc* alloc, u64 size, u64 align)  {
	u8* result = alloc_alloc(alloc, size, align);
	MemZero(result, size);
	return result;
}

void alloc_free(Alloc* alloc, void* ptr, u64 size) {
	if(alloc->lock) os_mutex_lock(alloc->mutex);
#if BUILD_DEBUG
	AllocHeader* h = OffsetBackStruct(ptr, AllocHeader);
	Assert(h->head_guard == MEM_ALLOC_HEADER_GUARD);
	Assert(h->size == size);
	u32* tail = OffsetAs(ptr, u32, size);
	Assert(*tail == MEM_ALLOC_TAIL_GUARD);
	h->head_guard = MEM_DEALLOC_HEADER_GUARD;
	intern_free_align(alloc, ptr, size + sizeof(u32));
#else
	intern_free_align(alloc, ptr, size);
#endif
	if(alloc->lock) os_mutex_unlock(alloc->mutex);
}

u8* alloc_realloc(Alloc* alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
	u8* res = alloc_alloc(alloc, new_size, align);
	MemCopy(res, ptr, old_size);
	alloc_free(alloc, ptr, old_size);
	return res;
}

u8* alloc_realloc_zero(Alloc* alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
	u8* res = alloc_alloc(alloc, new_size, align);
	MemCopy(res, ptr, old_size);
	MemZero(Offset(res, old_size), new_size - old_size);
	alloc_free(alloc, ptr, old_size);
	return res;
}

////////////////////////////////////////////////////////////////////////
// tlsf
// TLSF_Allocator tlsf_init() {
//   return {};
// }

// struct BinmapInfo {
//   u32 bin_idx;
//   u32 sub_bin_idx;
//   u64 rounded_size;
// };

// struct BlockMap {
//   u32 bin_idx;
//   u32 sub_bin_idx;
//   u64 rounded_size;
//   u32 idx;
// };

// intern BinmapInfo binmap_up(u64 size) {
//   u64 bin_idx = most_significant_bit(size | MinAllocation);
//   u64 subbin_size = 1 << (bin_idx - ctz(SubbinCount));
//   u64 rounded_size = AlignUp(size, subbin_size);
//   u32 sub_bin_idx = (rounded_size - (1 << bin_idx)) / subbin_size;

//   u32 adjusted_bin_idx = (bin_idx - ctz(MinAllocation)) + (sub_bin_idx / SubbinCount);
//   u32 adjusted_subbin_idx = sub_bin_idx % SubbinCount;

//   return {
//     .bin_idx = adjusted_bin_idx,
//     .sub_bin_idx = adjusted_subbin_idx,
//     .rounded_size = rounded_size,
//   };
// }

// intern BlockMap tlsf_find_free_block(TLSF_Allocator& a, u64 size) {
//   BinmapInfo map = binmap_up(size);
//   u32 sub_bin_bitmap = a.sub_bin_bitmaps[map.bin_idx] & (~0 << map.sub_bin_idx);
//   if(sub_bin_bitmap == 0) {
//     u32 bin_bitmap = a.bin_bitmap & (~0 << (map.bin_idx + 1));
//     if(bin_bitmap == 0) return {};
//     map.bin_idx = ctz(bin_bitmap);
//     sub_bin_bitmap = a.sub_bin_bitmaps[map.sub_bin_idx];
//   }

//   Assert(sub_bin_bitmap != 0);
//   map.sub_bin_idx = ctz(sub_bin_bitmap);
//   u32 idx = map.bin_idx * SubbinCount + map.sub_bin_idx;

//   return {
//     .bin_idx = map.bin_idx,
//     .sub_bin_idx = map.sub_bin_idx,
//     .rounded_size = map.rounded_size,
//     .idx = idx,
//   };
// }

// u8* tlsf_alloc(TLSF_Allocator& a, u64 size, u64 alignment) {
//   BlockMap block_map = tlsf_find_free_block(a, size);

//   return {};
// }

////////////////////////////////////////////////////////////////////////
// Allocator Interface
u8* mem_alloc(Allocator alloc, u64 size, u64 align) {
	switch(alloc.type) {
		InvalidDefaultCase;
		case AllocatorType_Arena:     return arena_alloc((Arena*)alloc.ctx, size, align);
		case AllocatorType_ArenaList: return arena_list_alloc((ArenaList*)alloc.ctx, size, align);
		case AllocatorType_Alloc:     return alloc_alloc((Alloc*)alloc.ctx, size, align);
	}
}
u8* mem_alloc_zero(Allocator alloc, u64 size, u64 align) {
	switch(alloc.type) {
		InvalidDefaultCase;
		case AllocatorType_Arena:     return arena_alloc_zero((Arena*)alloc.ctx, size, align);
		case AllocatorType_ArenaList: return arena_list_alloc((ArenaList*)alloc.ctx, size, align);
		case AllocatorType_Alloc:     return alloc_alloc_zero((Alloc*)alloc.ctx, size, align);
	}
}
u8* mem_realloc(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
	switch(alloc.type) {
		InvalidDefaultCase;
		case AllocatorType_Arena:     return arena_realloc((Arena*)alloc.ctx, ptr, old_size, new_size, align);
		case AllocatorType_ArenaList: return arena_list_realloc((ArenaList*)alloc.ctx, ptr, old_size, new_size, align);
		case AllocatorType_Alloc:     return alloc_realloc((Alloc*)alloc.ctx, ptr, old_size, new_size, align);
	}
}
u8* mem_realloc_zero(Allocator alloc, void* ptr, u64 old_size, u64 new_size, u64 align) {
	switch(alloc.type) {
		InvalidDefaultCase;
		case AllocatorType_Arena:     return arena_realloc_zero((Arena*)alloc.ctx, ptr, old_size, new_size, align);
		case AllocatorType_ArenaList: return arena_list_realloc_zero((ArenaList*)alloc.ctx, ptr, old_size, new_size, align);
		case AllocatorType_Alloc:     return alloc_realloc_zero((Alloc*)alloc.ctx, ptr, old_size, new_size, align);
	}
}
void mem_free(Allocator alloc, void* ptr, u64 size) {
	switch(alloc.type) {
		InvalidDefaultCase;
		case AllocatorType_Arena:     return;
		case AllocatorType_ArenaList: return;
		case AllocatorType_Alloc:     return alloc_free((Alloc*)alloc.ctx, ptr, size);
	}
}

////////////////////////////////////////////////////////////////////////
// General GPU allocator (segregated pow2)
GpuAllocSegList gpu_alloc_seglist_make(Allocator alloc) {
	GpuAllocSegList res = {
		.alloc = alloc,
	};
	res.range_cap = 8,
	res.data = push_array(alloc, GpuBlockList, res.range_cap);
	return res;
}
GpuMemId gpu_alloc_seglist_alloc(GpuAllocSegList& a, u64 size, u64 align) {
	Assert(size > 0);
	u64 alloc_size = align + size;
	u64 pow2_size = next_pow2(alloc_size);
	u64 pool_idx = ctz(pow2_size) - ctz(8);
	u32& p = a.heads[pool_idx];
	if(p == 0) {
		u64 cur_pos = align_up(a.pos, align);
		a.pos = cur_pos + pow2_size;
		GpuBlockList range = {0, true, cur_pos, alloc_size};
		u32 result = a.range_count;
		if(a.range_count >= a.range_cap) {
			a.range_cap *= 2;
			mem_realloc_array(a.alloc, a.data, a.range_count, a.range_cap) ;
		}
		a.data[a.range_count++] = range;
		return result;
	}
	u32 result = p;
	p = a.data[p].next;
	return result;
}
void gpu_alloc_seglist_free(GpuAllocSegList& a, GpuMemId h) {
	GpuBlockList& range = a.data[h];
	Assert(range.is_allocated == true);
	u64 pow2_size = next_pow2(range.range.size);
	u64 pool_idx = ctz(pow2_size) - ctz(8);
	u32& p = a.heads[pool_idx];
	range.next = p;
	p = h;
}

u64 gpu_alloc_seglist_get(GpuAllocSegList& a, GpuMemId h) {
	return a.data[h].range.offset;
}

////////////////////////////////////////////////////////////////////////
// Utils
struct SoALayout {
	u64 size;
	u64 offsets[8];
};

u64 mem_soa_size(u32 count, Slice<SoA_Field> fields) {
	u64 off = 0;
	Loop(i, fields.count) {
		SoA_Field field = fields[i];
		offset_push(off, field.elem_size*count, field.align);
	}
	return off;
}

SoALayout mem_soa_layout(u32 count, Slice<SoA_Field> fields) {
	SoALayout res = {};
	Loop(i, fields.count) {
		SoA_Field field = fields[i];
		res.offsets[i] = offset_push(res.size, field.elem_size*count, field.align);
	}
	return res;
}

u8* mem_alloc_soa(Allocator alloc, u32 count, Slice<SoA_Field> fields) {
	SoALayout layout = mem_soa_layout(count, fields);
	u8* buf = mem_alloc(alloc, layout.size, fields[0].align);
	Loop(i, fields.count) {
		*(fields[i].dst_ptr) = Offset(buf, layout.offsets[i]);
	}
	return buf;
}

u8* mem_realloc_soa(Allocator alloc, u32 old_count, u32 new_count, Slice<SoA_Field> fields) {
	void* old_ptr = *fields[0].dst_ptr;
	SoALayout layout = mem_soa_layout(new_count, fields);
	u8* buf = mem_alloc(alloc, layout.size, fields[0].align);
	Loop(i, fields.count) {
		void* old_ptr = *(fields[i].dst_ptr);
		void* new_ptr = Offset(buf, layout.offsets[i]);
		u64 old_ptr_size = fields[i].elem_size * old_count;
		MemCopy(new_ptr, old_ptr, old_ptr_size);
		*(fields[i].dst_ptr) = new_ptr;
	}
	mem_free(alloc, old_ptr, mem_soa_size(old_count, fields));
	return buf;
}

u8* mem_alloc_soa_zero(Allocator alloc, u32 count, Slice<SoA_Field> fields) {
	SoALayout layout = mem_soa_layout(count, fields);
	u8* buf = mem_alloc_zero(alloc, layout.size, fields[0].align);
	Loop(i, fields.count) {
		*(fields[i].dst_ptr) = Offset(buf, layout.offsets[i]);
	}
	return buf;
}

u8* mem_realloc_soa_zero(Allocator alloc, u32 old_count, u32 new_count, Slice<SoA_Field> fields) {
	void* old_ptr = *fields[0].dst_ptr;
	SoALayout layout = mem_soa_layout(new_count, fields);
	u8* buf = mem_alloc_zero(alloc, layout.size, fields[0].align);
	u64 old_size = mem_soa_size(old_count, fields);
	MemZero(Offset(buf, old_size), layout.size - old_size);
	Loop(i, fields.count) {
		void* old_ptr = *(fields[i].dst_ptr);
		void* new_ptr = Offset(buf, layout.offsets[i]);
		u64 old_ptr_size = fields[i].elem_size * old_count;
		MemCopy(new_ptr, old_ptr, old_ptr_size);
		*(fields[i].dst_ptr) = new_ptr;
	}
	mem_free(alloc, old_ptr, old_size);
	return buf;
}

u64 offset_push(u64& offset, u64 size, u64 align) {
	u64 result = align_up(offset, align);
	offset = result + size;
	return result;
}

MemFormatSize mem_format_size(f32 value) {
	u32 unit = 0;
	while(value >= 1024) {
		value /= 1024;
		unit++;
	}
	String mem_units[] = {"B", "KB", "MB", "GB", "TB"};
	MemFormatSize result = {
		.format = mem_units[unit],
		.format_id = unit,
		.size = value,
	};
	return result;
}

String::String(const char* str_) { 
	str = (u8*)str_;
	size = cstr_length(str);
}

Slice<u8> dstr_slice(Dstring dstr) {
	return Slice(dstr.str, dstr.size);
}
Dstring dstr_make(Allocator alloc) {
	Dstring res = {
		.alloc = alloc,
	};
	return res;
}
u8* dstr_alloc(Dstring& arr, u64 size) {
	if(size + arr.size > arr.cap) {
		if(arr.str) {
			u32 modifier = div_ceil(size+arr.size, arr.cap);
			u32 old_cap = arr.cap;
			arr.cap *= modifier;
			arr.str = mem_realloc_array(arr.alloc, arr.str, old_cap, arr.cap);
		} else {
			arr.cap = Max(size, DEFAULT_CAPACITY);
			arr.str = mem_alloc(arr.alloc, arr.cap);
		}
	}
	u8* res = arr.str+arr.size;
	arr.size += size;
	return res;
}
void dstr_push(Dstring& arr, String str) {
	u8* dst = dstr_alloc(arr, str.size);
	MemCopy(dst, str.str, str.size);
}
void dstr_pushf(Dstring& arr, String fmt, ...) {
	VaList argc;
	va_start(argc, fmt); 
	u32 need_bytes = str_fmt(null, fmt, argc);
	va_end(argc);
	u8* buf = dstr_alloc(arr, need_bytes);
	va_start(argc, fmt); 
	u32 final_size = str_fmt(buf, fmt, argc);
	va_end(argc);
	String res = {buf, final_size};
}
void dstr_clear(Dstring&arr) {
	arr.size = 0;
}

Dstring::operator String() { return String(str, size); }
String64::operator String() { return String(str, size); }

u64 cstr_length(const void* c) {
	u8* p = (u8*)c;
	for(;*p != 0; p++);
	return (u64)(p - (u8*)c);
}

////////////////////////////////////////////////////////////////////////
// String <=> Integer Conversions
u32 u64_length(u64 x) {
	u32 n = 1;
	while(n < 20 && x >= u64_pow10[n]) n++;
	return n;
}

u32 u64_write(u8* dest, u64 x) {
	u32 length = u64_length(x);
	u32 i = length;
	do {
		dest[--i] = '0' + (x % 10);
		x /= 10;
	} while(x);
	return length;
}

u32 i64_length(i64 x) {
	if(x < 0)
		return 1+u64_length(-x);
	else
		return u64_length(x);
}

u32 i64_write(u8* dest, i64 x) {
	if(x < 0) {
		dest[0] = '-';
		return 1 + u64_write(dest+1, -x);
	}
	else {
		return u64_write(dest, x);
	}
}

u32 f64_length(f64 x, u32 precision) {
	u32 len = 0;
	if(x < 0.0) {
		len++;
		x = -x;
	}
	len += u64_length(x);
	if(precision > 0) {
		len++;
		len += precision;
	}
	return len;
}

u32 f64_write(u8* dest, f64 x, u32 precision) {
	u32 len = 0;
	if(is_nan(x)) { dest[0]='n'; dest[1]='a'; dest[2]='n'; return 3; }
	if(is_inf(x)) { dest[0]='i'; dest[1]='n'; dest[2]='f'; return 3; }
	if(x < 0.0f) { dest[len++] = '-'; x = -x; }
	u64 ip = x;
	f64 fp = x - ip;
	u64 mult = u64_pow10[precision];
	u64 fr = fp * mult + 0.5;
	if(fr >= mult) { fr -= mult; ip++; }
	len += u64_write(dest + len, ip);
	if(precision > 0) {
		dest[len++] = '.';
		LoopReverse(i, precision) {
			dest[len + i] = '0' + fr % 10;
			fr /= 10; 
		}
		len += precision;
	}
	return len;
}

void u64_hex_write(u8* dest, u64 value, b32 upper) {
#define HEX_LENGTH 16
	var* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
	LoopReverse(i, HEX_LENGTH) {
		dest[i] = digits[value & 0xF];
		value >>= 4;
	}
}

u32 u32_length(u32 x) { return u64_length(x); }
u32 i32_length(i32 x) { return i64_length(x); }
u32 f32_length(f32 x, u32 precision) { return f64_length(x, precision); }
u32 u32_write(u8* dest, u32 x) { return u64_write(dest, x); }
u32 i32_write(u8* dest, i32 x) { return i64_write(dest, x); }
u32 f32_write(u8* dest, f32 x, u32 precision) { return f64_write(dest, x, precision); }

u64 u64_from_str(String str) {
	u64 x = 0;
	Loop(i, str.size) {
		x *= 10;
		x += str.str[i]-'0';
	}
	return x;
}

i64 i64_from_str(String str) {
	i64 x = 0;
	u32 i = 0;
	b32 negative = false;
	if(str.str[i] == '-') {
		negative = true;
		i++;
	}
	Loop(j, str.size - i) {
		x *= 10;
		x += str.str[i + j] - '0';
	}
	if(negative)
		x = -x;
	return x;
}

f64 f64_from_str(String str) {
	u8* p = str.str;
	u8* end = p + str.size;
	u64 mant = 0;
	u32 ndig = 0;
	i32 exp10 = 0;
	b32 neg = false;
	if(p < end && (*p == '-' || *p == '+')) neg = (*p++ == '-');

	for(;p < end; p++) {
		u32 d = (u32)*p - '0';
		if(d > 9) break;
		if(ndig < 19) { mant = mant * 10 + d; if(mant) ndig++; }
		else exp10++;
	}
	if(p < end && *p == '.') {
		p++;
		for(;p < end; p++) {
			u32 d = (u32)*p - '0';
			if(d > 9) break;
			if(ndig < 19) { mant = mant * 10 + d; if(mant) ndig++; exp10--; }
		}
	}
	if(p < end && (*p == 'e' || *p == 'E')) {
		p++;
		b32 eneg = false;
		if(p < end && (*p == '-' || *p == '+')) eneg = (*p++ == '-');
		i32 e = 0;
		for(;p < end; p++) {
			u32 d = (u32)*p - '0';
			if(d > 9) break;
			if(e < 10000) e = e * 10 + (i32)d;
		}
		exp10 += eneg ? -e : e;
	}
 
	if(mant == 0) return neg ? -0.0 : 0.0;
 
	f64 r = mant;
	if(mant <= (1ULL << 53) && exp10 >= -22 && exp10 <= 22) {
		r = (exp10 < 0) ? r / f64_pow10[-exp10] : r * f64_pow10[exp10];
	} else {
		if(exp10 > 400) exp10 = 400;
		if(exp10 < -400) exp10 = -400;
		while(exp10 > 22)  { r *= 1e22; exp10 -= 22; }
		while(exp10 < -22) { r /= 1e22; exp10 += 22; }
		r = (exp10 < 0) ? r / f64_pow10[-exp10] : r * f64_pow10[exp10];
	}
	return neg ? -r : r;
}

u32 u32_from_str(String str) { return u64_from_str(str); };
i32 i32_from_str(String str) { return i64_from_str(str); };
f32 f32_from_str(String str) { return f64_from_str(str); };

////////////////////////////////////////////////////////////////////////
// Great sprintf
struct Writer {
	u8* buf;
	u32 pos;
};

void w_bytes(Writer* w, u8* src, u32 len) {
	if(w->buf) MemCopy(w->buf + w->pos, src, len);
	w->pos += len;
}

void w_byte(Writer* w, u8 b) {
	if(w->buf) w->buf[w->pos] = b;
	w->pos += 1;
}

u32 str_fmt(u8* buf, String fmt, VaList args) {
	Writer w = {buf, 0};
	u8 tmp[64];
	u8* p = fmt.str;
	u8* end = p + fmt.size;
	while(p < end) {
		u8* run = p;
		while(p < end && *p != '%') p++;
		if(p > run) w_bytes(&w, run, p - run);
		if(p == end) break;
		u8* spec_start = p;
		p++; // skip '%'
		i32 prec = -1;
		if(p < end && *p == '.') {
			p++;
			prec = 0;
			for(;p < end && *p - '0' <= 9; p++)
				if(prec < 1000000) prec = prec * 10 + (*p - '0');
		}
		if(p >= end) { // spec cut off by end of format string
			w_bytes(&w, spec_start, end - spec_start);
			break;
		}
		u8 c = *p;
		b32 is64 = false;
		if((c == 'i' || c == 'u') && p + 2 < end && p[1] == '6' && p[2] == '4') {
			is64 = true;
			p += 2;
		}
		switch(c) {
			default: { // unknown: show it, consume nothing
				w_bytes(&w, spec_start, p - spec_start + 1);
			}break;
			case 'i': {
				u32 len = is64 ? i64_write(tmp, va_arg(args, i64)) : i32_write(tmp, va_arg(args, i32));
				w_bytes(&w, tmp, len);
			}break;
			case 'u': {
				u32 len = is64 ? u64_write(tmp, va_arg(args, u64)) : u32_write(tmp, va_arg(args, u32));
				w_bytes(&w, tmp, len);
			}break;
			case 'f': {
				f64 v = va_arg(args, f64);
#if BUILD_DEBUG
				if(is_inf(v)) {
					String inf = "inf";
					w_bytes(&w, inf.str, inf.size); break;
				} else if(is_nan(v)) {
					String nan = "nan";
					w_bytes(&w, nan.str, nan.size); break;
				}
#endif
				u32 len = f64_write(tmp, v, prec < 0 ? 6 : prec);
				w_bytes(&w, tmp, len);
			}break;
			case 's': {
				String s = va_arg(args, String);
				w_bytes(&w, s.str, s.size);
			}break;
			case 'c': {
				u8 ch = va_arg(args, i32);
				w_byte(&w, ch);
			}break;
			case 'p': {
				u64 v = va_arg(args, u64);
				u64_hex_write(tmp, v, true);
				w_bytes(&w, tmp, HEX_LENGTH);
			}break;
			case '%': {
				w_byte(&w, '%');
			}break;
		}
		p++; // past the conversion char
	}
	return w.pos;
}

////////////////////////////////////////////////////////////////////////
// Character Classification & Conversion Functions
b32 char_is_space(u8 c)        { return c == ' ' || c == '\t'; }
b32 char_is_newline(u8 c)      { return c == '\r' || c == '\n'; }
b32 char_is_ws(u8 c)           { return char_is_space(c) || char_is_newline(c); }
b32 char_is_upper(u8 c)        { return 'A' <= c && c <= 'Z'; }
b32 char_is_lower(u8 c)        { return 'a' <= c && c <= 'z'; }
b32 char_is_alpha(u8 c)        { return char_is_upper(c) || char_is_lower(c); }
b32 char_is_slash(u8 c)        { return c == '/' || c == '\\'; }
b32 char_is_digit(u8 c)        { return (c >= '0' && c <= '9'); }
u8 char_to_lower(u8 c)         { if(char_is_upper(c)) { c += ('a' - 'A'); } return c; }
u8 char_to_upper(u8 c)         { if(char_is_lower(c)) { c += ('A' - 'a'); } return c; }
u8 char_to_correct_slash(u8 c) { if(char_is_slash(c)) { c = '/'; } return c; }
b32 char_is_number_cont(u8 c)  { return char_is_digit(c) || c == '.' || c == '-'; }

////////////////////////////////////////////////////////////////////////
// String Constructors
String str_make(Slice<u8> str) {
	String res = String(str.data, str.size);
	return res;
}

String str_range(u8* first, u8* one_past_last) {
	String result = String(first, (u32)(one_past_last - first));
	return result;
}

////////////////////////////////////////////////////////////////////////
// String Stylization
String upper_from_str(Allocator arena, String str) {
	str = push_str_copy(arena, str);
	Loop(i, str.size) {
		str.str[i] = char_to_upper(str.str[i]);
	}
	return str;
}

String lower_from_str(Allocator arena, String str) {
	str = push_str_copy(arena, str);
	Loop(i, str.size) {
		str.str[i] = char_to_lower(str.str[i]);
	}
	return str;
}

////////////////////////////////////////////////////////////////////////
// String Matching
b32 str_match(String str0, String str1) {
	if(str0.size != str1.size) return false;
	Loop(i, str0.size) {
		if(str0.str[i] != str1.str[i]) {
			return false;
		}
	}
	return true;
}

b32 str_matchi(String str0, String str1) {
	if(str0.size != str1.size) return false;
	Loop(i, str0.size) {
		if(char_to_lower(str0.str[i]) != char_to_lower(str1.str[i])) {
			return false;
		}
	}
	return true;
}

u64 str_find_needle(String hay, String needle) {
	if(needle.size == 0 || needle.size > hay.size) return hay.size;
	u8 first_char = needle.str[0];
	u8 last_char = needle.str[needle.size-1];
	u8* p = hay.str;
	u8* end = hay.str + hay.size + 1 - needle.size;
	for(;p < end; p++) {
		if(p[0] == first_char && p[needle.size-1] == last_char) {
			if(str_match(str_range(p, p+needle.size), needle)) {
				return u64(p - hay.str);
			}
		}
	}
	return hay.size;
}

b32 str_ends_with(String string, String end) {
	String postfix = str_postfix(string, end.size);
	b32 is_match = str_match(end, postfix);
	return is_match;
}

////////////////////////////////////////////////////////////////////////
// String Slicing
String str_substr(String str, Rng1u range) {
	range.min = ClampTop(range.min, (u32)str.size);
	range.max = ClampTop(range.max, (u32)str.size);
	str.str += range.min;
	str.size = rng1u_dim(range);
	return str;
}

String str_prefix(String str, u64 size) {
	str.size = ClampTop(size, str.size);
	return str;
}

String str_skip(String str, u64 amt) {
	amt = ClampTop(amt, str.size);
	str.str += amt;
	str.size -= amt;
	return str;
}

String str_postfix(String str, u64 size) {
	size = ClampTop(size, str.size);
	str.str = (str.str + str.size) - size;
	str.size = size;
	return str;
}

String str_chop(String str, u64 amt) {
	amt = ClampTop(amt, str.size);
	str.size -= amt;
	return str;
}

////////////////////////////////////////////////////////////////////////
// String Formatting & Copying
String push_str_cat(Allocator arena, String s1, String s2) {
	String str;
	str.size = s1.size + s2.size;
	str.str = push_array(arena, u8, str.size + 1);
	MemCopy(str.str, s1.str, s1.size);
	MemCopy(str.str + s1.size, s2.str, s2.size);
	str.str[str.size] = 0;
	return str;
}

String push_str_copy(Allocator arena, String s) {
	String str;
	str.size = s.size;
	str.str = push_array(arena, u8, str.size + 1);
	MemCopy(str.str, s.str, s.size);
	str.str[str.size] = 0;
	return str;
}

String push_strfv(Allocator arena, String fmt, VaList args) {
	VaList tmp;
	va_copy(tmp, args); 
	u32 need_bytes = str_fmt(null, fmt, tmp);
	va_end(tmp);
	u8* buf = push_buffer(arena, need_bytes + 1);
	va_copy(tmp, args); 
	u32 final_size = str_fmt(buf, fmt, tmp);
	va_end(tmp);
	String res = {buf, final_size};
	res.str[res.size] = 0;
	return res;
}

String push_strf(Allocator arena, String fmt, ...) {
	VaList argc;
	va_start(argc, fmt);
	String result = push_strfv(arena, fmt, argc);
	va_end(argc);
	return result;
}

////////////////////////////////////////////////////////////////////////
// String List Construction Functions
StringNode* str_list_push_node(StringList& list, StringNode* node) {
	sll_queue_push(list.first, list.last, node);
	list.node_count++;
	list.total_size += node->string.size;
	return node;
}

StringNode* str_list_push_node_set_string(StringList& list, StringNode* node, String string) {
	sll_queue_push(list.first, list.last, node);
	list.node_count++;
	list.total_size += string.size;
	node->string = string;
	return node;
}

StringNode* str_list_push(Allocator arena, StringList& list, String string) {
	StringNode* node = push_struct(arena, StringNode);
	str_list_push_node_set_string(list, node, string);
	return node;
}

StringNode* str_list_pushf(Allocator arena, StringList& list, String fmt, ...) {
	VaList args;
	va_start(args, fmt);
	String string = push_strfv(arena, fmt, args);
	StringNode* result = str_list_push(arena, list, string);
	va_end(args);
	return result;
}

////////////////////////////////////////////////////////////////////////
// String utils
void str_copy(String64& dest, String str) {
	Assert(str.size <= 64);
	MemCopy(dest.str, str.str, str.size);
	dest.size = str.size;
}

String str_next_word(String line, u32& start) {
	// skip spaces
	while(start < line.size && char_is_space(line.str[start]))
		start++;
	u32 token_start = start;
	while(start < line.size && !char_is_space(line.str[start]))
		start++;
	return String(line.str + token_start, start - token_start);
}

String str_trim(String str) {
	u8 *s = str.str, *e = s + str.size;
	while(s < e && char_is_space(*s)) s++;
	while(e > s && char_is_space(*(e - 1))) --e;
	return String(s, u32(e - s));
}

i32 str_index_of_char(String string, u8 c) {
	Loop(i, string.size) {
		if(string.str[i] == c) {
			return i;
		}
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////
// String Path Helpers
u8* str_find_last_slash(String s) {
	u8* p = s.str + s.size - 1;
	for(;p >= s.str; --p) {
		if(char_is_slash(*p)) return p;
	}
	return s.str - 1;
}

String str_chop_last_slash(String s) {
	u8* p = str_find_last_slash(s);
	if(p >= s.str) s.size = u32(p - s.str);
	return s;
}

String str_chop_past_last_slash(String s) {
	u8* p = str_find_last_slash(s);
	if(p >= s.str) s.size = u32(p - s.str) + 1;
	return s;
}

String str_skip_slash(String s) {
	u8* p = s.str;
	for(;p < s.str+s.size; p++) {
		if(char_is_slash(*p)) {
			p++;
			return String(p, u32(s.str + s.size - p));
		}
	}
	return s;
}

String str_skip_last_slash(String s) {
	u8* p = str_find_last_slash(s);
	if(p >= s.str) {
		p += 1;
		s.size = (u32)(s.str + s.size - p);
		s.str = p;
	}
	return s;
}

String str_chop_last_dot(String s) {
	String res = s;
	u32 p = s.size;
	while(p > 0) {
		p -= 1;
		if(s.str[p] == '.') {
			res = str_prefix(s, p);
			break;
		}
	}
	return res;
}

String str_skip_last_dot(String s) {
	u8* ptr = s.str + s.size - 1;
	for(;ptr >= s.str; --ptr) {
		if(*ptr == '.') {
			break;
		}
	}
	if(ptr >= s.str) {
		ptr += 1;
		s.size = (u32)(s.str + s.size - ptr);
		s.str = ptr;
	}
	return s;
}

////////////////////////////////////////////////////////////////////////
// Wchar stuff
u64 wchar_to_char(char* out, const wchar_t* in, u64 out_size) {
	u64 out_len = 0;
	while(*in && out_len < out_size) {
		u16 wc = *in++;
		out[out_len++] = (char)wc;
	}
	if(out_len < out_size)
		out[out_len] = '\0';
	return out_len;
}

String push_str_wchar(Allocator arena, const wchar_t* in, u32 wchar_length) {
	u8* buf = push_buffer(arena, wchar_length + 1);
	Loop(i , wchar_length) {
		buf[i] = in[i];
	}
	buf[wchar_length] = 0;
	return String(buf, wchar_length);
}

////////////////////////////////////////////////////////////////////////
// IdPool
DIdPool id_pool_make(Allocator alloc) {
	DIdPool res = {
		.alloc = alloc,
	};
	return res;
}
u32 id_pool_push(DIdPool& p) {
	if(p.count >= p.cap) {
		if(p.ids) {
			u32 old_cap = p.cap;
			p.cap *= DEFAULT_RESIZE_FACTOR;
			p.ids = mem_realloc_array(p.alloc, p.ids, old_cap, p.cap);
			for(u32 i = old_cap; i < p.cap; i++) {
				p.ids[i] = i;
			}
		} else {
			p.cap = DEFAULT_CAPACITY;
			p.ids = push_array(p.alloc, u32, p.cap);
			for(u32 i = 0; i < p.cap; i++) {
				p.ids[i] = i;
			}
		}
	}
	return p.ids[p.count++];
}
void id_pool_remove(DIdPool& p, u32 id) { p.ids[--p.count] = id; }
void id_pool_destroy(DIdPool& p) { if(p.ids) mem_free(p.alloc, p.ids, p.cap * sizeof(u32)); }
void id_pool_clear(DIdPool& p) { Loop(i, p.cap) p.ids[i] = i; p.count = 0; }

///////////////////////////////////
// Radix
u32 sort_i32_key_to_u32(i32 x) { return x ^ 0x80000000; }
u32 sort_f32_key_to_u32(f32 sort_key) {
	u32 res = bit_cast(u32, sort_key);
	if(res & 0x80000000) {
		res = ~res;
	} else {
		res |= 0x80000000;
	}
	return res;
}

void sort_radix(Slice<SortEntry> arr) {
	Scratch scratch;
	SortEntry* src = arr.data;
	SortEntry* dst = push_array(scratch, SortEntry, arr.size);
	for(u32 shift = 0; shift < 32; shift += 8) {
		u32 counts[256] = {};

		// 1) histogram
		Loop(i, arr.size) {
			u32 byte = (src[i].sort_key >> shift) & 0xFF;
			counts[byte]++;
		}

		// 2) prefix sum (positions)
		u32 sum = 0;
		LoopArray(i, counts) {
			u32 c = counts[i];
			counts[i] = sum;
			sum += c;
		}

		// 3) distribute (stable)
		Loop(i, arr.size) {
			u32 byte = (src[i].sort_key >> shift) & 0xFF;
			dst[counts[byte]++] = src[i];
		}
		Swap(src, dst);
	}
}

void sort_radix_msd(Slice<SortEntry> arr) {
	Scratch scratch;
	SortEntry* src = arr.data;
	SortEntry* dst = push_array(scratch, SortEntry, arr.size);
	for(u32 shift = 0; shift < 32; shift += 8) {
		u32 counts[256] = {};

		Loop(i, arr.size) {
			u32 byte = (src[i].sort_key >> shift) & 0xFF;
			counts[byte]++;
		}

		u32 sum = 0;
		LoopArrayReverse(i, counts) {
			u32 c = counts[i];
			counts[i] = sum;
			sum += c;
		}

		Loop(i, arr.size) {
			u32 byte = (src[i].sort_key >> shift) & 0xFF;
			dst[counts[byte]++] = src[i];
		}
		Swap(src, dst);
	}
}

void sort_radix64(Slice<SortEntry64> arr) {
	Scratch scratch;
	SortEntry64* src = arr.data;
	SortEntry64* dst = push_array(scratch, SortEntry64, arr.size);
	for(u32 shift = 0; shift < 64; shift += 8) {
		u32 counts[256] = {};

		Loop(i, arr.size) {
			u32 byte = (src[i].sort_key >> shift) & 0xFF;
			counts[byte]++;
		}

		u32 sum = 0;
		LoopArray(i, counts) {
			u32 c = counts[i];
			counts[i] = sum;
			sum += c;
		}

		Loop(i, arr.size) {
			u32 byte = (src[i].sort_key >> shift) & 0xFF;
			dst[counts[byte]++] = src[i];
		}
		Swap(src, dst);
	}
}

void sort_radix64_msd(Slice<SortEntry64> arr) {
	Scratch scratch;
	SortEntry64* src = arr.data;
	SortEntry64* dst = push_array(scratch, SortEntry64, arr.size);
	for(u32 shift = 0; shift < 64; shift += 8) {
		u32 counts[256] = {};

		Loop(i, arr.size) {
			u32 byte = (src[i].sort_key >> shift) & 0xFF;
			counts[byte]++;
		}

		u32 sum = 0;
		LoopArrayReverse(i, counts) {
			u32 c = counts[i];
			counts[i] = sum;
			sum += c;
		}

		Loop(i, arr.size) {
			u32 byte = (src[i].sort_key >> shift) & 0xFF;
			dst[counts[byte]++] = src[i];
		}
		Swap(src, dst);
	}
}

u32 tctx_id() { return tctx.id; }

void tctx_init() {
	tctx.arenas[0] = arena_make();
	tctx.arenas[1] = arena_make();
	tctx.id = atomic_inc(&_next_thread_id);
}

Temp tctx_get_scratch() {
	Arena* arena = &tctx.arenas[0];
	return temp_begin(arena);
}

Temp tctx_get_scratch_conflict(Allocator conflict) {
	Arena* arena_conflict = (Arena*)conflict.ctx;
	Arena* arena_result = {};
	if(arena_conflict == &tctx.arenas[0]) {
		arena_result = &tctx.arenas[1];
	} else {
		Assert(arena_conflict == &tctx.arenas[1]);
		arena_result = &tctx.arenas[0];
	}
	return temp_begin(arena_result);
}

Scratch::operator Allocator()        { return {.type = AllocatorType_Arena, .ctx = temp.arena}; }
Scratch::Scratch()                   { temp = tctx_get_scratch(); }
Scratch::Scratch(Allocator conflict) { temp = tctx_get_scratch_conflict(conflict); }
Scratch::~Scratch()                  { temp_end(temp); }

void prof_init(Allocator arena) {
	var& g = prof_st;
	g.arena = arena_make(.lock = true);
	g.gpa = alloc_make(g.arena, .lock = true);
	LoopArray(i, g.prof_threads) {
		var& prof_thread = g.prof_threads[i];
		prof_thread.events[0] = array_make<ProfEvent>(g.gpa);
		prof_thread.events[1] = array_make<ProfEvent>(g.gpa);
		prof_thread.delayed_anchors = array_make<ProfAnchor>(g.gpa);
		prof_thread.launch_anchors = array_make<ProfAnchor>(g.gpa);
		for(var& anchors : prof_thread.recorded_anchors) anchors = array_make<ProfAnchor>(g.gpa);
	}
}

_ProfBlock::_ProfBlock(String label_, String func_, ProfType type_) {
	var& g = prof_st;
	ProfThread& prof_thread = g.prof_threads[tctx_id()];
	u32 label_idx = hash(label_) % ArrayCount(g.strs);
	u32 func_idx = hash(func_) % ArrayCount(g.strs);
	if(!bit_array_get(g.strs_exist, label_idx)) {
		g.strs[label_idx] = push_str_copy(g.arena, label_);
		bit_array_set(g.strs_exist, label_idx);
	}
	if(!bit_array_get(g.strs_exist, func_idx)) {
		g.strs[func_idx] = push_str_copy(g.arena, func_);
		bit_array_set(g.strs_exist, func_idx);
	}
	label = g.strs[label_idx];
	func = g.strs[func_idx];
	type = type_;
	ProfEvent event = {
		.type = ProfEventType_Push,
		.prof_type = type_,
		.tsc = cpu_now(),
		.label = label,
		.func = func,
	};
	array_push(prof_thread.events[atomic_load(&g.current_write)], event);
}

_ProfBlock::~_ProfBlock() {
	var& g = prof_st;
	ProfThread& prof_thread = g.prof_threads[tctx_id()];
	ProfEvent event = {
		.type = ProfEventType_Pop,
		.prof_type = type,
		.tsc = cpu_now(),
		.label = label,
		.func = func,
	};
	array_push(prof_thread.events[atomic_load(&g.current_write)], event);
}

void prof_begin_frame() {
	var& g = prof_st;
	for(var& prof_thread : g.prof_threads) array_clear(prof_thread.events[g.current_write]);
	g.current_frame_time.tsc_start = cpu_now();
}

void prof_end_frame() {
	Scratch scratch;
	var& g = prof_st;
	g.current_frame_time.tsc_end = cpu_now();
	if(g.paused) return;
	g.frames_times[current_frame % ArrayCount(g.frames_times)] = {
		g.current_frame_time.tsc_start,
		g.current_frame_time.tsc_end,
	};
	u32 read_buf = atomic_xor(&g.current_write, 1);
	for(var& prof_thread : g.prof_threads) {
		var& anchors = prof_thread.recorded_anchors[current_frame % ArrayCount(g.frames_times)];
		array_clear(anchors);
		u32 depth = 0;
		var stack = array_make<u32>(scratch);

		///////////////////////////////////
		// Process events
		for(var event : prof_thread.events[read_buf]) switch(event.type) {
			case ProfEventType_Push: {
				ProfAnchor anchor = {
					.type = event.prof_type,
					.label = event.label,
					.func = event.func,
					.depth = depth,
					.tsc_start = event.tsc,
				};
				depth++;

				// In prev frame was push event
				if(prof_thread.delayed_anchors.count) {
					array_push(prof_thread.delayed_anchors, anchor);
				} else {
					u32 idx = array_push(anchors, anchor);
					array_push(stack, idx);
				}
			}break;
			case ProfEventType_Pop: {
				// In prev frame was push event
				if(prof_thread.delayed_anchors.count) {
					ProfAnchor old_anchor = array_pop(prof_thread.delayed_anchors);
					u32 idx = array_push(anchors, old_anchor);
					array_push(stack, idx);
					depth++;
				}

				// FIXME: shouldn't happen 
				if(stack.count == 0) {
					continue;
				}
				u32 anchor_idx = array_pop(stack);
				ProfAnchor& anchor = anchors[anchor_idx];
				anchor.tsc_end = event.tsc;
				u64 elapsed = anchor.tsc_end - anchor.tsc_start;
				if(stack.count) {
					anchors[array_back(stack)].tsc_elapsed_excl -= elapsed;
				}
				anchor.tsc_elapsed_excl += elapsed;
				depth--;
			}break;
		}

		// We save long block time to handle it in next frames
		Loop(i, stack.count) {
			array_push(prof_thread.delayed_anchors, anchors[anchors.count - stack.count + i]);
		}
	}
}

ProfFrame prof_get_prev_frame() {
	var& g = prof_st;
	var& prof_thread = g.prof_threads[tctx_id()];
	ProfFrame result = {
		.frame_time = g.frames_times[(current_frame-1) % ArrayCount(g.frames_times)],
		.anchors = slice(prof_thread.recorded_anchors[(current_frame-1) % ArrayCount(g.frames_times)]),
	};
	return result;
}

void prof_launch_begin() {
	var& g = prof_st;
	g.current_frame_time.tsc_start = cpu_now();
}

void prof_launch_end() {
	Scratch scratch;
	var& g = prof_st;
	g.current_frame_time.tsc_end = cpu_now();
	g.launch_time = g.current_frame_time;
	for(var& prof_thread : g.prof_threads) {
		var& anchors = prof_thread.recorded_anchors[current_frame % ArrayCount(g.frames_times)];
		array_clear(anchors);
		u32 depth = 0;
		var stack = array_make<u32>(scratch);

		///////////////////////////////////
		// Process events
		Loop(i, prof_thread.events[0].count) {
			ProfEvent event = prof_thread.events[0][i];
			switch(event.type) {
				case ProfEventType_Push: {
					ProfAnchor anchor = {
						.type = event.prof_type,
						.label = event.label,
						.func = event.func,
						.depth = depth,
						.tsc_start = event.tsc,
					};
					depth++;
					u32 idx = array_push(anchors, anchor);
					array_push(stack, idx);
				}break;
				case ProfEventType_Pop: {
					u32 anchor_idx = array_pop(stack);
					ProfAnchor& anchor = anchors[anchor_idx];
					anchor.tsc_end = event.tsc;
					u64 elapsed = anchor.tsc_end - anchor.tsc_start;
					if(stack.count) {
						anchors[array_back(stack)].tsc_elapsed_excl -= elapsed;
					}
					anchor.tsc_elapsed_excl += elapsed;
					depth--;
				}break;
			}
		}
		var& launch_anchors = prof_thread.launch_anchors;
		array_reserve(launch_anchors, anchors.count);
		MemCopyArray(launch_anchors.data, anchors.data, anchors.count);
		launch_anchors.count = anchors.count;
	}
}

WaitGroup thread_wg_make(u32 count) {
	var& g = thread_st;
	u32 idx = atomic_inc(&g.wg_write) % Thread_MaxCounters;
	g.wg_slots[idx] = {.count = count};
	return WaitGroup(idx);
}
void thread_decrement_wg(WaitGroup wg) {
	var& g = thread_st;
	var& slot = g.wg_slots[wg.idx];
	if(atomic_dec(&slot.count) == 1) {
		atomic_store(&slot.is_done, 1);
		if(atomic_load(&slot.waiting_count) > 0) {
			os_futex_wake(slot.is_done, U32_MAX);
		}
	}
}

WaitGroup thread_push(TaskDesc desc) {
	var& g = thread_st;
	WaitGroup wg = thread_wg_make(1);
	Task t = {
		.fn = desc.fn,
		.ctx = desc.ctx,
		.wg = wg,
		.priority = desc.priority,
	};
	{
		// LockScope(g.task_mutex);
		queue_push(g.tasks[desc.priority], t);
	}
	if(atomic_load(&g.working_num) < Thread_NumWorkers) {
		os_sem_post(g.tasks_available);
	} else {
		atomic_inc(&g.tasks_available.futex);
	}
	atomic_inc(&g.remaining_tasks);
	return wg;
}
WaitGroup thread_push(void* ctx, TaskFn* fn, TaskPriority prio) { return thread_push({ctx, fn, prio}); }

WaitGroup thread_push_batch(Slice<TaskDesc> tasks) {
	var& g = thread_st;
	u32 count_by_prio[TaskPriority_COUNT] = {};
	Loop(i, tasks.count) count_by_prio[tasks[i].priority]++;
	WaitGroup wg = thread_wg_make(tasks.count);
	Loop(i, tasks.count) {
		Task t = {.fn = tasks[i].fn, .ctx = tasks[i].ctx, .wg = wg, .priority = tasks[i].priority};
		queue_push(g.tasks[tasks[i].priority], t);
	}
	atomic_add(&g.tasks_available.futex, tasks.count);
	atomic_add(&g.remaining_tasks, tasks.count);
	if(atomic_load(&g.working_num) < tasks.count) {
		os_futex_wake(g.tasks_available.futex, U32_MAX);
	}
	return wg;
}

Task thread_pop_locked() {
	var& g = thread_st;
	Task res = {};
	if(!queue_pop(g.tasks[TaskPriority_High], &res)) {
		b32 ok = queue_pop(g.tasks[TaskPriority_Low], &res); Assert(ok);
	}
	return res;
}

Task thread_pop() {
	var& g = thread_st;
	ProfBlock("sleep", ProfType_Sleep);
	atomic_dec(&g.working_num);
	os_sem_wait(thread_st.tasks_available);
	atomic_inc(&g.working_num);
	return thread_pop_locked();
}

b32 thread_try_pop(Task* out) {
	var& g = thread_st;
	if(os_sem_try_wait(g.tasks_available)) {
		*out = thread_pop_locked();
		return true;
	}
	return false;
}

void thread_worker(void* ctx) {
	var& g = thread_st;
	tctx_init();
	For {
		Task t = thread_pop();
		ProfBlock("working", t.priority == TaskPriority_High ? ProfType_Worker : ProfType_Async);
		t.fn(t.ctx);
		if(atomic_dec(&g.remaining_tasks) == 1) {
			if(atomic_load(&g.is_waiting_remaning_tasks) == 1) {
				os_futex_wake(g.is_waiting_remaning_tasks, 1);
			}
		}
		thread_decrement_wg(t.wg);
	}
}

void thread_wg_wait(WaitGroup wg) {
	ProfFunc;
	var& g = thread_st;
	var& slot = g.wg_slots[wg.idx];
	while(atomic_load(&slot.is_done) == 0) {
		Task t = {};
		if(thread_try_pop(&t)) {
			ProfBlock("Working", ProfType_Worker);
			t.fn(t.ctx);
			atomic_dec(&g.remaining_tasks);
			thread_decrement_wg(t.wg);
			continue;
		}
		atomic_inc(&slot.waiting_count);
		os_futex_wait(slot.is_done, 0);
	}
}

void thread_wait_remanings() {
	var& g = thread_st;
	atomic_store(&g.is_waiting_remaning_tasks, 1);
	if(atomic_load(&g.remaining_tasks)) {
		os_futex_wait(g.is_waiting_remaning_tasks, 1);
	}
	atomic_store(&g.is_waiting_remaning_tasks, 0);
}

u8* _thread_push_ctx(u64 size, u64 align) {
	var& g = thread_st;
	os_mutex_lock(g.ctx_mutex);
	g.ctx_write = align_up(g.ctx_write, align) % sizeof(g.ctx_buffer);
	if(g.ctx_write + size > sizeof(g.ctx_buffer)) {
		g.ctx_write = 0;
	}
	u8* res = &g.ctx_buffer[g.ctx_write];
	g.ctx_write += size;
	os_mutex_unlock(g.ctx_mutex);
	MemZero(res, size);
	return res;
}

void thread_pool_init() {
	ProfFunc;
	var& g = thread_st;
	g.arena = arena_make();
	g.working_num = Thread_NumWorkers;
	for(var& q : g.tasks) {
		q = queue_mpmc_make<Task, Thread_MaxTasks>();
	}
	Loop(i, Thread_NumWorkers) {
		g.threads[i] = os_thread_make(thread_worker, null);
	}
}

typedef enum VkDebugUtilsMessageSeverityFlagBitsEXT {
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT = 0x00000001,
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT = 0x00000010,
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT = 0x00000100,
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT = 0x00001000,
	VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT = 0x7FFFFFFF
} VkDebugUtilsMessageSeverityFlagBitsEXT;
typedef u32 VkDebugUtilsMessageTypeFlagsEXT;
typedef struct VkDebugUtilsMessengerCallbackDataEXT {
	u8 padd[40];
	const char* pMessage;
} VkDebugUtilsMessengerCallbackDataEXT;
typedef b32 VkBool32;
VkBool32 vk_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_types, const VkDebugUtilsMessengerCallbackDataEXT* callback_data, void* user_data) {
	switch(message_severity) {
		default:break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: {
			Trace(String(callback_data->pMessage));
		}break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: {
			Info(String(callback_data->pMessage));
		}break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: {
			String skip_warnings[] = {
				"vkCreateGraphicsPipelines(): pCreateInfos[0].pVertexInputState Vertex attribute at location",
				"vkCreateGraphicsPipelines(): pCreateInfos[0] (SPIR-V Interface) VK_SHADER_STAGE_VERTEX_BIT has an Output value declared at Location",
				"(Warning - This VUID has now been reported 10 times, which is the duplicate_message_limit value, this will be the last time reporting it).",
				"vkCreateGraphicsPipelines(): pCreateInfos[0] (SPIR-V Interface) [EntryPoint \"vs_main\", VK_SHADER_STAGE_VERTEX_BIT] has an Output value declared at Location",
			};
			for(var skip : skip_warnings) {
				String warn = callback_data->pMessage;
				if(warn.size >= skip.size) {
					if(str_match(str_prefix(warn, skip.size), skip)) {
						return false;
					}
				}
			}
			Warn(String(callback_data->pMessage));
		}break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: {
			Error(String(callback_data->pMessage));
		}break;
	}
	return false;
}


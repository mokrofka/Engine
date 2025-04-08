#pragma once

#include "defines.h"

#include <math/math_types.h>

struct Range {
  i32 min;
  i32 max;
};

KAPI b8 char_is_space(u8 c);
KAPI b8 char_is_upper(u8 c);
KAPI b8 char_is_lower(u8 c);
KAPI b8 char_is_alpha(u8 c);
KAPI b8 char_is_slash(u8 c);
KAPI u8 char_to_lower(u8 c);
KAPI u8 char_to_upper(u8 c);
KAPI u8 char_to_correct_slash(u8 c);

KAPI u64 cstr_length(u8 *c);

#define str_lit(S) str((u8*)(S), sizeof(S) - 1)

KAPI String str(u8* str, u64 size);
KAPI String str_range(u8 *first, u8 *one_past_last);
KAPI String str_zero();

u8* str_format_v(void* buffer, const void* format, void* va_listp);
KAPI u8* str_format(void* buffer, const void* format, ...);

KAPI String cstr(const void* c);
KAPI String cstr_capped(void* cstr, void* cap);
KAPI b8 cstr_equal(const void* str0, const void* str1);
KAPI b8 cstr_equali(const void* str0, const void* str1);

KAPI String push_str_cat(struct Arena* arena, String s1, String s2);
KAPI String push_str_copy(struct Arena *arena, String s);

KAPI String str_chop_last_segment(String string);
KAPI String str_chop_last_slash(String string);
KAPI String str_skip_last_slash(String string);
KAPI String str_chop_last_dot(String string);

KAPI String str_trim(String string);

KAPI i32 str_index_of(String str, u8 c);
KAPI i32 cstr_index_of(const char* str, char c);

KAPI b8 str_to_v4(const char* str, v4* out_vector);
KAPI b8 str_to_v3(const char* str, v3* out_vector);
KAPI b8 str_to_v2(const char* str, v2* out_vector);
KAPI b8 str_to_f32(const char* str, f32* f);
KAPI b8 str_to_f64(const char* str, f64* f);

KAPI b8 str_to_i8(const char* str, i8* f);
KAPI b8 str_to_i16(const char* str, i16* f);
KAPI b8 str_to_i32(const char* str, i32* f);
KAPI b8 str_to_i64(const char* str, i64* f);

KAPI b8 str_to_u8(const char* str, u8* f);
KAPI b8 str_to_u16(const char* str, u16* f);
KAPI b8 str_to_u32(const char* str, u32* f);
KAPI b8 str_to_u64(const char* str, u64* f);

KAPI b8 str_to_bool(const char* str, b8* f);

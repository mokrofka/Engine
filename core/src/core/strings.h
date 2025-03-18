#pragma once

#include "defines.h"

struct String {
  u8* str;
  u64 size;
};

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

KAPI u64 cstring8_length(u8 *c);

#define str8_lit(S)  str8((u8*)(S), sizeof(S) - 1)

KAPI String str8(u8* str, u64 size);
KAPI String str8_range(u8 *first, u8 *one_past_last);
KAPI String str8_zero();
KAPI String cstr8(const char* c);
KAPI String cstr8_capped(void *cstr, void *cap);

KAPI String push_str8_cat(struct Arena* arena, String s1, String s2);
KAPI String push_str8_copy(struct Arena *arena, String s);

KAPI String str8_chop_last_segment(String string);
KAPI String str8_chop_last_slash(String string);
KAPI String str8_skip_last_slash(String string);
KAPI String str8_chop_last_dot(String string);

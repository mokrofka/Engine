#pragma once
#include "defines.h"
#include "math_types.h"
#include "memory.h"

struct Range {
  i32 min;
  i32 max;
};

struct StringCursor {
  u8* at;
  u8* end;
};

struct StringArray {
  String *v;
  u64 count;
};

struct StringNode {
  StringNode* next;
  String string;
};

struct StringList {
  StringNode* first;
  StringNode* last;
  u64 node_count;
  u64 total_size;
};

struct String64 {
  u8 str[64];
  u64 size;
  Inline operator String() {
    return String{str, size};
  }
};

Inline u32 range_size(Range r) {
  u32 c = ((r.max > r.min) ? (r.max - r.min) : 0);
  return c;
}

////////////////////////////////
// Character Classification & Conversion Functions

Inline b32 char_is_space(u8 c) { return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v'; }
Inline b32 char_is_upper(u8 c) { return 'A' <= c && c <= 'Z'; }
Inline b32 char_is_lower(u8 c) { return 'a' <= c && c <= 'z'; }
Inline b32 char_is_alpha(u8 c) { return char_is_upper(c) || char_is_lower(c); }
Inline b32 char_is_slash(u8 c) { return c == '/' || c == '\\'; }
Inline u8 char_to_lower(u8 c) { if (char_is_upper(c)) { c += ('a' - 'A'); } return c; }
Inline u8 char_to_upper(u8 c) { if (char_is_lower(c)) { c += ('A' - 'a'); } return c; }
Inline u8 char_to_correct_slash(u8 c) { if (char_is_slash(c)) { c = '/'; } return c; }

////////////////////////////////
// C-String Measurement, Functions

Inline u64 cstr_length(void* c);
Inline b32 _strcmp(const void* str0, const void* str1);
Inline b32 cstr_match(const void* str0, const void* str1);
       b32 _strcmpi(const void* str0, const void* str1);
Inline b32 cstr_matchi(const void* str0, const void* str1);

////////////////////////////////
// String Constructors

#define str_lit(S) str((u8*)(S), sizeof(S) - 1) // deprecated

Inline String str(u8* str, u64 size);
u32 my_vsnprintf(void* buffer, u32 buffer_size, const void* format, void* argc_);

constexpr Inline String operator""_(const char* string, u64 len) {
  return str((u8*)string, len);
}

//        hello_world
//  first = &l
// one_past_last &r
// String: lo_wor
Inline String str_range(u8* first, u8* one_past_last);

Inline String str_zero();
Inline String str_cstr(const void* c);

String str_cstr_capped(const void* str_cstr, const void* cap);

////////////////////////////////
// String Stylization

String upper_from_str(Arena *arena, String string);
String lower_from_str(Arena *arena, String string);

////////////////////////////////
// String Matching

b32 str_match(String str0, String str1);
b32 str_matchi(String str0, String str1);
b32 str_ends_with(String string, String end);

////////////////////////////////
// String Slicing

// hello_world
// range {1, 4}
// String: ello
Inline String str_substr(String str, Range range);

// hello_world
// size 3
// String: hel
Inline String str_prefix(String str, u64 size);

// hello_world
// size 6
// String: world
Inline String str_skip(String str, u64 amt);

// hello_world
// size 4
// String: orld
Inline String str_postfix(String str, u64 size);

// hello_world
// size 3
// String: hello_wo
Inline String str8_chop(String str, u64 amt);

////////////////////////////////
// String Formatting & Copying

String push_str_cat(Arena* arena, String s1, String s2);
String push_str_copy(Arena *arena, String s);
String push_strfv(Arena* arena, const void* fmt, void* argc);
String push_strf(Arena* arena, const void* fmt, ...);

////////////////////////////////
// String some random stuff
void str_copy(String64& dest, String str);
String str_read_line(StringCursor* cursor);
String str_trim(String string);
i32 str_index_of(String str, u8 c);

////////////////////////////////
// String <=> Integer Conversions

// string -> integer
b32 str_to_v4(void* str, v4* out_vector);
b32 str_to_v3(const char* str, v3* out_vector);
b32 str_to_v2(const char* str, v2* out_vector);
b32 str_to_f32(const char* str, f32* f);
b32 str_to_f64(const char* str, f64* f);

b32 str_to_i8(const char* str, i8* f);
b32 str_to_i16(const char* str, i16* f);
b32 str_to_i32(const char* str, i32* f);
b32 str_to_i64(const char* str, i64* f);

b32 str_to_u8(const char* str, u8* f);
b32 str_to_u16(const char* str, u16* f);
b32 str_to_u32(const char* str, u32* f);
b32 str_to_u64(const char* str, u64* f);

b32 str_to_bool(const char* str, b8* f);

// integer -> string
// ...

////////////////////////////////
// String Path Helpers

String str_chop_after_last_slash(String string);
String str_chop_last_slash(String string);
String str_skip_last_slash(String string);
String str_chop_last_dot(String string);

////////////////////////////////

Inline u64 cstr_length(void* c) {
  u8* p = (u8*)c;
  for (; *p != 0; p += 1);
  return p - (u8*)c;
}

Inline b32 _strcmp(const void* str0, const void* str1) {
  u8* s0 = (u8*)str0;
  u8* s1 = (u8*)str1;
  while (*s0 && (*s0 == *s1)) { ++s0; ++s1; }
  return *s0 - *s1;
}

Inline b32 cstr_match(const void* str0, const void* str1) {
  return _strcmp(str0, str1) == 0;
}

Inline b32 cstr_matchi(const void* str0, const void* str1) {
  return _strcmpi((char*)str0, (char*)str1) == 0;
}

Inline String str(u8* str, u64 size) {
  String result = {str, size};
  return result;
}

Inline String str_range(u8* first, u8* one_past_last) {
  String result = {first, (u64)(one_past_last - first)};
  return result;
}

Inline String str_zero() {
  String result = {};
  return result;
}

Inline String str_cstr(const void* c) {
  String result = {(u8*)c, cstr_length((u8*)c)};
  return result;
}

Inline String str_substr(String str, Range range) {
  range.min = ClampTop(range.min, str.size);
  range.max = ClampTop(range.max, str.size);
  str.str += range.min;
  str.size = range_size(range);
  return str;
}

Inline String str_prefix(String str, u64 size) {
  str.size = ClampTop(size, str.size);
  return str;
}

Inline String str_skip(String str, u64 amt) {
  amt = ClampTop(amt, str.size);
  str.str += amt;
  str.size -= amt;
  return str;
}

Inline String str_postfix(String str, u64 size) {
  size = ClampTop(size, str.size);
  str.str = (str.str + str.size) - size;
  str.size = size;
  return str;
}

Inline String str8_chop(String str, u64 amt) {
  amt = ClampTop(amt, str.size);
  str.size -= amt;
  return str;
}

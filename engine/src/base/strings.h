#pragma once
#include "defines.h"
#include "math_types.h"
#include "memory.h"

INLINE u64 cstr_length(void* c);
struct String {
  u8* str;
  u64 size;
  String() = default;
  INLINE String(u8* str_, u64 size_) {
    str = str_;
    size = size_;
  }
  constexpr INLINE String(const char* str_) : str((u8*)str_), size(cstr_length((void*)str_)) {}
  INLINE operator bool() {
    return str;
  }
};

struct StringCursor { // TODO replace StringCursor by Range
  u8* at;
  u8* end;
};

struct StringArray {
  String *v;
  u32 count;
};

struct StringNode {
  StringNode* next;
  String string;
};

struct StringList {
  StringNode* first;
  StringNode* last;
  u32 node_count;
  u32 total_size;
};

struct String64 {
  u8 str[64];
  u64 size;
  INLINE operator String() {
    return String{str, size};
  }
  INLINE operator bool() {
    return size;
  }
};

INLINE u32 range_size(Range r) {
  u32 c = ((r.size > r.offset) ? (r.size - r.offset) : 0);
  return c;
}

////////////////////////////////
// Character Classification & Conversion Functions

INLINE b32 char_is_space(u8 c) { return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v'; }
INLINE b32 char_is_upper(u8 c) { return 'A' <= c && c <= 'Z'; }
INLINE b32 char_is_lower(u8 c) { return 'a' <= c && c <= 'z'; }
INLINE b32 char_is_alpha(u8 c) { return char_is_upper(c) || char_is_lower(c); }
INLINE b32 char_is_slash(u8 c) { return c == '/' || c == '\\'; }
INLINE u8 char_to_lower(u8 c) { if (char_is_upper(c)) { c += ('a' - 'A'); } return c; }
INLINE u8 char_to_upper(u8 c) { if (char_is_lower(c)) { c += ('A' - 'a'); } return c; }
INLINE u8 char_to_correct_slash(u8 c) { if (char_is_slash(c)) { c = '/'; } return c; }

////////////////////////////////
// C-String Measurement, Functions

INLINE u64 cstr_length(void* c);
INLINE b32 _strcmp(const void* str0, const void* str1);
INLINE b32 cstr_match(const void* str0, const void* str1);
       b32 _strcmpi(const void* str0, const void* str1);
INLINE b32 cstr_matchi(const void* str0, const void* str1);

////////////////////////////////
// String Constructors

#define str_lit(S) str((u8*)(S), sizeof(S) - 1)

INLINE String str(u8* str, u32 size);

//        hello_world
//  first = &l
// one_past_last &r
// String: lo_wor
INLINE String str_range(u8* first, u8* one_past_last);

INLINE String str_zero();
INLINE String str_cstr(const void* c);

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
INLINE String str_substr(String str, Range range);

// hello_world
// size 3
// String: hel
INLINE String str_prefix(String str, u32 size);

// hello_world
// size 6
// String: world
INLINE String str_skip(String str, u32 amt);

// hello_world
// size 4
// String: orld
INLINE String str_postfix(String str, u32 size);

// hello_world
// size 3
// String: hello_wo
INLINE String str_chop(String str, u32 amt);

////////////////////////////////
// String Formatting & Copying

String push_str_cat(Arena* arena, String s1, String s2);
KAPI String push_str_copy(Arena *arena, String s);
KAPI String push_strfv(Arena* arena, const void* fmt, void* argc);
KAPI String push_strf(Arena* arena, const void* fmt, ...);

////////////////////////////////
// String some random stuff
KAPI void str_copy(String64& dest, String str);
String str_read_line(StringCursor* cursor);
String new_str_read_line(StringCursor* cursor);
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
// wchar stuff

String push_str_wchar(Arena* arena, const wchar_t* in, u32 wchar_size);

////////////////////////////////

INLINE u64 cstr_length(void* c) {
  u8* p = (u8*)c;
  for (; *p != 0; p += 1);
  return (u64)(p - (u8*)c);
}

INLINE b32 _strcmp(const void* str0, const void* str1) {
  u8* s0 = (u8*)str0;
  u8* s1 = (u8*)str1;
  while (*s0 && (*s0 == *s1)) { ++s0; ++s1; }
  return *s0 - *s1;
}

INLINE b32 cstr_match(const void* str0, const void* str1) {
  return _strcmp(str0, str1) == 0;
}

INLINE b32 cstr_matchi(const void* str0, const void* str1) {
  return _strcmpi((char*)str0, (char*)str1) == 0;
}

INLINE String str(u8* str, u32 size) {
  String result = {str, size};
  return result;
}

INLINE String str_range(u8* first, u8* one_past_last) {
  String result = {first, (u32)(one_past_last - first)};
  return result;
}

INLINE String str_zero() {
  String result = {};
  return result;
}

INLINE String str_cstr(const void* c) {
  String result = {(u8*)c, cstr_length((u8*)c)};
  return result;
}

INLINE String str_substr(String str, Range range) {
  range.offset = ClampTop(range.offset, str.size);
  range.size = ClampTop(range.size, str.size);
  str.str += range.offset;
  str.size = range_size(range);
  return str;
}

INLINE String str_prefix(String str, u32 size) {
  str.size = ClampTop(size, str.size);
  return str;
}

INLINE String str_skip(String str, u32 amt) {
  amt = ClampTop(amt, str.size);
  str.str += amt;
  str.size -= amt;
  return str;
}

INLINE String str_postfix(String str, u32 size) {
  size = ClampTop(size, str.size);
  str.str = (str.str + str.size) - size;
  str.size = size;
  return str;
}

INLINE String str_chop(String str, u32 amt) {
  amt = ClampTop(amt, str.size);
  str.size -= amt;
  return str;
}

inline size_t wchar_to_char(char* out, const wchar_t* in, size_t out_size) {
  size_t out_len = 0;

  while (*in && out_len < out_size) {
    u16 wc = *in++;
    out[out_len++] = (char)wc;
  }

  if (out_len < out_size)
    out[out_len] = '\0'; // null-terminate

  return out_len;
}

inline String push_str_wchar(Arena* arena, const wchar_t* in, u32 wchar_length) {
  u8* buff = push_buffer(arena, wchar_length + 1);
  Loop (i , wchar_length) {
    buff[i] = in[i];
  }
  buff[wchar_length] = 0;
  return {buff, wchar_length};
}

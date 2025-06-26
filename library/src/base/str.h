#pragma once

INLINE u64 cstr_length(const void* c) {
  const u8* p = (u8*)c;
  for (; *p != 0; ++p);
  return (u64)(p - (u8*)c);
}

struct String {
  u8* str;
  u64 size;
  String() = default;
  INLINE String(u8* str_, u64 size_) {
    str = str_;
    size = size_;
  }
  INLINE String(const char* str_) : str((u8*)str_), size(cstr_length(str_)) {}
  INLINE String(u8* str_) : str((u8*)str_), size(cstr_length(str_)) {}
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
  u64 count;
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
};

INLINE u32 range_size(Range r) {
  u32 c = ((r.size > r.offset) ? (r.size - r.offset) : 0);
  return c;
}

////////////////////////////////////////////////////////////////////////
// Character Classification & Conversion Functions

INLINE b32 char_is_space(u8 c) { return c == ' '; }
INLINE b32 char_is_upper(u8 c) { return 'A' <= c && c <= 'Z'; }
INLINE b32 char_is_lower(u8 c) { return 'a' <= c && c <= 'z'; }
INLINE b32 char_is_alpha(u8 c) { return char_is_upper(c) || char_is_lower(c); }
INLINE b32 char_is_slash(u8 c) { return c == '/' || c == '\\'; }
INLINE u8 char_to_lower(u8 c) { if (char_is_upper(c)) { c += ('a' - 'A'); } return c; }
INLINE u8 char_to_upper(u8 c) { if (char_is_lower(c)) { c += ('A' - 'a'); } return c; }
INLINE u8 char_to_correct_slash(u8 c) { if (char_is_slash(c)) { c = '/'; } return c; }

////////////////////////////////////////////////////////////////////////
// String Constructors

// #define String(S) String((u8*)(S), sizeof(S) - 1)

//        hello_world
// first = &e
// one_past_last = &r
// String: ello_wor
KAPI String str_range(u8* first, u8* one_past_last);
KAPI String str_cstr_capped(const void* String, const void* cap);

////////////////////////////////////////////////////////////////////////
// String Stylization

KAPI String upper_from_str(Arena *arena, String string);
KAPI String lower_from_str(Arena *arena, String string);

////////////////////////////////////////////////////////////////////////
// String Matching

KAPI b32 str_match(String str0, String str1);
KAPI b32 str_matchi(String str0, String str1);
KAPI b32 str_ends_with(String string, String end);

////////////////////////////////////////////////////////////////////////
// String Slicing

// hello_world
// range {1, 4}
// String: ello
KAPI String str_substr(String str, Range range);

// hello_world
// size 3
// String: hel
KAPI String str_prefix(String str, u64 size);

// hello_world
// size 6
// String: world
KAPI String str_skip(String str, u64 amt);

// hello_world
// size 4
// String: orld
KAPI String str_postfix(String str, u64 size);

// hello_world
// size 3
// String: hello_wo
KAPI String str_chop(String str, u64 amt);

////////////////////////////////////////////////////////////////////////
// String Formatting & Copying

KAPI String push_str_cat(Arena* arena, String s1, String s2);
KAPI String push_str_copy(Arena *arena, String s);
KAPI String push_strfv(Arena* arena, String fmt, void* argc);
KAPI String push_strf(Arena* arena, String fmt, ...);

////////////////////////////////////////////////////////////////////////
// String utils
KAPI void str_copy(String64& dest, String str);
KAPI String str_read_line(StringCursor* cursor);
KAPI String str_trim(String string);
KAPI i32 str_index_of(String str, u8 c);

////////////////////////////////////////////////////////////////////////
// String <=> Integer Conversions
// ...

// string -> integer
KAPI b32 str_to_v4(void* str, v4* out_vector);
KAPI b32 str_to_v3(const char* str, v3* out_vector);
KAPI b32 str_to_v2(const char* str, v2* out_vector);
KAPI b32 str_to_f32(const char* str, f32* f);
KAPI b32 str_to_f64(const char* str, f64* f);

KAPI b32 str_to_i8(const char* str, i8* f);
KAPI b32 str_to_i16(const char* str, i16* f);
KAPI b32 str_to_i32(const char* str, i32* f);
KAPI b32 str_to_i64(const char* str, i64* f);

KAPI b32 str_to_u8(const char* str, u8* f);
KAPI b32 str_to_u16(const char* str, u16* f);
KAPI b32 str_to_u32(const char* str, u32* f);
KAPI b32 str_to_u64(const char* str, u64* f);

KAPI b32 str_to_bool(const char* str, b8* f);

// integer -> string
// ...

////////////////////////////////////////////////////////////////////////
// String Path Helpers

// one/two/three -> one/two/
KAPI String str_chop_after_last_slash(String string);

// one/two/three -> one/two
KAPI String str_chop_last_slash(String string);

// one/two/three -> three
String str_skip_last_slash(String string);
KAPI String str_chop_last_dot(String string);

////////////////////////////////////////////////////////////////////////
// Wchar stuff

KAPI size_t wchar_to_char(char* out, const wchar_t* in, size_t out_size);
KAPI String push_str_wchar(Arena* arena, const wchar_t* in, u32 wchar_length);

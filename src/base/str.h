#pragma once
#include "base.h"
#include "mem.h"

////////////////////////////////////////////////////////////////////////
// Base

struct String {
  u8* str;
  u64 size;
  String() = default;
  NO_DEBUG String(u8* str_, u64 size_);
  NO_DEBUG String(const char* str_);
  NO_DEBUG String(u8* str_);
};

struct StringArray {
  String* v;
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
  u32 size;
};

KAPI u64 cstr_length(const void* c);

////////////////////////////////////////////////////////////////////////
// Character Classification & Conversion Functions

KAPI b32 char_is_space(u8 c);
KAPI b32 char_is_upper(u8 c);
KAPI b32 char_is_lower(u8 c);
KAPI b32 char_is_alpha(u8 c);
KAPI b32 char_is_slash(u8 c);
KAPI b32 char_is_digit(u8 c);
KAPI u8 char_to_lower(u8 c);
KAPI u8 char_to_upper(u8 c);
KAPI u8 char_to_correct_slash(u8 c);
KAPI b32 char_is_number_cont(u8 c);

////////////////////////////////////////////////////////////////////////
// String Constructors

//        hello_world
// first = e
// one_past_last = r
// result: ello_wor
KAPI String str_range(u8* first, u8* one_past_last);
KAPI String str_cstr_capped(const void* String, const void* cap);

////////////////////////////////////////////////////////////////////////
// String Stylization

KAPI String upper_from_str(Allocator arena, String string);
KAPI String lower_from_str(Allocator arena, String string);

////////////////////////////////////////////////////////////////////////
// String Matching

KAPI b32 str_match(String str0, String str1);
KAPI b32 str_matchi(String str0, String str1);
KAPI b32 str_ends_with(String string, String end);
KAPI b32 equal(String a, String b);

////////////////////////////////////////////////////////////////////////
// String Slicing

// hello_world
// range {1, 4}
// result: ello
KAPI String str_substr(String str, Range range);

// hello_world
// size 3
// result: hel
KAPI String str_prefix(String str, u64 size);

// hello_world
// size 6
// result: world
KAPI String str_skip(String str, u64 amt);

// hello_world
// size 4
// result: orld
KAPI String str_postfix(String str, u64 size);

// hello_world
// size 3
// result: hello_wo
KAPI String str_chop(String str, u64 amt);

////////////////////////////////////////////////////////////////////////
// String Formatting & Copying

KAPI String push_str_cat(Allocator arena, String s1, String s2);
KAPI String push_str_copy(Allocator arena, String s);
KAPI String push_strfv(Allocator arena, String fmt, va_list argc);
KAPI String push_strf(Allocator arena, String fmt, ...);

////////////////////////////////////////////////////////////////////////
// String List Construction Functions

KAPI StringNode* str_list_push_node(StringList* list, StringNode* node);
KAPI StringNode* str_list_push(Allocator arena, StringList* list, String string);
KAPI StringNode* str_list_pushf(Allocator arena, StringList* list, String fmt, ...);

////////////////////////////////////////////////////////////////////////
// String utils

KAPI void str_copy(String64& dest, String str);
KAPI String str_next_word(String line, u32& start);
KAPI String str_read_line(Range* range);
KAPI String str_trim(String string);
KAPI i32 str_index_of(String str, u8 c);

////////////////////////////////////////////////////////////////////////
// String <=> Integer Conversions
// ...

KAPI u64 u64_from_str(String str);
KAPI i64 i64_from_str(String str);
KAPI f64 f64_from_str(String str);
INLINE u32 u32_from_str(String str) { return u64_from_str(str); };
INLINE i32 i32_from_str(String str) { return i64_from_str(str); };
INLINE f32 f32_from_str(String str) { return f64_from_str(str); };

////////////////////////////////////////////////////////////////////////
// String Path Helpers

// one/two/three -> one/two/
KAPI String str_chop_after_last_slash(String string);

// one/two/three -> one/two
KAPI String str_chop_last_slash(String string);

// one/two/three -> three
KAPI String str_skip_last_slash(String string);

// name.fmt -> name
KAPI String str_chop_last_dot(String string);

// name.fmt -> fmt
KAPI String str_skip_last_dot(String string);

////////////////////////////////////////////////////////////////////////
// Wchar stuff

KAPI u64 wchar_to_char(char* out, const wchar_t* in, u64 out_size);
KAPI String push_str_wchar(Allocator arena, const wchar_t* in, u32 wchar_length);

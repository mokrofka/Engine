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

struct DString {
  u8* str;
  u32 size;
  u32 cap;
  Allocator alloc;
  void init(Allocator alloc_);
  void add(String str);
  void clear();
  operator String();
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
  operator String();
};

u64 cstr_length(const void* c);

////////////////////////////////////////////////////////////////////////
// Character Classification & Conversion Functions

b32 char_is_space(u8 c);
b32 char_is_upper(u8 c);
b32 char_is_lower(u8 c);
b32 char_is_alpha(u8 c);
b32 char_is_slash(u8 c);
b32 char_is_digit(u8 c);
u8 char_to_lower(u8 c);
u8 char_to_upper(u8 c);
u8 char_to_correct_slash(u8 c);
b32 char_is_number_cont(u8 c);

////////////////////////////////////////////////////////////////////////
// String Constructors

//        hello_world
// first = e
// one_past_last = r
// result: ello_wor
String str_range(u8* first, u8* one_past_last);
String str_cstr_capped(const void* String, const void* cap);

////////////////////////////////////////////////////////////////////////
// String Stylization

String upper_from_str(Allocator arena, String string);
String lower_from_str(Allocator arena, String string);

////////////////////////////////////////////////////////////////////////
// String Matching

b32 str_match(String str0, String str1);
b32 str_matchi(String str0, String str1);
b32 str_ends_with(String string, String end);
b32 equal(String a, String b);

////////////////////////////////////////////////////////////////////////
// String Slicing

// hello_world
// range {1, 4}
// result: ello
String str_substr(String str, Range range);

// hello_world
// size 3
// result: hel
String str_prefix(String str, u64 size);

// hello_world
// size 6
// result: world
String str_skip(String str, u64 amt);

// hello_world
// size 4
// result: orld
String str_postfix(String str, u64 size);

// hello_world
// size 3
// result: hello_wo
String str_chop(String str, u64 amt);

////////////////////////////////////////////////////////////////////////
// String Formatting & Copying

String push_str_cat(Allocator arena, String s1, String s2);
String push_str_copy(Allocator arena, String s);
String push_strfv(Allocator arena, String fmt, VaList argc);
String push_strf(Allocator arena, String fmt, ...);

////////////////////////////////////////////////////////////////////////
// String List Construction Functions

StringNode* str_list_push_node(StringList* list, StringNode* node);
StringNode* str_list_push(Allocator arena, StringList* list, String string);
StringNode* str_list_pushf(Allocator arena, StringList* list, String fmt, ...);

////////////////////////////////////////////////////////////////////////
// String utils

void str_copy(String64& dest, String str);
String str_next_word(String line, u32& start);
String str_read_line(Range* range);
String str_trim(String string);
i32 str_index_of(String str, u8 c);

////////////////////////////////////////////////////////////////////////
// String <=> Integer Conversions
// ...

u64 u64_from_str(String str);
i64 i64_from_str(String str);
f64 f64_from_str(String str);
INLINE u32 u32_from_str(String str) { return u64_from_str(str); };
INLINE i32 i32_from_str(String str) { return i64_from_str(str); };
INLINE f32 f32_from_str(String str) { return f64_from_str(str); };

////////////////////////////////////////////////////////////////////////
// String Path Helpers

// one/two/three -> one/two/
String str_chop_after_last_slash(String string);

// one/two/three -> one/two
String str_chop_last_slash(String string);

// one/two/three -> three
String str_skip_last_slash(String string);

// name.fmt -> name
String str_chop_last_dot(String string);

// name.fmt -> fmt
String str_skip_last_dot(String string);

////////////////////////////////////////////////////////////////////////
// Wchar stuff

u64 wchar_to_char(char* out, const wchar_t* in, u64 out_size);
String push_str_wchar(Allocator arena, const wchar_t* in, u32 wchar_length);

struct Lexer {
  u8* cur;
  u8* end;
};

Lexer lexer_init(String buffer);
String lexer_next_token(Lexer* l);
String lexer_next_integer(Lexer* l);


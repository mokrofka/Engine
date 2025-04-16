#include "lib.h"

#include <stdarg.h>
#include <stdio.h>

////////////////////////////////
// C-String Measurement, Functions

b32 _strcmpi(const void* str0, const void* str1) {
  u8* str_a = (u8*)str0;
  u8* str_b = (u8*)str1;
  while (*str_a && *str_b) {
    u8 s0 = *str_a;
    u8 s1 = *str_b;
    s0 = char_to_lower(s0);
    s1 = char_to_lower(s1);

    if (s0 != s1) {
      return s0 - s1;
    }

    ++str_a;
    ++str_b;
  }

  return str_a - str_b;
}

////////////////////////////////
// String Constructors

String str_cstr_capped(const void *str_cstr, const void *cap) {
  u8* ptr = (u8*)str_cstr;
  u8* opl = (u8*)cap;
  for (;ptr < opl && *ptr != 0; ptr += 1);
  u64 size = (u64)(ptr - (u8*)str_cstr);
  String result = str((u8*)str_cstr, size);
  return result;
}

////////////////////////////////
// String Stylization

String upper_from_str(Arena *arena, String str) {
  str = push_str_copy(arena, str);
  Loop (i, str.size) {
    str.str[i] = char_to_upper(str.str[i]);
  }
  return str;
}

String lower_from_str(Arena *arena, String str) {
  str = push_str_copy(arena, str);
  Loop (i, str.size) {
    str.str[i] = char_to_lower(str.str[i]);
  }
  return str;
}

////////////////////////////////
// String Matching

b32 str_match(String str0, String str1) {
  b32 result = false;
  if (str0.size == str1.size) {
    result = true;
    Loop (i, str0.size) {
      if (str0.str[i] != str1.str[i]) {
        result = false;
        break;
      }
    }
  }
  return result;
}

b32 str_matchi(String str0, String str1) {
  b32 result = false;
  if (str0.size == str1.size) {
    result = true;
    Loop (i, str0.size) {
      u8 s0 = str0.str[i];
      u8 s1 = str1.str[i];;
      s0 = char_to_lower(s0);
      s1 = char_to_lower(s1);
      if (s0 != s1) {
        result = false;
        break;
      }
    }
  }
  return result;
}

b32 str_ends_with(String string, String end) {
  String postfix = str_postfix(string, end.size);
  b32 is_match = str_match(end, postfix);
  return is_match;
}

////////////////////////////////
// String Formatting & Copying

String push_str_cat(Arena* arena, String s1, String s2) {
  String str;
  str.size = s1.size + s2.size;
  str.str = push_array(arena, u8, str.size + 1);
  MemCopy(str.str, s1.str, s1.size);
  MemCopy(str.str + s1.size, s2.str, s2.size);
  str.str[str.size] = 0;
  return str;
}

String push_str_copy(Arena* arena, String s) {
  String str;
  str.size = s.size;
  str.str = push_array(arena, u8, str.size + 1);
  MemCopy(str.str, s.str, s.size);
  str.str[str.size] = 0;
  return str;
}

String push_strfv(Arena* arena, const void* format, void* argc) {
  va_list argc_copy;
  argc_copy = (char*)argc;
  u64 need_bytes = vsnprintf(0, 0, (char*)format, argc_copy) + 1;
  va_end(argc_copy);
  
  u8* buffer = push_buffer(arena, u8, KB(1));
  u64 final_size = vsnprintf((char*)buffer, need_bytes, (char*)format, argc_copy);
  
  String result = {buffer, final_size};
  return result;
}

String push_strf(Arena* arena, const void* format, ...) {
  va_list argc;
  va_start(argc, format);
  String result = push_strfv(arena, format, argc);
  va_end(argc);
  return result;
}

////////////////////////////////
// String some random stuff
void str_copy(void* dest, String str) {
  MemCopy(dest, str.str, str.size);
  u8* c = (u8*)dest;
  c[str.size] = 0;
}

String str_read_line(StringCursor* cursor) {
  while (cursor->at < cursor->end) {
    u8* line_start = cursor->at;

    // Find end of line
    while (cursor->at < cursor->end && *cursor->at != '\n' && *cursor->at != '\r') {
      cursor->at++;
    }

    u64 len = cursor->at - line_start;

    // Handle \r\n or \n
    if (cursor->at < cursor->end && *cursor->at == '\r') cursor->at++;
    if (cursor->at < cursor->end && *cursor->at == '\n') cursor->at++;

    // If line is not empty, return it
    if (len > 0) {
      String result = {line_start, len};
      return result;
    }

    // If line was empty, loop to read the next one
  }

  // If nothing left
  String result = { 0, 0 };
  return result;
}

String str_trim(String string) {
  String result;
  u8* s = string.str;
  while (char_is_space(*s)) {
    ++s;
  }
  result.str = s;
  if (*s) {
    u8* e = string.str + string.size;
    while (char_is_space(*(e-1))) {
      --e;
    }
    result.size = e - result.str;
  }
  
  return result;
}

i32 str_index_of(String string, u8 c) {
  if (!string.str) {
    return -1;
  }
  if (string.size > 0) {
    for (u32 i = 0; i < string.size; ++i) {
      if (string.str[i] == c) {
        return i;
      }
    }
  }
  
  return -1;
}

////////////////////////////////
// String <=> Integer Conversions

// string -> integer
b32 str_to_v4(void* str, v4* out_vector) {
  char* s = (char*)str;
  MemZeroStruct(out_vector);
  i32 result = sscanf(s, "%f %f %f %f", &out_vector->x, &out_vector->y, &out_vector->z, &out_vector->w);
  return result != -1;
}

b32 str_to_v3(const char* str, v3* out_vector) {
  MemZeroStruct(out_vector);
  i32 result = sscanf(str, "%f %f %f", &out_vector->x, &out_vector->y, &out_vector->z);
  return result != -1;
}

b32 str_to_v2(const char* str, v2* out_vector) {
  MemZeroStruct(out_vector);
  i32 result = sscanf(str, "%f %f", &out_vector->x, &out_vector->y);
  return result != -1;
}

b32 str_to_f32(const char* str, f32* f) {
  *f = 0;
  i32 result = sscanf(str, "%f", f);
  return result != -1;
}

b32 str_to_f64(const char* str, f64* f) {
  *f = 0;
  i32 result = sscanf(str, "%lf", f);
  return result != -1;
}

b32 str_to_i8(const char* str, i8* i) {
  *i = 0;
  i32 result = sscanf(str, "%hhi", i);
  return result != -1;
}

b32 str_to_i16(const char* str, i16* i) {
  *i = 0;
  i32 result = sscanf(str, "%hi", i);
  return result != -1;
}

b32 str_to_i32(const char* str, i32* i) {
  *i = 0;
  i32 result = sscanf(str, "%i", i);
  return result != -1;
}

b32 str_to_i64(const char* str, i64* i) {
  *i = 0;
  i32 result = sscanf(str, "%lli", i);
  return result != -1;
}

b32 str_to_u8(const char* str, u8* i) {
  *i = 0;
  i32 result = sscanf(str, "%hhu", i);
  return result != -1;
}

b32 str_to_u16(const char* str, u16* i) {
  *i = 0;
  i32 result = sscanf(str, "%hu", i);
  return result != -1;
}

b32 str_to_u32(const char* str, u32* i) {
  *i = 0;
  i32 result = sscanf(str, "%u", i);
  return result != -1;
}

b32 str_to_u64(const char* str, u64* i) {
  *i = 0;
  i32 result = sscanf(str, "%llu", i);
  return result != -1;
}

b32 str_to_bool(const char* str, b8* b) {
  return cstr_match(str, "1") || cstr_matchi(str, "true");
}

// integer -> string
// ...

////////////////////////////////
// String Path Helpers

// one/two/three -> one/two/
String str_chop_after_last_slash(String string){
  if (string.size > 0) {
    u8* ptr = string.str + string.size - 1;
    for (; ptr >= string.str; --ptr) {
      if (*ptr == '/' || *ptr == '\\') {
        break;
      }
    }
    if (ptr >= string.str) {
      string.size = (u64)(ptr - string.str) + 1;
    } else {
      string.size = 0;
    }
  }
  return string;
}

// one/two/three -> one/two
String str_chop_last_slash(String string) {
  if (string.size > 0) {
    u8* ptr = string.str + string.size - 1;
    for (; ptr >= string.str; --ptr) {
      if (*ptr == '/' || *ptr == '\\') {
        break;
      }
    }
    if (ptr >= string.str) {
      string.size = (u64)(ptr - string.str);
    } else {
      string.size = 0;
    }
  }
  return string;
}

// one/two/three -> three
String str_skip_last_slash(String string) {
  if (string.size > 0) {
    u8* ptr = string.str + string.size - 1;
    for (; ptr >= string.str; --ptr) {
      if (*ptr == '/' || *ptr == '\\') {
        break;
      }
    }
    if (ptr >= string.str) {
      ptr += 1;
      string.size = (u64)(string.str + string.size - ptr);
      string.str = ptr;
    }
  }
  return string;
}

String str_chop_last_dot(String string) {
  String result = string;
  u64 p = string.size;
  for (; p > 0;) {
    p -= 1;
    if (string.str[p] == '.') {
      result = str_prefix(string, p);
      break;
    }
  }
  return result;
}

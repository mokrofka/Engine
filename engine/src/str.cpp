#include "str.h"

#include "memory.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

u32 range_size(Range r) {
  u32 c = ((r.max > r.min) ? (r.max - r.min) : 0);
  return c;
}

b8 char_is_space(u8 c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == '\f' || c == '\v';
}

b8 char_is_upper(u8 c) {
  return 'A' <= c && c <= 'Z';
}

b8 char_is_lower(u8 c) {
  return 'a' <= c && c <= 'z';
}

b8 char_is_alpha(u8 c) {
  return char_is_upper(c) || char_is_lower(c);
}

b8 char_is_slash(u8 c) {
  return c == '/' || c == '\\';
}

u8 char_to_lower(u8 c) {
  if (char_is_upper(c)) {
    c += ('a' - 'A');
  }
  return c;
}

u8 char_to_upper(u8 c) {
  if (char_is_lower(c)) {
    c += ('A' - 'a');
  }
  return c;
}

u8 char_to_correct_slash(u8 c) {
  if (char_is_slash(c)) {
    c = '/';
  }
  return c;
}

u64 cstr_length(u8* c) {
  u8* p = c;
  for (; *p != 0; p += 1);
  return p - c;
}

String str(u8* str, u64 size) {
  String result = {str, size};
  return result;
}

String str_range(u8* first, u8* one_past_last) {
  String result = {first, (u64)(one_past_last - first)};
  return result;
}

String str_zero() {
  String result = {};
  return result;
}

u8* str_format_v(void* buffer, const void* format, void* va_listp) {
  if (!format) {
    return 0;
  }

  // Create a copy of the va_listp since vsnprintf can invalidate the elements of the list
  // while finding the required buffer length.
  va_list list_copy;
#ifdef _MSC_VER
  list_copy = (char*)va_listp;
#elif defined(KPLATFORM_APPLE)
  list_copy = va_listp;
#else
  va_copy(list_copy, va_listp);
#endif
  i32 length = vsnprintf(0, 0, (char*)format, list_copy);
  va_end(list_copy);
  vsnprintf((char*)buffer, length + 1, (char*)format, (char*)va_listp);
  ((u8*)(buffer))[length] = 0;
  return (u8*)buffer;
}

u8* str_format(void* buffer, const void* format, ...) {
  if (!format) {
    return 0;
  }

  __builtin_va_list arg_ptr;
  va_start(arg_ptr, format);
  u8* result = str_format_v(buffer, format, arg_ptr);
  va_end(arg_ptr);
  return result;
}

String cstr(const void* c) {
  String result = {(u8*)c, cstr_length((u8*)c)};
  return result;
}

String cstr_capped(void *cstr, void *cap) {
  char *ptr = (char *)cstr;
  char *opl = (char *)cap;
  for (;ptr < opl && *ptr != 0; ptr += 1);
  u64 size = (u64)(ptr - (char *)cstr);
  String result = str((u8*)cstr, size);
  return result;
}

b8 cstr_equal(const void* str0, const void* str1) {
  return strcmp((char*)str0, (char*)str1) == 0;
}

b8 cstr_equali(const void* str0, const void* str1) {
  return _strcmpi((char*)str0, (char*)str1) == 0;
}

String str8_substr(String str, Range range) {
  range.min = ClampTop(range.min, str.size);
  range.max = ClampTop(range.max, str.size);
  str.str += range.min;
  str.size = range_size(range);
  return str;
}

String str8_prefix(String str, u64 size) {
  str.size = ClampTop(size, str.size);
  return str;
}

String str8_skip(String str, u64 amt) {
  amt = ClampTop(amt, str.size);
  str.str += amt;
  str.size -= amt;
  return str;
}

String str8_postfix(String str, u64 size) {
  size = ClampTop(size, str.size);
  str.str = (str.str + str.size) - size;
  str.size = size;
  return str;
}

String str8_chop(String str, u64 amt) {
  amt = ClampTop(amt, str.size);
  str.size -= amt;
  return str;
}

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

String str_chop_last_segment(String string){
  if (string.size > 0){
    u8 *ptr = string.str + string.size - 1;
    for (;ptr >= string.str; ptr -= 1){
      if (*ptr == '/' || *ptr == '\\') {
        break;
      }
    }
    if (ptr >= string.str){
      string.size = (u64)(ptr - string.str) + 1;
    }
    else{
      string.size = 0;
    }
  }
  return string;
}

String str_chop_last_slash(String string){
  if (string.size > 0){
    u8 *ptr = string.str + string.size - 1;
    for (;ptr >= string.str; ptr -= 1){
      if (*ptr == '/' || *ptr == '\\'){
        break;
      }
    }
    if (ptr >= string.str){
      string.size = (u64)(ptr - string.str);
    }
    else{
      string.size = 0;
    }
  }
  return string;
}

String str_skip_last_slash(String string) {
  if (string.size > 0) {
    u8* ptr = string.str + string.size - 1;
    for (; ptr >= string.str; ptr -= 1) {
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
      result = str8_prefix(string, p);
      break;
    }
  }
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
    u8* e = string.str + string.size-1;
    while (char_is_space(*(--e)));
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

b8 str_to_v4(const char* str, v4* out_vector) {
  if (!str) {
    return false;
  }
  
  MemZeroStruct(out_vector);
  i32 result = sscanf(str, "%f %f %f %f", &out_vector->x, &out_vector->y, &out_vector->z, &out_vector->w);
  return result != -1;
}

b8 str_to_v3(const char* str, v3* out_vector) {
  if (!str) {
    return false;
  }
  
  MemZeroStruct(out_vector);
  i32 result = sscanf(str, "%f %f %f", &out_vector->x, &out_vector->y, &out_vector->z);
  return result != -1;
  
}

b8 str_to_v2(const char* str, v2* out_vector) {
  if (!str) {
    return false;
  }
  
  MemZeroStruct(out_vector);
  i32 result = sscanf(str, "%f %f", &out_vector->x, &out_vector->y);
  return result != -1;

}

b8 str_to_f32(const char* str, f32* f) {
  if (!str) {
    return false;
  }
  
  *f = 0;
  i32 result = sscanf(str, "%f", f);
  return result != -1;

}

b8 str_to_f64(const char* str, f64* f) {
  if (!str) {
    return false;
  }
  
  *f = 0;
  i32 result = sscanf(str, "%lf", f);
  return result != -1;
}

b8 str_to_i8(const char* str, i8* i) {
  if (!str) {
    return false;
  }
  
  *i = 0;
  i32 result = sscanf(str, "%hhi", i);
  return result != -1;
}

b8 str_to_i16(const char* str, i16* i) {
  if (!str) {
    return false;
  }
  
  *i = 0;
  i32 result = sscanf(str, "%hi", i);
  return result != -1;
}

b8 str_to_i32(const char* str, i32* i) {
  if (!str) {
    return false;
  }
  
  *i = 0;
  i32 result = sscanf(str, "%i", i);
  return result != -1;
}

b8 str_to_i64(const char* str, i64* i) {
  if (!str) {
    return false;
  }
  
  *i = 0;
  i32 result = sscanf(str, "%lli", i);
  return result != -1;
}

b8 str_to_u8(const char* str, u8* i) {
  if (!str) {
    return false;
  }
  
  *i = 0;
  i32 result = sscanf(str, "%hhu", i);
  return result != -1;
}

b8 str_to_u16(const char* str, u16* i) {
  if (!str) {
    return false;
  }
  
  *i = 0;
  i32 result = sscanf(str, "%hu", i);
  return result != -1;
}

b8 str_to_u32(const char* str, u32* i) {
  if (!str) {
    return false;
  }
  
  *i = 0;
  i32 result = sscanf(str, "%u", i);
  return result != -1;
}

b8 str_to_u64(const char* str, u64* i) {
  if (!str) {
    return false;
  }
  
  *i = 0;
  i32 result = sscanf(str, "%llu", i);
  return result != -1;
}

b8 str_to_bool(const char* str, b8* b) {
  if (!str) {
    return false;
  }
  
  return cstr_equal(str, "1") || cstr_equali(str, "true");
}

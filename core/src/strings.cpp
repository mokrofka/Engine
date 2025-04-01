#include "strings.h"

#include "memory.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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

u8* str_format_v(u8* buffer, const char* format, char* va_listp) {
  if (!format) {
    return 0;
  }

  // Create a copy of the va_listp since vsnprintf can invalidate the elements of the list
  // while finding the required buffer length.
  va_list list_copy;
#ifdef _MSC_VER
  list_copy = va_listp;
#elif defined(KPLATFORM_APPLE)
  list_copy = va_listp;
#else
  va_copy(list_copy, va_listp);
#endif
  i32 length = vsnprintf(0, 0, format, list_copy);
  va_end(list_copy);
  vsnprintf((char*)buffer, length + 1, format, va_listp);
  buffer[length] = 0;
  return buffer;
}

char* str_format(u8* buffer, const char* format, ...) {
  if (!format) {
    return 0;
  }

  __builtin_va_list arg_ptr;
  va_start(arg_ptr, format);
  char* result = (char*)str_format_v(buffer, format, arg_ptr);
  va_end(arg_ptr);
  return result;
}

String cstr(const char* c) {
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

b8 cstr_equal(const char* str0, const char* str1) {
  return strcmp(str0, str1) == 0;
}

b8 cstr_equali(const char* str0, const char* str1) {
  return _strcmpi(str0, str1) == 0;
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
  return (string);
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
  return (result);
}

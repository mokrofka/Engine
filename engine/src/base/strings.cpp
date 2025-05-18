#include "lib.h"

#include <stdarg.h>
#include <stdio.h>

////////////////////////////////
// C-String Measurement, Functions

internal u32 write_uint(u8* dest, u32 value) {
  u8 temp[10];
  u32 count = 0;
  do {
    temp[count++] = '0' + (value % 10);
    value /= 10;
  } while (value);
  for (u32 i = 0; i < count; ++i) {
    dest[i] = temp[count - i - 1];
  }
  return count;
}

internal u32 write_int(u8* dest, i32 value) {
  u32 count = 0;
  if (value < 0) {
    dest[count++] = '-';
    value = -value;
  }
  return count + write_uint(dest + count, value);
}

internal u32 write_float(u8* dest, float value, u32 precision) {
  // Handle special cases
  if (value != value) { // NaN check
    const char* nan_str = "NaN";
    for (u32 i = 0; nan_str[i] != '\0'; i++) {
      dest[i] = nan_str[i];
    }
    return 3; // "NaN" is 3 characters long
  }
  if (value == 0.0f) { // Zero check
    dest[0] = '0';
    dest[1] = '\0';
    return 1;
  }

  // Handle negative values
  u32 len = 0;
  if (value < 0.0f) {
    dest[len++] = '-';
    value = -value;
  }

  // Process the integer part
  u32 integer_part = (u32)value;
  value -= integer_part;                      // Remove the integer part
  len += write_int(dest + len, integer_part); // Reuse write_int for integer part

  // Process the fractional part
  if (precision > 0) {
    dest[len++] = '.';
    // value *= (float)(10 ^ precision); // Shift decimal point
    value *= (float)Pow(10, precision); // Shift decimal point correctly
    u32 frac_part = (u32)value;
    len += write_int(dest + len, frac_part); // Reuse write_int for fractional part
  }

  return len;
}

internal u32 write_string(u8* dest, String val) {
  u32 len = 0;
  Loop (i, val.size) {
    dest[len++] = val.str[i]; // Copy each character from the String to the buffer
  }
  return len;  // Return the number of characters written
}

internal u32 uint_length(u32 value) {
  u32 count = 0;
  do {
    ++count;
    value /= 10;
  } while (value);
  return count;
}

internal u32 int_length(i32 value) {
  u32 count = 0;
  if (value < 0) {
    ++count;
    value = -value;
  }
  return count + uint_length(value);
}

u32 float_length(float value, u32 precision) {
  u32 len = 0;
  if (value < 0.0f) {
    ++len;
    value = -value;
  }
  
  // Process the integer part
  u32 integer_part = (u32)value;
  value -= integer_part;                      // Remove the integer part
  len += int_length(integer_part); // Reuse write_int for integer part

  // Process the fractional part
  if (precision > 0) {
    ++len;
    value *= (f32)Pow(10, precision); // Shift decimal point correctly
    u32 frac_part = (u32)value;
    len += int_length(frac_part); // Reuse write_int for fractional part
  }

  return len;
}

u32 my_vsnprintf(void* buffer, u32 buffer_size, const void* format, void* argc_) {
  va_list argc = (char*)argc_;
  
  u8* buff = (u8*)buffer;
  u8* fmt = (u8*)format;
  u32 written = 0;
  if (buffer_size == 0) {
    u32 length = {};
    u8* p = (u8*)fmt;
    for (; *p != 0; p += 1) {
      if (*p == '%') {
        ++p;
        switch (*p) {
          case 'i': {
            i32 val = va_arg(argc, i32);
            length += int_length(val);
          } break;
          case 'u': {
            u32 val = va_arg(argc, u32);
            length += int_length(val);
          } break;
          case 'f': {
            f32 val = va_arg(argc, f64);
            length += float_length(val, 6);
          } break;
          case 's': {
            String val = va_arg(argc, String);
            length += val.size;
          } break;
          case 'c': {
            length += 1;
          } break;
          case '.': {
            ++p; // skip .
            u32 precision = *fmt - '0';
            
            ++p; // skip number
            f32 val = va_arg(argc, f64);
            length += float_length(val, precision);
            ++length; // plus .
            // ++p; // don't skip because of for
          } break;
        }
      } else {
        ++length;
      }
    }
    return length;
  }
  else {
    while (written < buffer_size) {
      if (*fmt == '%') {
        ++fmt;
        switch (*fmt) {
          case 'i': {
            i32 val = va_arg(argc, i32);
            u32 len = write_int(buff + written, val);
            ++fmt;
            written += len;
          } break;
          case 'u': {
            u32 val = va_arg(argc, u32);
            u32 len = write_uint(buff + written, val);
            ++fmt;
            written += len;
          } break;
          case 'f': {
            f32 val = va_arg(argc, f64);
            u32 len = write_float(buff + written, val, 6);
            ++fmt;
            written += len;
          } break;
          case 's': {
            String val = va_arg(argc, String);
            u32 len = write_string(buff + written, val);
            ++fmt;
            written += len;
          } break;
          case 'c': {
            char val = va_arg(argc, i32);
            buff[written] = val;
            ++fmt;
            written += 1;
          } break;
          case '.': {
            ++fmt; // skip .
            u32 precision = *fmt - '0';
            
            f32 val = va_arg(argc, f64);
            u32 len = write_float(buff + written, val, precision);
            ++fmt; // skip number
            ++fmt; // skip f
            written += len;
          } break;
        }
      } else {
        buff[written++] = *fmt++;
      };
    }
  }
  return written;
}

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
  u32 size = (u32)(ptr - (u8*)str_cstr);
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
  if (str0.size != str1.size) return false;
  u8* a = str0.str;
  u8* b = str1.str;
  Loop (i, str0.size) {
    if (*a != *b) return false;
    ++a;
    ++b;
  }

  return true;
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
  va_list argc_copy = (va_list)argc;
  u32 need_bytes = my_vsnprintf(0, 0, format, argc) + 1;
  va_end(argc_copy);
  
  u8* buffer = push_buffer(arena, u8, need_bytes);
  u32 final_size = my_vsnprintf(buffer, need_bytes, format, argc_copy);
  
  String result = {buffer, final_size};
  result.str[result.size] = 0;
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

// String str_read_line(StringCursor* cursor) {
//   while (cursor->at < cursor->end) {
//     u8* line_start = cursor->at;

//     // Find end of line
//     while (cursor->at < cursor->end && *cursor->at != '\n' && *cursor->at != '\r') {
//       cursor->at++;
//     }

//     u64 len = cursor->at - line_start;

//     // Handle \r\n or \n
//     if (cursor->at < cursor->end && *cursor->at == '\r') cursor->at++;
//     if (cursor->at < cursor->end && *cursor->at == '\n') cursor->at++;

//     // If line is not empty, return it
//     if (len > 0) {
//       String result = {line_start, len};
//       return result;
//     }

//     // If line was empty, loop to read the next one
//   }

//   // If nothing left
//   String result = { 0, 0 };
//   return result;
// }

// should be little different on linux
String str_read_line(StringCursor* cursor) {
  while (cursor->at < cursor->end) {
    u8* line_start = cursor->at;

    // Find end of line
    while (cursor->at < cursor->end && *cursor->at != '\r') {
      cursor->at++;
    }

    u32 len = cursor->at - line_start;

    // Handle \r\n or \n
    cursor->at += 2;

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
  if (string) {
    Loop (i, string.size) {
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
      string.size = (u32)(ptr - string.str) + 1;
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
      string.size = (u32)(ptr - string.str);
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
      string.size = (u32)(string.str + string.size - ptr);
      string.str = ptr;
    }
  }
  return string;
}

String str_chop_last_dot(String string) {
  String result = string;
  u32 p = string.size;
  for (; p > 0;) {
    p -= 1;
    if (string.str[p] == '.') {
      result = str_prefix(string, p);
      break;
    }
  }
  return result;
}


void str_copy(String64& dest, String str) {
  MemCopy(dest.str, str.str, str.size);
  dest.size = str.size;
}

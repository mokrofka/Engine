#include "base_inc.h"

////////////////////////////////////////////////////////////////////////

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

internal u32 write_float(u8* dest, f32 value, u32 precision) {
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
    // value *= (f32)(10 ^ precision); // Shift decimal point
    value *= (f32)Pow(10, precision); // Shift decimal point correctly
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

u32 float_length(f32 value, u32 precision) {
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

////////////////////////////////////////////////////////////////////////
// Great sprintf
u32 my_sprintf(u8* buff, String fmt, va_list argc) {

  // Calculate length
  if (buff == null) {
    u32 length = 0;
    for (u8* p = fmt.str; p < fmt.str+fmt.size; ++p) {
      if (*p == '%') {
        ++p; // skip '%'
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
            f32 val = va_arg(argc, f64); // f64 - because of compiler
            #define DefaultFloatAccuracy 6
            length += float_length(val, DefaultFloatAccuracy);
          } break;
          case 's': {
            String val = va_arg(argc, String);
            length += val.size;
          } break;
          case 'c': {
            i32 val = va_arg(argc, i32);
            length += 1;
          } break;
          case '.': {
            ++p;                         // skip '.'
            u32 precision = *p - '0';    // 'num' - '0'
            ++p;                         // skip number
            f32 val = va_arg(argc, f64); // f64 - because of compiler

            length += float_length(val, precision);
          } break;
        }
      } else {
        ++length;
      }
    }
    return length;
  }

  // Write into buffer
  u32 written = 0;
  for (u8* p = fmt.str; p < fmt.str+fmt.size; ++p) {
    if (*p == '%') {
      ++p; // skip '%'
      switch (*p) {
        case 'i': {
          i32 val = va_arg(argc, i32);
          u32 len = write_int(buff + written, val);
          written += len;
        } break;
        case 'u': {
          u32 val = va_arg(argc, u32);
          u32 len = write_uint(buff + written, val);
          written += len;
        } break;
        case 'f': {
          f32 val = va_arg(argc, f64); // f64 - because of compiler
          u32 len = write_float(buff + written, val, DefaultFloatAccuracy);
          written += len;
        } break;
        case 's': {
          String val = va_arg(argc, String);
          u32 len = write_string(buff + written, val);
          written += len;
        } break;
        case 'c': {
          char val = va_arg(argc, i32); // i32 - because of compiler
          buff[written] = val;
          written += 1;
        } break;
        case '.': {
          ++p;                      // skip '.'
          u32 precision = *p - '0'; // 'num' - '0'
          ++p;                      // skip number

          f32 val = va_arg(argc, f64);
          u32 len = write_float(buff + written, val, precision);
          written += len;
        } break;
      }
    } else {
      buff[written++] = *p;
    };
  }
  return written;
}

////////////////////////////////////////////////////////////////////////
// String Constructors

String str_range(u8* first, u8* one_past_last) {
  String result = {first, (u32)(one_past_last - first)};
  return result;
}

String str_cstr_capped(const void *cstr, const void *cap) {
  u8* ptr = (u8*)cstr;
  u8* opl = (u8*)cap;
  for (;ptr < opl && *ptr != 0; ptr += 1);
  u32 size = (u32)(ptr - (u8*)cstr);
  String result = String((u8*)cstr, size);
  return result;
}

////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
// String Matching

b32 str_match(String str0, String str1) {
  if (str0.size != str1.size) return false;
  Loop (i, str0.size) {
    if (str0.str[i] != str1.str[i]) {
      return false;
    }
  }
  return true;
}

b32 str_matchi(String str0, String str1) {
  if (str0.size != str1.size) return false;
  Loop (i, str0.size) {
    if (char_to_lower(str0.str[i]) != char_to_lower(str1.str[i])) {
      return false;
    }
  }
  return true;
}

b32 str_ends_with(String string, String end) {
  String postfix = str_postfix(string, end.size);
  b32 is_match = str_match(end, postfix);
  return is_match;
}

////////////////////////////////////////////////////////////////////////
// String Slicing

String str_substr(String str, Range range) {
  range.offset = ClampTop(range.offset, str.size);
  range.size = ClampTop(range.size, str.size);
  str.str += range.offset;
  str.size = range_size(range);
  return str;
}

String str_prefix(String str, u64 size) {
  str.size = ClampTop(size, str.size);
  return str;
}

String str_skip(String str, u64 amt) {
  amt = ClampTop(amt, str.size);
  str.str += amt;
  str.size -= amt;
  return str;
}

String str_postfix(String str, u64 size) {
  size = ClampTop(size, str.size);
  str.str = (str.str + str.size) - size;
  str.size = size;
  return str;
}

String str_chop(String str, u64 amt) {
  amt = ClampTop(amt, str.size);
  str.size -= amt;
  return str;
}

////////////////////////////////////////////////////////////////////////
// String Formatting & Copying

String push_str_cat(Arena* arena, String s1, String s2) {
  String str;
  str.size = s1.size + s2.size;
  str.str = push_array(arena, u8, str.size + 1); // for C-str
  MemCopy(str.str, s1.str, s1.size);
  MemCopy(str.str + s1.size, s2.str, s2.size);
  str.str[str.size] = 0;
  return str;
}

String push_str_copy(Arena* arena, String s) {
  String str;
  str.size = s.size;
  str.str = push_array(arena, u8, str.size + 1); // for C-str
  MemCopy(str.str, s.str, s.size);
  str.str[str.size] = 0;
  return str;
}

String push_strfv(Arena* arena, String fmt, va_list argc) {
  va_list va_list_argc;
  va_copy(va_list_argc, argc); 
  u32 need_bytes = my_sprintf(0, fmt, va_list_argc);
  va_end(va_list_argc);

  u8* buff = push_buffer(arena, need_bytes + 1); // for C-str

  va_copy(va_list_argc, argc); 
  u32 final_size = my_sprintf(buff, fmt, va_list_argc);
  va_end(va_list_argc);
  
  String result = {buff, final_size};
  result.str[result.size] = 0;
  return result;
}

String push_strf(Arena* arena, String fmt, ...) {
  va_list argc;
  va_start(argc, fmt);
  String result = push_strfv(arena, fmt, argc);
  va_end(argc);
  return result;
}

////////////////////////////////////////////////////////////////////////
// String utils

void str_copy(String64& dest, String str) {
  MemCopy(dest.str, str.str, str.size);
  dest.size = str.size;
}

String str_next_word(String line, u32& start) {
  // skip spaces
  while (start < line.size && line.str[start] == ' ')
    start++;

  u32 token_start = start;
  while (start < line.size && line.str[start] != ' ')
    start++;

  return {line.str + token_start, start - token_start};
}

// the end of line win: \r\n, linux: \n
String str_read_line(Range* range) {
  while (range->offset < range->size) {
    u8* line_start = (u8*)range->offset;
    u8* start = (u8*)range->offset;
    u8* end = (u8*)range->size;

#if OS_WINDOWS
    while (start < end && *start != '\r') {
      ++start;
    }
    u32 len = start - line_start;
    // move to next line and handle \r\n
    range->offset += len + 2;
#else
    while (start < end && *start != '\n') {
      ++start;
    }
    u32 len = start - line_start;
    // move to next line and handle \n
    range->offset += len + 1;

#endif

    // If line is not empty, return it
    if (len > 0) {
      String result = {line_start, len};
      return result;
    }

    // If line was empty, loop to read the next one
  }

  // If nothing left
  String result = {0, 0};
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
  Loop (i, string.size) {
    if (string.str[i] == c) {
      return i;
    }
  }
  
  return -1;
}

////////////////////////////////////////////////////////////////////////
// String <=> Integer Conversions

u64 u64_from_str(String str) {
  u64 x = 0;
  Loop (i, str.size) {
    x *= 10;
    x += str.str[i]-'0';
  }
  return x;
}

i64 i64_from_str(String str) {
  i64 x = 0;
  u32 i = 0;
  b32 negative = false;

  if (str.str[i] == '-') {
    negative = true;
    i++;
  }

  Loop(j, str.size - i) {
    x *= 10;
    x += str.str[i + j] - '0';
  }

  if (negative)
    x = -x;
  return x;
}

f64 f64_from_str(String str) {
  f64 x = 0.0;
  f64 frac = 0.0;
  f64 factor = 0.1;
  b32 is_fraction = false;
  b32 negative = false;
  i32 j = 0;

  // handle optional sign
  if (str.size > 0) {
    if (str.str[j] == '-') {
      negative = true;
      j++;
    }
  }

  for (; j < str.size; ++j) {
    u8 ch = str.str[j];
    if (ch == '.') {
      is_fraction = true;
      ++j;
      break;
    }

    x = x * 10 + (ch - '0'); // integer part
  }
  for (; j < str.size; ++j) {
    u8 ch = str.str[j];
    frac += (ch - '0') * factor; // fractional part
    factor *= 0.1;
  }

  x += frac;
  if (negative)
    x = -x;
  return x;
}

////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////
// Wchar stuff

u64 wchar_to_char(char* out, const wchar_t* in, u64 out_size) {
  u64 out_len = 0;

  while (*in && out_len < out_size) {
    u16 wc = *in++;
    out[out_len++] = (char)wc;
  }

  if (out_len < out_size)
    out[out_len] = '\0'; // null-terminate

  return out_len;
}

String push_str_wchar(Arena* arena, const wchar_t* in, u32 wchar_length) {
  u8* buff = push_buffer(arena, wchar_length + 1);
  Loop (i , wchar_length) {
    buff[i] = in[i];
  }
  buff[wchar_length] = 0;
  return {buff, wchar_length};
}


#include "str.h"
#include "logger.h"

intern u32 u32_length(u32 v) {
  if (v < 10) return 1;
  if (v < 100) return 2;
  if (v < 1000) return 3;
  if (v < 10000) return 4;
  if (v < 100000) return 5;
  if (v < 1000000) return 6;
  if (v < 10000000) return 7;
  if (v < 100000000) return 8;
  if (v < 1000000000) return 9;
  return 10;
}

intern u32 u64_length(u64 v) {
  if (v < 10) return 1;
  if (v < 100) return 2;
  if (v < 1000) return 3;
  if (v < 10000) return 4;
  if (v < 100000) return 5;
  if (v < 1000000) return 6;
  if (v < 10000000) return 7;
  if (v < 100000000) return 8;
  if (v < 1000000000) return 9;
  if (v < 10000000000) return 10;
  if (v < 100000000000) return 11;
  if (v < 1000000000000) return 12;
  if (v < 10000000000000) return 13;
  if (v < 100000000000000) return 14;
  if (v < 1000000000000000) return 15;
  if (v < 10000000000000000) return 16;
  if (v < 100000000000000000) return 17;
  if (v < 1000000000000000000) return 18;
  if (v < 10000000000000000000ull) return 19;
  return 20;
}

intern u32 u32_write(u8* dest, u32 value) {
  u32 length = u32_length(value);
  u32 i = length;
  do {
    dest[--i] = '0' + (value % 10);
    value /= 10;
  } while (value);
  return length;
}

intern u32 u64_write(u8* dest, u64 value) {
  u32 length = u64_length(value);
  u32 i = length;
  do {
    dest[--i] = '0' + (value % 10);
    value /= 10;
  } while (value);
  return length;
}

intern u32 i32_length(i32 v) {
  if (v < 0)
    return 1+u32_length(-v);
  else
    return u32_length(v);
}

intern u32 i64_length(i64 v) {
  if (v < 0)
    return 1+u64_length(-v);
  else
    return u64_length(v);
}

intern u32 i32_write(u8* dest, i32 value) {
  if (value < 0) {
    dest[0] = '-';
    return 1 + u32_write(dest+1, -value);
  }
  else {
    return u32_write(dest, value);
  }
}

intern u32 i64_write(u8* dest, i64 value) {
  if (value < 0) {
    dest[0] = '-';
    return 1 + u64_write(dest+1, -value);
  }
  else {
    return u64_write(dest, value);
  }
}

intern u32 f32_length(f32 value, u32 precision) {
  u32 len = 0;
  if (value < 0.0f) {
    ++len;
    value = -value;
  }
  
  // Process the integer part
  u32 integer_part = value;
  value -= integer_part; // Remove the integer part
  len += u32_length(integer_part);

  // Process the fractional part
  if (precision > 0) {
    ++len; // for '.'
    len += precision;
  }

  return len;
}

intern u32 f64_length(f64 value, u32 precision) {
  u32 len = 0;
  if (value < 0.0f) {
    ++len;
    value = -value;
  }
  
  // Process the integer part
  u64 integer_part = value;
  value -= integer_part; // Remove the integer part
  len += u64_length(integer_part);

  // Process the fractional part
  if (precision > 0) {
    ++len; // for '.'
    len += precision;
  }

  return len;
}

intern u32 f32_write(u8* dest, f32 value, u32 precision) {
  u32 len = 0;
  if (value < 0.0f) {
    dest[len++] = '-';
    value = -value;
  }

  // Process the integer part
  u32 integer_part = (u32)value;
  value -= integer_part; // Remove the integer part
  len += u32_write(dest + len, integer_part);

  // Process the fractional part
  if (precision > 0) {
    dest[len++] = '.';

    u32 multiplier = 1;
    for (u32 i = 0; i < precision; ++i) multiplier *= 10;
    u32 frac_part = (u32)(value * multiplier + 0.5); // rounding since possible 0.02 * multi = 1.999...
    u32 frac_len = u32_length(frac_part);
    for (u32 i = 0; i < precision - frac_len; ++i) {
      dest[len++] = '0';
    }
    len += u32_write(dest + len, frac_part);
  }

  return len;
}

intern u32 f64_write(u8* dest, f64 value, u32 precision) {
  u32 len = 0;
  if (value < 0.0f) {
    dest[len++] = '-';
    value = -value;
  }

  // Process the integer part
  u64 integer_part = value;
  value -= integer_part; // Remove the integer part
  len += u64_write(dest + len, integer_part);

  // Process the fractional part
  if (precision > 0) {
    dest[len++] = '.';

    // for (u32 i = 0; i < precision; ++i) {
    //   value *= 10;
    //   u32 integer = value;
    //   value -= integer;
    //   dest[len++] = '0' + integer;
    // }

    // NOTE: feels more accurate
    u64 multiplier = 1;
    for (u32 i = 0; i < precision; ++i) multiplier *= 10;
    u64 frac_part = (u64)(value * multiplier + 0.5); // rounding since possible 0.02 * multi = 1.999...
    u32 frac_len = u64_length(frac_part);
    for (u32 i = 0; i < precision - frac_len; ++i) {
      dest[len++] = '0';
    }
    len += u64_write(dest + len, frac_part);
  }

  return len;
}

#define HEX_LENGTH 16
global u8 HEX[] = "0123456789ABCDEF";

u32 hex_u64_write(u8* dest, u64 value) {
  u32 len = HEX_LENGTH;

  for (i32 i = len - 1; i >= 0; --i) {
    dest[i] = HEX[value & 0xF]; // last 4 bits
    value >>= 4;                // shift right 4 bits
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
            if (str_match(String(p+1, 2), "64")) {
              p += 2; // skip "64"
              i64 val = va_arg(argc, i64);
              length += i64_length(val);
              break;
            }
            i32 val = va_arg(argc, i32);
            length += i32_length(val);
          } break;
          case 'u': {
            if (str_match(String(p+1, 2), "64")) {
              p += 2; // skip "64"
              u64 val = va_arg(argc, u64);
              length += u64_length(val);
              break;
            }
            u32 val = va_arg(argc, u32);
            length += u32_length(val);
          } break;
          case 'f': {
            f64 val = va_arg(argc, f64); // f64 - because of compiler
            #define DefaultFloatPrecision 6
            length += f64_length(val, DefaultFloatPrecision);
          } break;
          case 's': {
            String val = va_arg(argc, String);
            length += val.size;
          } break;
          case 'c': {
            i32 val = va_arg(argc, i32); // i32 - because of compiler
            length += 1;
          } break;
          case '.': {
            ++p;                         // skip '.'
            u32 precision = *p - '0';    // 'num' - '0'
            ++p;                         // skip number
            Assert(*p == 'f');
            f64 val = va_arg(argc, f64); // f64 - because of compiler
            length += f64_length(val, precision);
          } break;
          case 'p': {
            length += HEX_LENGTH;
          }; break;
          case '%': {
            length += 1;
          }; break;
        }
      }
      else {
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
          if (str_match(String(p+1, 2), "64")) {
            p += 2; // skip "64"
            i64 val = va_arg(argc, i64);
            u32 len = i64_write(buff + written, val);
            written += len;
            break;
          }
          i32 val = va_arg(argc, i32);
          u32 len = i32_write(buff + written, val);
          written += len;
        } break;
        case 'u': {
          if (str_match(String(p+1, 2), "64")) {
            p += 2; // skip "64"
            i64 val = va_arg(argc, i64);
            u32 len = i64_write(buff + written, val);
            written += len;
            break;
          }
          u32 val = va_arg(argc, u32);
          u32 len = u32_write(buff + written, val);
          written += len;
        } break;
        case 'f': {
          f64 val = va_arg(argc, f64); // f64 - because of compiler
          u32 len = f64_write(buff + written, val, DefaultFloatPrecision);
          written += len;
        } break;
        case 's': {
          String val = va_arg(argc, String);
          MemCopy(buff+written, val.str, val.size);
          written += val.size;
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
          f64 val = va_arg(argc, f64);
          u32 len = f64_write(buff + written, val, precision);
          written += len;
        } break;
        case 'p': {
          u64 val = va_arg(argc, u64);
          u32 len = hex_u64_write(buff + written, val);
          written += len;
        }; break;
        case '%': {
          buff[written] = '%';
          written += 1;
        }; break;
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

String upper_from_str(Allocator arena, String str) {
  str = push_str_copy(arena, str);
  Loop (i, str.size) {
    str.str[i] = char_to_upper(str.str[i]);
  }
  return str;
}

String lower_from_str(Allocator arena, String str) {
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

String push_str_cat(Allocator arena, String s1, String s2) {
  String str;
  str.size = s1.size + s2.size;
  str.str = push_array(arena, u8, str.size + 1); // for C-str
  MemCopy(str.str, s1.str, s1.size);
  MemCopy(str.str + s1.size, s2.str, s2.size);
  str.str[str.size] = 0;
  return str;
}

String push_str_copy(Allocator arena, String s) {
  String str;
  str.size = s.size;
  str.str = push_array(arena, u8, str.size + 1); // for C-str
  MemCopy(str.str, s.str, s.size);
  str.str[str.size] = 0;
  return str;
}

String push_strfv(Allocator arena, String fmt, va_list argc) {
  va_list va_list_argc;
  va_copy(va_list_argc, argc); 
  u32 need_bytes = my_sprintf(null, fmt, va_list_argc);
  va_end(va_list_argc);

  u8* buff = push_buffer(arena, need_bytes + 1); // for C-str

  va_copy(va_list_argc, argc); 
  u32 final_size = my_sprintf(buff, fmt, va_list_argc);
  va_end(va_list_argc);
  
  String result = {buff, final_size};
  result.str[result.size] = 0;
  return result;
}

String push_strf(Allocator arena, String fmt, ...) {
  va_list argc;
  va_start(argc, fmt);
  String result = push_strfv(arena, fmt, argc);
  va_end(argc);
  return result;
}

////////////////////////////////////////////////////////////////////////
// String List Construction Functions
StringNode* str_list_push_node(StringList* list, StringNode* node) {
  SLLQueuePush(list->first, list->last, node);
  list->node_count += 1;
  list->total_size += node->string.size;
  return node;
}

StringNode* str_list_push_node_set_string(StringList* list, StringNode* node, String string) {
  SLLQueuePush(list->first, list->last, node);
  list->node_count += 1;
  list->total_size += string.size;
  node->string = string;
  return node;
}

StringNode* str_list_push(Allocator arena, StringList* list, String string) {
  StringNode* node = push_struct(arena, StringNode);
  str_list_push_node_set_string(list, node, string);
  return node;
}

////////////////////////////////////////////////////////////////////////
// String utils

void str_copy(String64& dest, String str) {
  MemCopy(dest.str, str.str, str.size);
  dest.size = str.size;
}

String str_next_word(String line, u32& start) {
  // skip spaces
  while (start < line.size && char_is_space(line.str[start]))
    start++;

  u32 token_start = start;
  while (start < line.size && !char_is_space(line.str[start]))
    start++;

  return {line.str + token_start, start - token_start};
}

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

// one/two/three -> two/three
String str_skip_slash(String string) {
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
  return string;
}

// one/two/three -> one/two
String str_chop_last_slash(String string) {
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
  return string;
}

// one/two/three -> three
String str_skip_last_slash(String string) {
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

String push_str_wchar(Allocator arena, const wchar_t* in, u32 wchar_length) {
  u8* buff = push_buffer(arena, wchar_length + 1);
  Loop (i , wchar_length) {
    buff[i] = in[i];
  }
  buff[wchar_length] = 0;
  return {buff, wchar_length};
}


#include "str.h"
#include "logger.h"

String str_make(u8* str, u64 size) {
  String res = {};
  res.str = str;
  res.size = size;
  return res;
}
String str_make(Slice<u8> str) {
  String res = str_make(str.data, str.size);
  return res;
}
u64 cstr_length(const void* c) {
  u8* p = (u8*)c;
  for (; *p != 0; ++p);
  return (u64)(p - (u8*)c);
}

String::String(const char* str_) { 
  str = (u8*)str_;
  size = cstr_length(str);
}

Slice<u8> dstr_slice(Dstring dstr) {
  return Slice(dstr.str, dstr.size);
}
Dstring dstr_make(Allocator alloc) {
  Dstring res = {
    .alloc = alloc,
  };
  return res;
}
void dstr_push(Dstring& arr, String str) {
  if (str.size + arr.size > arr.cap) {
    if (arr.str) {
      u32 modifier = CeilIntDiv(str.size+arr.size, arr.cap);
      u32 old_cap = arr.cap;
      arr.cap *= modifier;
      arr.str = mem_realloc_array(arr.alloc, arr.str, old_cap, arr.cap);
    } else {
      arr.cap = Max(str.size, (u64)DEFAULT_CAPACITY);
      arr.str = mem_alloc(arr.alloc, arr.cap);
    }
  }
  MemCopy(arr.str+arr.size, str.str, str.size);
  arr.size += str.size;
}
void dstr_clear(Dstring&arr) {
  arr.size = 0;
}

Dstring::operator String() { return str_make(str, size); }
String64::operator String() { return str_make(str, size); }

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
    u32 pad = (frac_len < precision) ? (precision - frac_len) : 0;
    for (u32 i = 0; i < pad; ++i) {
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
    u64 multiplier = 1;
    for (u32 i = 0; i < precision; ++i) multiplier *= 10;
    u64 frac_part = (u64)(value * multiplier + 0.5); // rounding since possible 0.02 * multi = 1.999...
    u32 frac_len = u64_length(frac_part);
    u32 pad = (frac_len < precision) ? (precision - frac_len) : 0;
    for (u32 i = 0; i < pad; ++i) {
      dest[len++] = '0';
    }
    len += u64_write(dest + len, frac_part);
  }
  return len;
}

#define HEX_LENGTH 16
global u8 HEX[] = "0123456789ABCDEF";

intern u32 hex_u64_write(u8* dest, u64 value) {
  u32 len = HEX_LENGTH;
  for (i32 i = len - 1; i >= 0; --i) {
    dest[i] = HEX[value & 0xF];
    value >>= 4;
  }
  return len;
}

////////////////////////////////////////////////////////////////////////
// Great sprintf

intern u32 my_sprintf(u8* buf, String fmt, VaList argc) {
  // Calculate length
  if (buf == null) {
    u32 length = 0;
    for (u8* p = fmt.str; p < fmt.str+fmt.size; ++p) {
      if (*p == '%') {
        ++p; // skip '%'
        switch (*p) {
          case 'i': {
            if (str_match(str_make(p+1, 2), "64")) {
              p += 2; // skip "64"
              i64 val = va_arg(argc, i64);
              length += i64_length(val);
            }
            else {
              i32 val = va_arg(argc, i32);
              length += i32_length(val);
            }
          } break;
          case 'u': {
            if (str_match(str_make(p+1, 2), "64")) {
              p += 2; // skip "64"
              u64 val = va_arg(argc, u64);
              length += u64_length(val);
            }
            else {
              u32 val = va_arg(argc, u32);
              length += u32_length(val);
            }
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
            UnusedVariable(val);
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
            va_arg(argc, void*);
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
          if (str_match(str_make(p+1, 2), "64")) {
            p += 2; // skip "64"
            i64 val = va_arg(argc, i64);
            u32 len = i64_write(buf + written, val);
            written += len;
          }
          else {
            i32 val = va_arg(argc, i32);
            u32 len = i32_write(buf + written, val);
            written += len;
          }
        } break;
        case 'u': {
          if (str_match(str_make(p+1, 2), "64")) {
            p += 2; // skip "64"
            u64 val = va_arg(argc, u64);
            u32 len = u64_write(buf + written, val);
            written += len;
            break;
          }
          else {
            u32 val = va_arg(argc, u32);
            u32 len = u32_write(buf + written, val);
            written += len;
          }
        } break;
        case 'f': {
          f64 val = va_arg(argc, f64); // f64 - because of compiler
          u32 len = f64_write(buf + written, val, DefaultFloatPrecision);
          written += len;
        } break;
        case 's': {
          String val = va_arg(argc, String);
          MemCopy(buf+written, val.str, val.size);
          written += val.size;
        } break;
        case 'c': {
          char val = va_arg(argc, i32); // i32 - because of compiler
          buf[written] = val;
          written += 1;
        } break;
        case '.': {
          ++p;                      // skip '.'
          u32 precision = *p - '0'; // 'num' - '0'
          ++p;                      // skip number
          f64 val = va_arg(argc, f64);
          u32 len = f64_write(buf + written, val, precision);
          written += len;
        } break;
        case 'p': {
          u64 val = va_arg(argc, u64);
          u32 len = hex_u64_write(buf + written, val);
          written += len;
        }; break;
        case '%': {
          buf[written] = '%';
          written += 1;
        }; break;
      }
    } else {
      buf[written++] = *p;
    };
  }
  return written;
}

////////////////////////////////////////////////////////////////////////
// Character Classification & Conversion Functions

b32 char_is_space(u8 c)        { return c == ' ' || c == '\t'; }
b32 char_is_newline(u8 c)      { return c == '\r' || c == '\n'; }
b32 char_is_ws(u8 c)           { return char_is_space(c) || char_is_newline(c); }
b32 char_is_upper(u8 c)        { return 'A' <= c && c <= 'Z'; }
b32 char_is_lower(u8 c)        { return 'a' <= c && c <= 'z'; }
b32 char_is_alpha(u8 c)        { return char_is_upper(c) || char_is_lower(c); }
b32 char_is_slash(u8 c)        { return c == '/' || c == '\\'; }
b32 char_is_digit(u8 c)        { return (c >= '0' && c <= '9'); }
u8 char_to_lower(u8 c)         { if (char_is_upper(c)) { c += ('a' - 'A'); } return c; }
u8 char_to_upper(u8 c)         { if (char_is_lower(c)) { c += ('A' - 'a'); } return c; }
u8 char_to_correct_slash(u8 c) { if (char_is_slash(c)) { c = '/'; } return c; }
b32 char_is_number_cont(u8 c)  { return (c >= '0' && c <= '9') || c == '.' || c == '-'; }

////////////////////////////////////////////////////////////////////////
// String Constructors

String str_range(u8* first, u8* one_past_last) {
  String result = str_make(first, (u32)(one_past_last - first));
  return result;
}

String str_cstr_capped(const void *cstr, const void *cap) {
  u8* ptr = (u8*)cstr;
  u8* opl = (u8*)cap;
  for (;ptr < opl && *ptr != 0; ptr += 1);
  u32 size = (u32)(ptr - (u8*)cstr);
  String result = str_make((u8*)cstr, size);
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

u64 str_find_needle(String string, u64 start_pos, String needle) {
  u8 first_char = needle.str[0];
  u8 last_char = needle.str[needle.size-1];
  u8* p = string.str + start_pos;
  u64 stop_offset = Max(string.size + 1, needle.size) - needle.size;
  u8* stop_p = string.str + stop_offset;
  if (needle.size > 0) {
    for (;p < stop_p; ++p) {
      if (*p == first_char) {
        if (p[needle.size-1] == last_char) {
          if (str_match(str_range(p, p+needle.size), needle)) {
            break;
          }
        }
      }
    }
  }
  u64 res = string.size;
  if (p < stop_p) {
    res = (u64)(p - string.str);
  }
  return res;
}

b32 str_ends_with(String string, String end) {
  String postfix = str_postfix(string, end.size);
  b32 is_match = str_match(end, postfix);
  return is_match;
}

b32 equal(String a, String b) { return str_match(a, b); }

////////////////////////////////////////////////////////////////////////
// String Slicing

String str_substr(String str, Rng1u range) {
  range.min = ClampTop(range.min, (u32)str.size);
  range.max = ClampTop(range.max, (u32)str.size);
  str.str += range.min;
  str.size = rng1u_dim(range);
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
  str.str = push_array(arena, u8, str.size + 1);
  MemCopy(str.str, s1.str, s1.size);
  MemCopy(str.str + s1.size, s2.str, s2.size);
  str.str[str.size] = 0;
  return str;
}

String push_str_copy(Allocator arena, String s) {
  String str;
  str.size = s.size;
  str.str = push_array(arena, u8, str.size + 1);
  MemCopy(str.str, s.str, s.size);
  str.str[str.size] = 0;
  return str;
}

String push_strfv(Allocator arena, String fmt, VaList args) {
  VaList va_list_argc;
  va_copy(va_list_argc, args); 
  u32 need_bytes = my_sprintf(null, fmt, va_list_argc);
  va_end(va_list_argc);
  u8* buf = push_buffer(arena, need_bytes + 1);
  va_copy(va_list_argc, args); 
  u32 final_size = my_sprintf(buf, fmt, va_list_argc);
  va_end(va_list_argc);
  String result = str_make(buf, final_size);
  result.str[result.size] = 0;
  return result;
}

String push_strf(Allocator arena, String fmt, ...) {
  VaList argc;
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

StringNode* str_list_pushf(Allocator arena, StringList* list, String fmt, ...) {
  VaList args;
  va_start(args, fmt);
  String string = push_strfv(arena, fmt, args);
  StringNode* result = str_list_push(arena, list, string);
  va_end(args);
  return result;
}

////////////////////////////////////////////////////////////////////////
// String utils

void str_copy(String64& dest, String str) {
  Assert(str.size <= 64);
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
  return str_make(line.str + token_start, start - token_start);
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
  b32 negative = false;
  i32 j = 0;

  // Sign
  if (j < str.size) {
    if (str.str[j] == '-') {
      negative = true;
      j++;
    } else if (str.str[j] == '+') {
      ++j;
    }
  }

  // Integer part
  for (; j < str.size; ++j) {
    u8 ch = str.str[j];
    if (ch == '.' || ch == 'e' || ch == 'E')
      break;
    x = x * 10 + (ch - '0');
  }

  // Fractional part
  if (j < str.size && str.str[j] == '.') {
    ++j;
    for (; j < str.size; ++j) {
      u8 ch = str.str[j];
      if (ch == 'e' || ch == 'E')
        break;
      frac += (ch - '0') * factor;
      factor *= 0.1;
    }
  }
  x += frac;

  // Exponent
  if (j < str.size && (str.str[j] == 'e' || str.str[j] == 'E')) {
    ++j;
    b32 exp_negative = false;
    if (j < str.size) {
      if (str.str[j] == '-') {
        exp_negative = true;
        ++j;
      } else if (str.str[j] == '+') {
        ++j;
      }
    }
    i32 exp = 0;
    for (; j < str.size; ++j) {
      exp = exp * 10 + (str.str[j] - '0');
    }
    if (exp_negative)
      exp = -exp;
    x *= Pow(10.0, exp);
  }

  if (negative)
    x = -x;
  return x;
}

u32 u32_from_str(String str) { return u64_from_str(str); };
i32 i32_from_str(String str) { return i64_from_str(str); };
f32 f32_from_str(String str) { return f64_from_str(str); };

////////////////////////////////////////////////////////////////////////
// String Path Helpers

String str_chop_past_last_slash(String string){
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

String str_skip_last_dot(String string) {
  u8* ptr = string.str + string.size - 1;
  for (; ptr >= string.str; --ptr) {
    if (*ptr == '.') {
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

////////////////////////////////////////////////////////////////////////
// Wchar stuff

u64 wchar_to_char(char* out, const wchar_t* in, u64 out_size) {
  u64 out_len = 0;
  while (*in && out_len < out_size) {
    u16 wc = *in++;
    out[out_len++] = (char)wc;
  }
  if (out_len < out_size)
    out[out_len] = '\0';
  return out_len;
}

String push_str_wchar(Allocator arena, const wchar_t* in, u32 wchar_length) {
  u8* buf = push_buffer(arena, wchar_length + 1);
  Loop (i , wchar_length) {
    buf[i] = in[i];
  }
  buf[wchar_length] = 0;
  return str_make(buf, wchar_length);
}

Lexer lexer_init(String buffer) {
  Lexer lexer = {
    .cur = buffer.str,
    .end = buffer.str + buffer.size,
  };
  return lexer;
}

String lexer_next_token(Lexer* l) {
  while (l->cur < l->end && char_is_space(*l->cur)) {
    l->cur++;
  }
  if (l->cur == l->end) return {};
  u8* current = l->cur;
  while (l->cur < l->end && !char_is_space(*l->cur)) {
    l->cur++;
  }
  String result = str_make(current, (u64)l->cur - (u64)current);
  return result;
}

String lexer_next_integer(Lexer* l) {
  while (l->cur < l->end && char_is_space(*l->cur)) {
    l->cur++;
  }
  if (l->cur == l->end) return {};
  while (!char_is_digit(*l->cur)) {
    ++l->cur;
  }
  u8* current = l->cur;
  while (char_is_digit(*l->cur)) {
    ++l->cur;
  }
  String result = str_make(current, (u64)l->cur - (u64)current);
  return result;
}

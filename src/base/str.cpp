#include "base_impl.h"

read_only u64 pow10[] = {
  1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000,
  1000000000ull, 10000000000ull, 100000000000ull, 1000000000000ull,
  10000000000000ull, 100000000000000ull, 1000000000000000ull,
  10000000000000000ull, 100000000000000000ull, 1000000000000000000ull,
  10000000000000000000ull
};

#define HEX_LENGTH 16
read_only global u8 HEX[] = "0123456789ABCDEF";

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

u64 cstr_length(const void* c) {
  u8* p = (u8*)c;
  for (; *p != 0; ++p);
  return (u64)(p - (u8*)c);
}

u32 u64_length(u64 x) {
  u32 n = 1;
  while (n < 20 && x >= pow10[n]) n++;
  return n;
}
u32 u32_length(u32 x) { return u64_length(x); }

u32 u64_write(u8* dest, u64 x) {
  u32 length = u64_length(x);
  u32 i = length;
  do {
    dest[--i] = '0' + (x % 10);
    x /= 10;
  } while (x);
  return length;
}
u32 u32_write(u8* dest, u32 x) { return u64_write(dest, x); }

u32 i64_length(i64 x) {
  if (x < 0)
    return 1+u64_length(-x);
  else
    return u64_length(x);
}
u32 i32_length(i32 x) { return i64_length(x); }

u32 i64_write(u8* dest, i64 x) {
  if (x < 0) {
    dest[0] = '-';
    return 1 + u64_write(dest+1, -x);
  }
  else {
    return u64_write(dest, x);
  }
}
u32 i32_write(u8* dest, i32 x) { return i64_write(dest, x); }

u32 f64_length(f64 x, u32 precision) {
  u32 len = 0;
  if (x < 0.0f) {
    ++len;
    x = -x;
  }
  
  // Process the integer part
  u64 integer_part = x;
  x -= integer_part; // Remove the integer part
  len += u64_length(integer_part);

  // Process the fractional part
  if (precision > 0) {
    ++len; // for '.'
    len += precision;
  }
  return len;
}
u32 f32_length(f32 x, u32 precision) { return f64_length(x, precision); }

u32 f64_write(u8* dest, f64 x, u32 precision) {
  u32 len = 0;
  if (x < 0.0f) {
    dest[len++] = '-';
    x = -x;
  }

  // Process the integer part
  u64 integer_part = x;
  x -= integer_part; // Remove the integer part
  len += u64_write(dest + len, integer_part);

  // Process the fractional part
  if (precision > 0) {
    dest[len++] = '.';
    u64 multiplier = 1;
    for (u32 i = 0; i < precision; ++i) multiplier *= 10;
    u64 frac_part = (u64)(x * multiplier + 0.5); // rounding since possible 0.02 * multi = 1.999...
    u32 frac_len = u64_length(frac_part);
    u32 pad = (frac_len < precision) ? (precision - frac_len) : 0;
    for (u32 i = 0; i < pad; ++i) {
      dest[len++] = '0';
    }
    len += u64_write(dest + len, frac_part);
  }
  return len;
}
u32 f32_write(u8* dest, f32 x, u32 precision) { return f64_write(dest, x, precision); }

void u64_hex_write(u8* dest, u64 value) {
  LoopReverse (i, HEX_LENGTH) {
    dest[i] = HEX[value & 0xF];
    value >>= 4;
  }
}

////////////////////////////////////////////////////////////////////////
// Great sprintf

struct Writer {
  u8* buf;
  u32 pos;
};

void w_bytes(Writer* w, u8* src, u32 len) {
  if (w->buf) MemCopy(w->buf + w->pos, src, len);
  w->pos += len;
}

void w_byte(Writer* w, u8 b) {
  if (w->buf) w->buf[w->pos] = b;
  w->pos += 1;
}

intern u32 my_sprintf(u8* buf, String fmt, VaList argc) {
  Writer w = {buf, 0};
  for (u8 *p = fmt.str, *end = p + fmt.size; p < end; ++p) {
    if (*p != '%') { w_byte(&w, *p); continue; }
    ++p; // skip '%'
    switch (*p) {
      // default: {
      //   Info("%s", str_make(p-60, 120));
      // }
      InvalidDefaultCase;
      case 'i': {
        if (p + 2 < end && (p[1] == '6' && p[2] == '4')) {
          p += 2;
          i64 val = va_arg(argc, i64);
          u8 tmp[32];
          u32 len = i64_write(tmp, val);
          w_bytes(&w, tmp, len);
        } else {
          i32 val = va_arg(argc, i32);
          u8 tmp[32];
          u32 len = i32_write(tmp, val);
          w_bytes(&w, tmp, len);
        }
      } break;
      case 'u': {
        if (p + 2 < end && (p[1] == '6' && p[2] == '4')) {
          p += 2;
          u64 val = va_arg(argc, u64);
          u8 tmp[32];
          u32 len = u64_write(tmp, val);
          w_bytes(&w, tmp, len);
        } else {
          u32 val = va_arg(argc, u32);
          u8 tmp[32];
          u32 len = u32_write(tmp, val);
          w_bytes(&w, tmp, len);
        }
      } break;
      case 'f': {
        f64 val = va_arg(argc, f64);
        if (is_inf(val)) {
          String inf = "inf";
          w_bytes(&w, inf.str, inf.size);
        } else if (is_nan(val)) {
          String nan = "nan";
          w_bytes(&w, nan.str, nan.size);
        } else {
          u32 float_precision = 6;
          u8 tmp[64];
          u32 len = f64_write(tmp, val, float_precision);
          w_bytes(&w, tmp, len);
        }
      } break;
      case '.': {
        u32 precision = *++p - '0';
        ++p; // skip number, land on 'f'
        f64 val = va_arg(argc, f64);
        if (is_inf(val)) {
          String inf = "inf";
          w_bytes(&w, inf.str, inf.size);
        } else if (is_nan(val)) {
          String nan = "nan";
          w_bytes(&w, nan.str, nan.size);
        } else {
          u8 tmp[64];
          u32 len = f64_write(tmp, val, precision);
          w_bytes(&w, tmp, len);
        }
      } break;
      case 's': {
        String val = va_arg(argc, String);
        w_bytes(&w, val.str, val.size);
      } break;
      case 'c': {
        w_byte(&w, (u8)va_arg(argc, i32));
      } break;
      case 'p': {
        u64 val = va_arg(argc, u64);
        u8 tmp[HEX_LENGTH];
        u64_hex_write(tmp, val);
        w_bytes(&w, tmp, HEX_LENGTH);
      } break;
      case '%': {
        w_byte(&w, '%');
      } break;
    }
  }
  return w.pos;
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
b32 char_is_number_cont(u8 c)  { return char_is_digit(c) || c == '.' || c == '-'; }

////////////////////////////////////////////////////////////////////////
// String Constructors

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

String str_range(u8* first, u8* one_past_last) {
  String result = str_make(first, (u32)(one_past_last - first));
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

u64 str_find_needle(String hay, String needle) {
  if (needle.size == 0 || needle.size > hay.size) return hay.size;
  u8 first_char = needle.str[0];
  u8 last_char = needle.str[needle.size-1];
  u8* p = hay.str;
  u8* end = hay.str + hay.size - needle.size + 1;
  for (;p < end; ++p) {
    if (p[0] == first_char && p[needle.size-1] == last_char) {
      if (str_match(str_range(p, p+needle.size), needle)) {
        return u64(p - hay.str);
      }
    }
  }
  return hay.size;
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
  VaList tmp;
  va_copy(tmp, args); 
  u32 need_bytes = my_sprintf(null, fmt, tmp);
  va_end(tmp);
  u8* buf = push_buffer(arena, need_bytes + 1);
  va_copy(tmp, args); 
  u32 final_size = my_sprintf(buf, fmt, tmp);
  va_end(tmp);
  String res = str_make(buf, final_size);
  res.str[res.size] = 0;
  return res;
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

StringNode* str_list_push_node(StringList& list, StringNode* node) {
  sll_list_queue_push(list, node);
  ++list.node_count;
  list.total_size += node->string.size;
  return node;
}

StringNode* str_list_push_node_set_string(StringList& list, StringNode* node, String string) {
  sll_list_queue_push(list, node);
  ++list.node_count;
  list.total_size += string.size;
  node->string = string;
  return node;
}

StringNode* str_list_push(Allocator arena, StringList& list, String string) {
  StringNode* node = push_struct(arena, StringNode);
  str_list_push_node_set_string(list, node, string);
  return node;
}

StringNode* str_list_pushf(Allocator arena, StringList& list, String fmt, ...) {
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

String str_trim(String str) {
  u8 *s = str.str, *e = s + str.size;
  while (s < e && char_is_space(*s)) ++s;
  while (e > s && char_is_space(*(e - 1))) --e;
  return str_make(s, u32(e - s));
}

i32 str_index_of_char(String string, u8 c) {
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

u8* str_find_last_slash(String s) {
  u8* p = s.str + s.size - 1;
  for (; p >= s.str; --p) {
    if (char_is_slash(*p)) return p;
  }
  return s.str - 1;
}

String str_chop_last_slash(String s) {
  u8* p = str_find_last_slash(s);
  if (p >= s.str) s.size = u32(p - s.str);
  return s;
}

String str_chop_past_last_slash(String s) {
  u8* p = str_find_last_slash(s);
  if (p >= s.str) s.size = u32(p - s.str) + 1;
  return s;
}

String str_skip_slash(String s) {
  u8* p = s.str;
  for (; p < s.str+s.size; ++p) {
    if (char_is_slash(*p)) {
      ++p;
      return str_make(p, u32(s.str + s.size - p));
    }
  }
  return s;
}

String str_skip_last_slash(String s) {
  u8* p = str_find_last_slash(s);
  if (p >= s.str) {
    p += 1;
    s.size = (u32)(s.str + s.size - p);
    s.str = p;
  }
  return s;
}

String str_chop_last_dot(String s) {
  String res = s;
  u32 p = s.size;
  for (; p > 0;) {
    p -= 1;
    if (s.str[p] == '.') {
      res = str_prefix(s, p);
      break;
    }
  }
  return res;
}

String str_skip_last_dot(String s) {
  u8* ptr = s.str + s.size - 1;
  for (; ptr >= s.str; --ptr) {
    if (*ptr == '.') {
      break;
    }
  }
  if (ptr >= s.str) {
    ptr += 1;
    s.size = (u32)(s.str + s.size - ptr);
    s.str = ptr;
  }
  return s;
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

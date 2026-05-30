#include "tokenizer.h"

global String tokens_str_names[] = {
#define X(name) Stringify(name),
  TOKEN_TYPE_LIST
#undef X
};

Slice<Token> tokens_from_str(Allocator arena, String string) {
  var tokens = darray_make<Token>(arena);
  u32 off = 0;
  u8* str = string.str;
  u32 column = 1;
  u32 line = 1;
  for (u32 advance = 0; off < string.size; off += advance) {
    TokenType token_type = TokenType_Null;
    u8 byte      = str[off+0];
    u8 next_byte = (off+1 < string.size) ? str[off+1] : 0;
    var is_end = [&]() {return off+advance >= string.size;};
    var is_next_end = [&]() {return off+advance+1 >= string.size;};
    var cur_byte = [&]() {return str[off+advance];};
    var cur_next_byte = [&]() {return str[off+advance+1];};
    u32 token_line = line;
    switch (byte) {
      case '(': token_type = TokenType_OpenParen; advance = 1; break; 
      case ')': token_type = TokenType_CloseParen; advance = 1; break; 
      case ':': token_type = TokenType_Colon; advance = 1; break;
      case ';': token_type = TokenType_Semicolon; advance = 1; break;
      case '*': token_type = TokenType_Asterisk; advance = 1; break;
      case '[': token_type = TokenType_OpenBracket; advance = 1; break;
      case ']': token_type = TokenType_CloseBracket; advance = 1; break;
      case '{': token_type = TokenType_OpenBrace; advance = 1; break;
      case '}': token_type = TokenType_CloseBrace; advance = 1; break;
      case '=': token_type = TokenType_Equals; advance = 1; break;
      case ',': token_type = TokenType_Comma; advance = 1; break;
      case '|': token_type = TokenType_Or; advance = 1; break;
      case '#': token_type = TokenType_Pound; advance = 1; break;
      case '-': token_type = TokenType_Minus; advance = 1; break;
      case '"': token_type = TokenType_String; advance = 1; {
        while (!is_end() && str[off + advance] != '"') {
          if (cur_byte() == '\\' && !is_next_end()) {
            ++advance;
          }
          ++advance;
        }
        if (cur_byte() == '"') {
          ++advance;
        }
        ++off;
      } break;
      default: {
        if (char_is_space(byte)) {
          token_type = TokenType_Spacing;
          advance = 1;
        } else if (byte == '\r') {
          token_type = TokenType_NewLine;
          advance = 2;
          ++line;
        } else if (byte == '\n') {
          token_type = TokenType_NewLine;
          advance = 1;
          ++line;
        } else if (byte == '/' && next_byte == '/') {
          token_type = TokenType_Comment;
          advance = 2;
          while (!is_end() && !char_is_newline(cur_byte())) {
            ++advance;
          }
        } else if (byte == '/' && next_byte == '*') { // TODO: Handle column offset on new lines here
          token_type = TokenType_Comment;
          advance = 2;
          while (!is_end() && !is_next_end() && !((cur_byte() == '*') && (cur_next_byte() == '/'))) {
            if (cur_byte() == '\n'){
              ++line;
            }
            ++advance;
          }
          if(!is_end() && cur_byte() == '*') {
            advance += 2;
          }
        } else if (char_is_alpha(byte)) {
          token_type = TokenType_Identifier;
          advance = 1;
          while (!is_end() && (char_is_alpha(cur_byte()) || char_is_digit(cur_byte()) || cur_byte() == '_')) {
            ++advance;
          }
        } else if (char_is_digit(byte)) {
          token_type = TokenType_Number;
          advance = 1;
          while (!is_end() && char_is_digit(cur_byte())) {
            ++advance;
          }
          if (!is_end() && cur_byte() == '.') {
            ++advance;
            while (!is_end() && char_is_digit(cur_byte())) {
              ++advance;
            }
          }
        } else {
          token_type = TokenType_Null;
          advance = 1;
        }
      }
    }
    u32 tok_len = advance;
    if (token_type == TokenType_String) {
      tok_len -= 2;
    }
    Token token = {
      .type = token_type,
      .str = str_make(str+off, tok_len),
      .column = column,
      .line = token_line,
    };
    if (token_type == TokenType_NewLine) {
      column = 1;
    } else {
      column += advance;
    }
    array_push(tokens, token);
  }
  return slice(tokens);
}

Parser parser_make(Slice<Token> tokens) {
  Parser res = {
    .tokens = tokens,
  };
  return res;
}

b32 tok_is_trivia(TokenType type) { return type == TokenType_Spacing || type == TokenType_NewLine || type == TokenType_Comment; }
void tok_skip_trivia(Parser& t) {
  while (t.i < t.tokens.count && tok_is_trivia(t.tokens[t.i].type)) {
    ++t.i;
  }
}
b32 tok_is_end(Parser& t) {
  tok_skip_trivia(t);
  return t.i >= t.tokens.count;
}
Token tok_peek(Parser& t) {
  if (tok_is_end(t)) Error("unexpected end");
  return t.tokens[t.i];
}
Token tok_prev(Parser& t) {
  return t.tokens[t.i-1];
}
Token tok_advance(Parser& t) {
  Token tok = tok_peek(t);
  ++t.i;
  return tok;
}
b32 tok_check(Parser& t, TokenType type) {
  if (tok_is_end(t)) return false;
  return tok_peek(t).type == type;
}
b32 tok_match(Parser& t, TokenType type) {
  if (tok_check(t, type)) {
    tok_advance(t);
    return true;
  }
  return false;
}
Token tok_require(Parser& t, TokenType type) {
  if (!tok_check(t, type)) {
    Token tok = tok_peek(t);
    Error("expected token type %s, got %s, line: %u, column: %u, token: '%s'", tokens_str_names[type], tok.str, tok.line, tok.column, tok.str);
  }
  return tok_advance(t);
}
b32 tok_ident_check(Parser& t, String name) {
  if (!tok_check(t, TokenType_Identifier)) {
    return false;
  }
  return str_match(tok_peek(t).str, name);
}
b32 tok_ident_match(Parser& t, String name) {
  if (tok_ident_check(t, name)) {
    tok_advance(t);
    return true;
  }
  return false;
}
Token tok_ident_require(Parser& t, String name) {
  if (!tok_ident_check(t, name)) {
    Token tok = tok_peek(t);
    Error("expected '%s', got '%s'", name, tok.str);
  }
  return tok_advance(t);
}

f32 parse_f32(Parser& t) {
  b32 negative = false;
  if (tok_match(t, TokenType_Minus)) {
    negative = true;
  }
  Token tok = tok_require(t, TokenType_Number);
  f32 v = f32_from_str(tok.str);
  return negative ? -v : v;
}
f32 parse_u32(Parser& t) {
  b32 negative = false;
  if (tok_match(t, TokenType_Minus)) {
    negative = true;
  }
  Token tok = tok_require(t, TokenType_Number);
  i32 v = u32_from_str(tok.str);
  return negative ? -v : v;
}
f32 parse_i32(Parser& t) {
  b32 negative = false;
  if (tok_match(t, TokenType_Minus)) {
    negative = true;
  }
  Token tok = tok_require(t, TokenType_Number);
  i32 v = i32_from_str(tok.str);
  return negative ? -v : v;
}

v3 parse_v3(Parser& t) {
  return v3( parse_f32(t), parse_f32(t), parse_f32(t));
}

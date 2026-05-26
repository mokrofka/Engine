#include "tokenizer.h"

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
    Token token = {
      .type = token_type,
      .str = String(str+off, advance),
      .column = column,
      .line = token_line,
    };
    if (token_type == TokenType_NewLine) {
      column = 1;
    } else {
      column += advance;
    }
    darray_add(tokens, token);
  }
  return slice(tokens);
}

b32 tok_is_trivia(TokenType type) { return type == TokenType_Spacing || type == TokenType_NewLine || type == TokenType_Comment; }
void tok_skip_trivia(Tokenizer& t) {
  while (t.i < t.tokens.count && tok_is_trivia(t.tokens[t.i].type)) {
    ++t.i;
  }
}
b32 tok_eof(Tokenizer& t) {
  tok_skip_trivia(t);
  return t.i >= t.tokens.count;
}
Token tok_peek(Tokenizer& t) {
  if (tok_eof(t))
    Error("unexpected eof");
  return t.tokens[t.i];
}
Token tok_next(Tokenizer& t) {
  Token tok = tok_peek(t);
  ++t.i;
  return tok;
}
b32 tok_match(Tokenizer& t, TokenType type) {
  if (!tok_eof(t) && t.tokens[t.i].type == type) {
    ++t.i;
    return true;
  }
  return false;
}
Token tok_require(Tokenizer& t, TokenType type) {
  Token tok = tok_next(t);
  if (tok.type != type) {
    Error("expected token type %i, got %i", type, tok.type);
    return {};
  }
  return tok;
}
Token tok_require_ident(Tokenizer& t, String expected) {
  Token tok = tok_require(t, TokenType_Identifier);
  if (!str_match(tok.str, expected)) {
    Error("expected '%s', got '%s'", expected, tok.str);
    return {};
  }
  return tok;
}

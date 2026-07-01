#pragma once
#include "lib.h"

#define TOKEN_TYPE_LIST     \
  X(TokenType_Null)         \
                            \
  X(TokenType_OpenParen)    \
  X(TokenType_CloseParen)   \
  X(TokenType_Colon)        \
  X(TokenType_Semicolon)    \
  X(TokenType_Asterisk)     \
  X(TokenType_OpenBracket)  \
  X(TokenType_CloseBracket) \
  X(TokenType_OpenBrace)    \
  X(TokenType_CloseBrace)   \
  X(TokenType_Equals)       \
  X(TokenType_Comma)        \
  X(TokenType_Or)           \
  X(TokenType_Pound)        \
  X(TokenType_Minus)        \
                            \
  X(TokenType_String)       \
  X(TokenType_Identifier)   \
  X(TokenType_Number)       \
                            \
  X(TokenType_Spacing)      \
  X(TokenType_NewLine)      \
  X(TokenType_Comment)

enum TokenType {
#define X(name) name,
  TOKEN_TYPE_LIST
#undef X
};

struct Token {
  TokenType type;
  String str;
  u32 column;
  u32 line;
};

Slice<Token> tokens_from_str(Allocator arena, String string);

struct Parser {
  Slice<Token> tokens;
  u32 i;
};

Parser parser_make(Slice<Token> tokens);

b32 tok_is_trivia(TokenType type);
void tok_skip_trivia(Parser& p);
b32 tok_is_end(Parser& p);
Token tok_peek(Parser& p);
Token tok_prev(Parser& p);
Token tok_advance(Parser& p);
b32 tok_check(Parser& p, TokenType type);
b32 tok_match(Parser& p, TokenType type);
Token tok_require(Parser& p, TokenType type);
b32 tok_ident_check(Parser& p, String name);
b32 tok_ident_match(Parser& p, String name);
Token tok_ident_require(Parser& p, String name);

f32 parse_f32(Parser& p);
f32 parse_u32(Parser& p);
f32 parse_i32(Parser& p);

v3 parse_v3(Parser& p);


#pragma once
#include "lib.h"

enum TokenType {
  TokenType_Null,

  TokenType_OpenParen,
  TokenType_CloseParen,
  TokenType_Colon,
  TokenType_Semicolon,
  TokenType_Asterisk,
  TokenType_OpenBracket,
  TokenType_CloseBracket,
  TokenType_OpenBrace,
  TokenType_CloseBrace,
  TokenType_Equals,
  TokenType_Comma,
  TokenType_Or,
  TokenType_Pound,
  TokenType_Minus,

  TokenType_String,
  TokenType_Identifier,
  TokenType_Number,

  TokenType_Spacing,
  TokenType_NewLine,
  TokenType_Comment,
};

struct Token {
  TokenType type;
  String str;
  u32 column;
  u32 line;
};

Slice<Token> tokens_from_str(Allocator arena, String string);

struct Tokenizer {
  Slice<Token> tokens;
  u32 i;
};

b32 tok_is_trivia(TokenType type);
void tok_skip_trivia(Tokenizer& t);
b32 tok_eof(Tokenizer& t);
Token tok_peek(Tokenizer& t);
Token tok_next(Tokenizer& t);
b32 tok_match(Tokenizer& t, TokenType type);
Token tok_require(Tokenizer& t, TokenType type);
Token tok_require_ident(Tokenizer& t, String expected);

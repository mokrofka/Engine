#pragma once
#include "base/lib.h"

#define TOKEN_TYPE_LIST    \
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
	// X(TokenType_Spacing)      \
	// X(TokenType_NewLine)      \
	// X(TokenType_Comment)

enum TokenType {
#define X(name) name,
	TOKEN_TYPE_LIST
#undef X
};

#define MetaTypeLIST \
	X(MetaType_Null) \
	X(MetaType_u8) \
	X(MetaType_u32) \
	X(MetaType_i32) \
	X(MetaType_b32) \
	X(MetaType_f32) \
	X(MetaType_String) \
	X(MetaType_v2) \
	X(MetaType_v3) \
	X(MetaType_v4) \
	X(MetaType_Rng2) \
	X(MetaType_Rng3) \

enum MetaType {
	#define X(x) x,
	MetaTypeLIST
	#undef X
	MetaType_COUNT,
};

struct Token {
	TokenType type;
	String str;
	u32 column;
	u32 line;
};

struct Parser {
	Slice<Token> tokens;
	u32 cur;
};



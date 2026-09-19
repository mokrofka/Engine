#include "meta.h"

global String tokens_str_names[] = {
#define X(name) Stringify(name),
	TOKEN_TYPE_LIST
#undef X
};

global String meta_type_str[] = {
#define X(x) [x] = Stringify(x),
	MetaTypeLIST
#undef X
};

Slice<Token> tokens_from_str(Allocator arena, String string) {
	var tokens = array_make<Token>(arena);
	u32 off = 0;
	u8* str = string.str;
	u32 column = 1;
	u32 line = 1;
	for(u32 advance = 0; off < string.size; off += advance) {
		TokenType token_type = TokenType_Null;
		u8 byte = str[off+0];
		u8 next_byte = (off+1 < string.size) ? str[off+1] : 0;
#define is_end 						 (off+advance >= string.size)
#define is_next_end 	 (off+advance+1 >= string.size)
#define cur_byte 				 (str[off+advance])
#define cur_next_byte (str[off+advance+1])
		u32 token_line = line;
		column += advance;
		switch(byte) {
			default: {
				if(char_is_space(byte)) {
					// token_type = TokenType_Spacing;
					advance = 1;
					column++;
					continue;
				} else if(byte == '\r') {
					// token_type = TokenType_NewLine;
					column = -1;
					advance = 2;
					line++;
					continue;
				} else if(byte == '\n') {
					// token_type = TokenType_NewLine;
					column = 0;
					advance = 1;
					line++;
					continue;
				} else if(byte == '/' && next_byte == '/') {
					// token_type = TokenType_Comment;
					advance = 2;
					while(!is_end && !char_is_newline(cur_byte)) {
						advance++;
					}
					continue;
			} else if(byte == '/' && next_byte == '*') { // TODO: Handle column offset on new lines here
					// token_type = TokenType_Comment;
					advance = 2;
					while(!is_end && !is_next_end && !((cur_byte == '*') && (cur_next_byte == '/'))) {
						if(cur_byte == '\n'){
							line++;
						}
						advance++;
					}
					if(!is_end && cur_byte == '*') {
						advance += 2;
					}
					continue;
				} else if(char_is_alpha(byte)) {
					token_type = TokenType_Identifier;
					advance = 1;
					while(!is_end && (char_is_alpha(cur_byte) || char_is_digit(cur_byte) || cur_byte == '_')) {
						advance++;
					}
				} else if(char_is_digit(byte)) {
					token_type = TokenType_Number;
					advance = 1;
					while(!is_end && char_is_digit(cur_byte)) {
						advance++;
					}
					if(!is_end && cur_byte == '.') {
						advance++;
						while(!is_end && char_is_digit(cur_byte)) {
							advance++;
						}
					}
				} else {
					token_type = TokenType_Null;
					advance = 1;
				}
			}break;
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
				while(!is_end && cur_byte != '"') {
					advance++;
				}
				if(cur_byte == '"') {
					advance++;
				}
				off++;
			}break;
		}

		///////////////////////////////////
		// push token
		u32 tok_len = advance;
		if(token_type == TokenType_String) {
			tok_len -= 2;
		}
		Token token = {
			.type = token_type,
			.str = String(str+off, tok_len),
			.column = column,
			.line = token_line,
		};
		array_push(tokens, token);
	}
	return slice(tokens);

#undef is_end
#undef is_next_end
#undef cur_byte
#undef cur_next_byte
}

Parser parser_make(Slice<Token> tokens) {
	Parser res = {
		.tokens = tokens,
	};
	return res;
}

Token tok_peek(Parser& p) {
	Assert(p.cur < p.tokens.count);
	return p.tokens[p.cur];
}
Token tok_peek(Parser& p, u32 off) {
	Assert(p.cur+off < p.tokens.count);
	return p.tokens[p.cur+off];
}
Token tok_prev(Parser& p) {
	return p.tokens[p.cur-1];
}
Token tok_advance(Parser& p) {
	Assert(p.cur < p.tokens.count);
	return p.tokens[p.cur++];
}
b32 tok_match(Parser& p, TokenType type) {
	Token t = tok_peek(p);
	if(t.type == type) {
		tok_advance(p);
		return true;
	}
	return false;
}
b32 tok_match_name(Parser& p, String name) {
	Token t = tok_peek(p);
	if(t.type == TokenType_Identifier && str_match(t.str, name)) {
		tok_advance(p);
		return true;
	}
	return false;
}
Token tok_expect(Parser& p, TokenType type) {
	Token t = tok_peek(p);
	if(t.type != type) {
		AssertMsg("expected token type %s, got %s, line: %u, column: %u, token: '%s'", tokens_str_names[type], t.str, t.line, t.column, t.str);
	}
	return tok_advance(p);
}
Token tok_expect_name(Parser& p, String name) {
	Token t = tok_peek(p);
	if(t.type == TokenType_Identifier && str_match(t.str, name)) {
		AssertMsg("expected '%s', got '%s'", name, t.str);
	}
	return tok_advance(p);
}


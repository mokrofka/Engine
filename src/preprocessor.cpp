#include "lib.h"
#include "tokenizer.h"

struct Tokenizer {
  Slice<Token> tokens;
  u32 i;
};

b32 is_trivia(TokenType type) { return type == TokenType_Spacing || type == TokenType_NewLine || type == TokenType_Comment; }
void skip_trivia(Tokenizer& t) {
  while (t.i < t.tokens.count && is_trivia(t.tokens[t.i].type)) {
    ++t.i;
  }
}
b32 eof(Tokenizer& t) {
  skip_trivia(t);
  return t.i >= t.tokens.count;
}
Token peek(Tokenizer& t) {
  if (eof(t))
    Error("unexpected eof");
  return t.tokens[t.i];
}
Token next(Tokenizer& t) {
  Token tok = peek(t);
  ++t.i;
  return tok;
}
b32 match(Tokenizer& t, TokenType type) {
  if (!eof(t) && t.tokens[t.i].type == type) {
    ++t.i;
    return true;
  }
  return false;
}
Token require(Tokenizer& t, TokenType type) {
  Token tok = next(t);
  if (tok.type != type) {
    Error("expected token type %i, got %i", type, tok.type);
    return {};
  }
  return tok;
}
Token require_ident(Tokenizer& t, String expected) {
  Token tok = require(t, TokenType_Identifier);
  if (!str_match(tok.str, expected)) {
    Error("expected '%s', got '%s'", expected, tok.str);
    return {};
  }
  return tok;
}

i32 main(i32 count, char* args[]) {
  tctx_init();
  os_init(args[0]);
  Scratch scratch;
  OS_Handle file = os_file_open(push_strf(scratch, "%s/../src/generated.h", os_get_current_directory()), OS_AccessFlag_Write | OS_AccessFlag_Trunc);
  var string = dstr_make(scratch);
  Slice buf = os_file_path_read_all(scratch, push_strf(scratch, "%s/../src/com.h", os_get_current_directory()));
  Slice tokens = tokens_from_str(scratch, String(buf.data, buf.size));
  Tokenizer t = {.tokens = tokens};
  while (!eof(t)) {
    Token tok = next(t);
    switch (tok.type) {
      default:{} break;
      case TokenType_Identifier: {
        if (str_match(tok.str, "Introspect")) {
          require_ident(t, "struct");
          Token name = require(t, TokenType_Identifier);
          dstr_add(string, push_strf(scratch, "MemberDefinition members_of_%s[] = {\n", name.str));
          require(t, TokenType_OpenBrace);
          while (!match(t, TokenType_CloseBrace)) {
            Token type = require(t, TokenType_Identifier);
            b32 is_pointer = false;
            if (peek(t).type == TokenType_Asterisk) {
              next(t);
              is_pointer = true;
            }
            Token field = require(t, TokenType_Identifier);
            require(t, TokenType_Semicolon);
            String s = push_strf(scratch, "  {MetaType_%s, \"%s\", OffsetOf(%s,%s)},\n", type.str, field.str, name.str, field.str);
            dstr_add(string, s);
          }
          dstr_add(string, String("};\n"));
        }
      } break;
    }
  }
  os_file_write(file, string.size, string.str);
  os_exit(0);
}

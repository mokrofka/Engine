#include "lib.h"
#include "tokenizer.h"

i32 main(i32 count, char* args[]) {
  tctx_init();
  os_init(args[0]);
  Scratch scratch;
  OS_Handle file = os_file_open(push_strf(scratch, "%s/../src/generated.h", os_get_current_directory()), OS_AccessFlag_Write | OS_AccessFlag_Trunc);
  var string = dstr_make(scratch);
  Slice buf = os_file_path_read_all(scratch, push_strf(scratch, "%s/../src/com.h", os_get_current_directory()));
  Slice tokens = tokens_from_str(scratch, String(buf.data, buf.size));
  Tokenizer t = {.tokens = tokens};
  while (!tok_eof(t)) {
    Token tok = tok_next(t);
    switch (tok.type) {
      default:{} break;
      case TokenType_Identifier: {
        if (str_match(tok.str, "Introspect")) {
          tok_require_ident(t, "struct");
          Token name = tok_require(t, TokenType_Identifier);
          dstr_add(string, push_strf(scratch, "MemberDefinition members_of_%s[] = {\n", name.str));
          tok_require(t, TokenType_OpenBrace);
          while (!tok_match(t, TokenType_CloseBrace)) {
            Token type = tok_require(t, TokenType_Identifier);
            b32 is_pointer = false;
            if (tok_peek(t).type == TokenType_Asterisk) {
              tok_next(t);
              is_pointer = true;
            }
            Token field = tok_require(t, TokenType_Identifier);
            tok_require(t, TokenType_Semicolon);
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

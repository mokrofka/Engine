#include "lib.h"
#include "tokenizer.h"

i32 main(i32 count, char* args[]) {
  tctx_init();
  os_init(args[0]);
  Scratch scratch;
  OS_Handle file = os_file_open(push_strf(scratch, "%s/../src/generated.h", os_get_current_directory()), OS_AccessFlag_Write | OS_AccessFlag_Trunc);
  Slice buf = os_file_path_read_all(scratch, push_strf(scratch, "%s/../src/com.h", os_get_current_directory()));
  Slice tokens = tokens_from_str(scratch, str_make(buf));
  Parser p = parser_make(tokens);
  var string = dstr_make(scratch);
  while (!tok_is_end(p)) {
    Token tok = tok_advance(p);
    switch (tok.type) {
      default:{} break;
      case TokenType_Identifier: {
        if (str_match(tok.str, "Introspect")) {
          tok_ident_require(p, "struct");
          Token struct_name = tok_require(p, TokenType_Identifier);
          dstr_add(string, push_strf(scratch, "MemberDefinition members_of_%s[] = {\n", struct_name.str));
          tok_require(p, TokenType_OpenBrace);
          while (!tok_match(p, TokenType_CloseBrace)) {
            Token field_type = tok_require(p, TokenType_Identifier);
            // b32 is_pointer = false;
            // if (tok_peek(p).type == TokenType_Asterisk) {
            //   tok_advance(p);
            //   is_pointer = true;
            // }
            Token field_name = tok_require(p, TokenType_Identifier);
            tok_require(p, TokenType_Semicolon);
            String s = push_strf(scratch, "  {MetaType_%s, \"%s\", OffsetOf(%s,%s)},\n", field_type.str, field_name.str, struct_name.str, field_name.str);
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

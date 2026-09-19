#include "base/base.cpp"
#include "meta.cpp"

i32 main(i32 count, char** args) {
	Scratch scratch;
	tctx_init();
	os_init(Slice(args, count));
	Map<u8, 32> saved = {};
	LoopEnumNonZero(i, MetaType) {
		map_set(saved, hash(meta_type_str[i]), {});
	}

	String files[] = {
		"com.h",
		"types.h",
	};
	Array<String, ArrayCount(files)> buffers = {};
	LoopArray(i, files) {
		buffers[i] = os_file_path_read_all_str(scratch, push_strf(scratch, "%s/../src/%s", os_cur_directory(), files[i]));
	}
	
	var string = dstr_make(scratch);
	var enum_meta_type_string = dstr_make(scratch);
	dstr_push(enum_meta_type_string, "enum {\n");
	b32 first_enum_meta_type = true;

	LoopArray(i, files) {
		Slice tokens = tokens_from_str(scratch, buffers[i]);
		Parser p = parser_make(tokens);
		while(p.cur < p.tokens.count) {
			Token tok = tok_advance(p);
			if(tok.type == TokenType_Identifier) {
				if(str_match(tok.str, "Introspect")) {
					tok_expect_name(p, "struct");
					Token struct_name = tok_expect(p, TokenType_Identifier);
					dstr_push(string, push_strf(scratch, "MemberDefinition members_of_%s[] = {\n", struct_name.str));
					tok_expect(p, TokenType_OpenBrace);
					while(!tok_match(p, TokenType_CloseBrace)) {
						Token field_type = tok_expect(p, TokenType_Identifier);
						Token field_name = tok_expect(p, TokenType_Identifier);
						if(tok_match(p, TokenType_OpenBracket)) {
							tok_expect(p, TokenType_Number);
							tok_expect(p, TokenType_CloseBracket);
						}
						tok_expect(p, TokenType_Semicolon);
						String s = push_strf(scratch, "\t{MetaType_%s, \"%s\", OffsetOf(%s,%s)},\n", field_type.str, field_name.str, struct_name.str, field_name.str);
						dstr_push(string, s);

						// Enum meta type
						String meta_type = push_strf(scratch, "MetaType_%s", field_type.str);
						u64 h = hash(meta_type);
						if(var[_, ok] = map_get(saved, h); !ok) {
							meta_type = push_strf(scratch, "\t%s", meta_type);
							if(first_enum_meta_type) {
								meta_type = push_strf(scratch, "%s = %u", meta_type, MetaType_COUNT);
								first_enum_meta_type = false;
							}
							meta_type = push_strf(scratch, "%s,\n", meta_type);
							map_set(saved, h, {});
							dstr_push(enum_meta_type_string, meta_type);
						}
					}
					dstr_push(string, String("};\n"));
				}
			}
		}
	}
	dstr_push(enum_meta_type_string, "};\n");
	dstr_push(enum_meta_type_string, string);
	os_file_path_write_all(push_strf(scratch, "%s/../src/generated.h", os_cur_directory()), dstr_slice(enum_meta_type_string));
	os_exit(0);
}


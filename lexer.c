#include "lexer.h"
#include "light_array.h"
#include <string.h>

// https://docs.microsoft.com/en-us/cpp/c-language/c-floating-point-constants?view=vs-2017

static bool
is_letter(char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static bool
is_number(char c) {
	return c >= '0' && c <= '9';
}

static bool
string_equal_token(const char* s, Token* t) {
    s32 i = 0;
    for(; i < t->length; ++i) {
        if(s[i] != t->data[i]) return false;
    }

    if(s[i]) return false;

    return true;
}

static void
match_keyword(Token* t) {
    t->flags |= TOKEN_FLAG_KEYWORD;

    if(0) {
    } else if(string_equal_token("int", t)) {
        t->type = TOKEN_KEYWORD_INT; t->flags |= TOKEN_FLAG_TYPE_KEYWORD;
    } else if(string_equal_token("float", t)) {
        t->type = TOKEN_KEYWORD_FLOAT; t->flags |= TOKEN_FLAG_TYPE_KEYWORD;
    } else if(string_equal_token("double", t)) {
        t->type = TOKEN_KEYWORD_DOUBLE; t->flags |= TOKEN_FLAG_TYPE_KEYWORD;
    } else if(string_equal_token("long", t)) {
        t->type = TOKEN_KEYWORD_LONG; t->flags |= TOKEN_FLAG_TYPE_KEYWORD;
    } else if(string_equal_token("void", t)) {
        t->type = TOKEN_KEYWORD_VOID; t->flags |= TOKEN_FLAG_TYPE_KEYWORD;
    } else if(string_equal_token("char", t)) {
        t->type = TOKEN_KEYWORD_CHAR; t->flags |= TOKEN_FLAG_TYPE_KEYWORD;
    } else if(string_equal_token("short", t)) {
        t->type = TOKEN_KEYWORD_SHORT; t->flags |= TOKEN_FLAG_TYPE_KEYWORD;
    } else if(string_equal_token("signed", t)) {
        t->type = TOKEN_KEYWORD_SIGNED; t->flags |= TOKEN_FLAG_TYPE_KEYWORD;
    } else if(string_equal_token("unsigned", t)) {
        t->type = TOKEN_KEYWORD_UNSIGNED; t->flags |= TOKEN_FLAG_TYPE_KEYWORD;
    } else if(string_equal_token("auto", t)) {
        t->type = TOKEN_KEYWORD_AUTO;
    } else if(string_equal_token("break", t)) {
        t->type = TOKEN_KEYWORD_BREAK;
    } else if(string_equal_token("case", t)) {
        t->type = TOKEN_KEYWORD_CASE;
    } else if(string_equal_token("const", t)) {
        t->type = TOKEN_KEYWORD_CONST;
    } else if(string_equal_token("continue", t)) {
        t->type = TOKEN_KEYWORD_CONTINUE;
    } else if(string_equal_token("default", t)) {
        t->type = TOKEN_KEYWORD_DEFAULT;
    } else if(string_equal_token("do", t)) {
        t->type = TOKEN_KEYWORD_DO;
    } else if(string_equal_token("else", t)) {
        t->type = TOKEN_KEYWORD_ELSE;
    } else if(string_equal_token("enum", t)) {
        t->type = TOKEN_KEYWORD_ENUM;
    } else if(string_equal_token("extern", t)) {
        t->type = TOKEN_KEYWORD_EXTERN;
    } else if(string_equal_token("for", t)) {
        t->type = TOKEN_KEYWORD_FOR;
    } else if(string_equal_token("goto", t)) {
        t->type = TOKEN_KEYWORD_GOTO;
    } else if(string_equal_token("if", t)) {
        t->type = TOKEN_KEYWORD_IF;
    } else if(string_equal_token("inline", t)) {
        t->type = TOKEN_KEYWORD_INLINE;
    } else if(string_equal_token("register", t)) {
        t->type = TOKEN_KEYWORD_REGISTER;
    } else if(string_equal_token("restrict", t)) {
        t->type = TOKEN_KEYWORD_RESTRICT;
    } else if(string_equal_token("return", t)) {
        t->type = TOKEN_KEYWORD_RETURN;
    } else if(string_equal_token("sizeof", t)) {
        t->type = TOKEN_KEYWORD_SIZEOF;
    } else if(string_equal_token("static", t)) {
        t->type = TOKEN_KEYWORD_STATIC;
    } else if(string_equal_token("struct", t)) {
        t->type = TOKEN_KEYWORD_STRUCT;
    } else if(string_equal_token("switch", t)) {
        t->type = TOKEN_KEYWORD_SWITCH;
    } else if(string_equal_token("typedef", t)) {
        t->type = TOKEN_KEYWORD_TYPEDEF;
    } else if(string_equal_token("union", t)) {
        t->type = TOKEN_KEYWORD_UNION;
    } else if(string_equal_token("volatile", t)) {
        t->type = TOKEN_KEYWORD_VOLATILE;
    } else if(string_equal_token("while", t)) {
        t->type = TOKEN_KEYWORD_WHILE;
    } else {
        t->flags &= (~TOKEN_FLAG_KEYWORD);
    }
}

static Token
token_number(u8* at, s32 line, s32 column) {
    Token r = {0};
    r.line = line;
    r.column = column;
    r.data = at;

    bool floating = false;

    while(true) {
        if(*at == '.' && floating == false) { 
            floating = true;
            ++at;
            continue;
        } else if(is_number(*at)) {
            ++at;
        } else {
            break;
        }
    }

    if(floating) {
        // e suffix
        if(*at == 'e' || *at == 'E') {
            ++at;
            if(*at == '-' || *at == '+') {
                ++at;
            }
            while(is_number(*at)) ++at;
        }

        // f | F L suffixes
        if(*at == 'f' || *at == 'F') {
            r.type = TOKEN_FLOAT_LITERAL;
            ++at;
        } else if(*at == 'l' || *at == 'L') {
            r.type = TOKEN_LONG_DOUBLE_LITERAL;
            ++at;
        } else {
            r.type = TOKEN_DOUBLE_LITERAL;
            ++at;
        }
    } else {
        bool uns = false;
        s32 long_count = 0;
        // u U | l L | ll LL | ul UL | ull ULL suffixes
        if(*at == 'u' || *at == 'U') {
            ++at;
            uns = true;
        }
        if(*at == 'l') {
            ++at; ++long_count;
            if(*at == 'l') {
                ++at; ++long_count;
            }
        }
        if(*at == 'L') {
            ++at; ++long_count;
            if(*at == 'L') {
                ++at; ++long_count;
            }
        }
        if(uns) {
            r.type = TOKEN_INT_U_LITERAL + long_count;
        } else {
            r.type = TOKEN_INT_LITERAL + long_count;
        }
    }

    r.length = at - r.data;

    return r;
}

static bool
is_hex_digit(char c) {
	return (is_number(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

static Token
token_next(Lexer* lexer) {
	u8* at = lexer->stream + lexer->index;
	if (!at) return (Token){ 0 };

	Token r = { 0 };
	r.data = at;
	r.line = (s32)lexer->line;
	r.column = (s32)lexer->column;

	switch (*at) {
		case 0: return (Token) { 0 };

        case '<':{
            ++at;
            if(*at == '=') {
                r.type = TOKEN_LESS_EQUAL;
                ++at;
            } else if(*at == '<') {
                ++at;
                if(*at == '=') {
                    r.type = TOKEN_SHL_EQUAL;
                } else {
                    r.type = TOKEN_BITSHIFT_LEFT;
                }
            } else {
                r.type = '<';
            }
            r.length = at - r.data;
        }break;
        case '>':{
            ++at;
            if(*at == '=') {
                r.type = TOKEN_GREATER_EQUAL;
                ++at;
            } else if(*at == '>') {
                ++at;
                if(*at == '=') {
                    r.type = TOKEN_SHR_EQUAL;
                } else {
                    r.type = TOKEN_BITSHIFT_RIGHT;
                }
            } else {
                r.type = '>';
            }
            r.length = at - r.data;
        }break;
        case '!':{
            ++at;
            if(*at == '=') {
                r.type = TOKEN_LESS_EQUAL;
                ++at;
            } else {
                r.type = '!';
            }
            r.length = at - r.data;
        }break;
        case '|':{
            ++at;
            if(*at == '=') {
                r.type = TOKEN_OR_EQUAL;
                ++at;
            } else if(*at == '|') {
                r.type = TOKEN_LOGIC_OR;
                ++at;
            } else {
                r.type = '|';
            }
            r.length = at - r.data;
        }break;
        case '=':{
            ++at;
            if(*at == '=') {
                r.type = TOKEN_EQUAL_EQUAL;
                ++at;
            } else {
                r.type = '=';
            }
            r.length = at - r.data;
        }break;
        case '/':{
            ++at;
            if(*at == '=') {
                r.type = TOKEN_DIV_EQUAL;
                ++at;
            } else {
                r.type = '/';
            }
            r.length = at - r.data;
        }break;
		case '&': {
            ++at;
            if(*at == '=') {
                r.type = TOKEN_AND_EQUAL;
                ++at;
            } else if(*at == '&') {
                r.type = TOKEN_LOGIC_AND;
                ++at;
            } else {
                r.type = '&';
            }
            r.length = at - r.data;
        }break;
        case '+':{
            ++at;
            if(*at == '=') {
                r.type = TOKEN_PLUS_EQUAL;
                ++at;
            } else if(*at == '+') {
                r.type = TOKEN_PLUS_PLUS;
                ++at;
            } else {
                r.type = '+';
            }
            r.length = at - r.data;
        }break; 
        case '-':{
            ++at;
            if(*at == '=') {
                r.type = TOKEN_MINUS_EQUAL;
                ++at;
            } else if(*at == '-') {
                r.type = TOKEN_MINUS_MINUS;
                ++at;
            } else if(*at == '>') {
                r.type = TOKEN_ARROW;
                ++at;
            } else {
                r.type = '-';
            }
            r.length = at - r.data;
        }break;
		case '~':{
            ++at;
            if(*at == '=') {
                r.type = TOKEN_NOT_EQUAL;
                ++at;
            } else {
                r.type = '~';
            }
            r.length = at - r.data;
        }break;
		case '%':{
            ++at;
            if(*at == '=') {
                r.type = TOKEN_MOD_EQUAL;
                ++at;
            } else {
                r.type = '%';
            }
            r.length = at - r.data;
        }break;
        case '*': {
            ++at;
            if(*at == '=') {
                r.type = TOKEN_TIMES_EQUAL;
                ++at;
            } else {
                r.type = '*';
            }
            r.length = at - r.data;
        }break;
        case '^':{
            ++at;
            if(*at == '=') {
                r.type = TOKEN_XOR_EQUAL;
                ++at;
            } else {
                r.type = '^';
            }
            r.length = at - r.data;
        }break;

        case '\'': {
			// TODO(psv): Implement Long suffix  L' c-char-sequence '
            r.type = TOKEN_CHAR_LITERAL;
            at++;
            if (*at == '\'') {
                at++;
                // TODO(psv): empty character constant error
            } else if(*at == '\\') {
                ++at;
                switch (*at) {
                    case 'a':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                    case 'e':
                    case '\\':
                    case '\'':
                    case '"':
                    case '?':
                        at++;
                        break;
                    case 0:
                        break;
                    case 'x':
                        at++;
                        if (is_hex_digit(*at) || is_number(*at)) {
                            at++;
                            if (is_hex_digit(*at) || is_number(*at)) {
                                at++;
                            }
                        } else {
                            //printf("invalid escape sequence '\\x%c", *at);
                        }
                    default: {
                        //printf("invalid escape sequence '\\x%c", *at);
                    }break;
                }
                if(*at != '\'') {
                    //printf("expected end of character literal");
                }
                ++at;
            } else {
                ++at;
            }
            r.length = at - r.data;
        } break;

        case '.': {
            if(!is_number(at[1])) {
                r.type = *at;
                r.length = 1;
                ++at;
                break;
            }
        }

		// Token string
		case '"': {
			r.type = TOKEN_STRING_LITERAL;
			at++;	// skip "

			for (; *at != '"'; ++at) {
				if (*at == 0) {
					break;
				} else if (*at == '\\') {
					at++;
					if (*at == '"') {
						at++;
					} else {
						switch (*at) {
						case 'a':
						case 'b':
						case 'f':
						case 'n':
						case 'r':
						case 't':
						case 'v':
						case 'e':
						case '\\':
						case '\'':
						case '"':
						case '?':
							break;
						case 0:
							break;
						case 'x':
							at++;
							if (is_hex_digit(*at) || is_number(*at)) {
								at++;
								if (is_hex_digit(*at) || is_number(*at)) {
									at++;
								}
							} else {
								//printf("invalid escape sequence '\\x%c", *at);
							}
						default: {
							//printf("invalid escape sequence '\\x%c", *at);
						}break;
						}
					}
				}
			}
			at++; // skip "
			r.length = at - r.data;
		} break;

		default: {
            if(*at == '.') {
                // float starting with .
                r = token_number(at, lexer->line, lexer->column);
                break;
            } else if (is_number(*at)) {
				if (*at == '0' && at[1] == 'x') {
					// hex
					at += 2;
					while (*at && is_hex_digit(*at)) {
						++at;
					}
					r.type = TOKEN_INT_HEX_LITERAL;
                } else if(*at == '0' && is_number(at[1])) {
                    // octal
                    at += 1;
                    while(*at && is_number(*at)) {
                        ++at;
                    }
                    r.type = TOKEN_INT_OCT_LITERAL;
                } else if(*at == '0' && at[1] == 'b') {
                    // binary
                    at += 2;
                    while(*at && (*at == '1' || *at == '0')) {
                        ++at;
                    }
                    r.type = TOKEN_INT_BIN_LITERAL;
				} else {
					r = token_number(at, lexer->line, lexer->column);
                    break;
				}
			} else if (is_letter(*at) || *at == '_') {
                // identifier
				for (; is_letter(*at) || is_number(*at) || *at == '_'; ++at);
				r.type = TOKEN_IDENTIFIER;

                r.length = at - r.data;
                match_keyword(&r);
                break;
			} else {
				r.type = *at;
			    r.length = 1;
                ++at;
			}
			r.length = at - r.data;
		} break;
	}

	lexer->stream += r.length;
	lexer->column += r.length;
	return r;
}

static void
lexer_eat_whitespace(Lexer* lexer) {
    while(true) {
        u8 c = lexer->stream[lexer->index];
        if (c == ' ' ||
		    c == '\t' ||
		    c == '\v' ||
		    c == '\f' ||
		    c == '\r') 
        {
            lexer->index++;
            lexer->column++;
        } else if(c == '\n') {
            lexer->index++;
            lexer->line++;
            lexer->column = 0;
        } else {
            if(c == '/' && lexer->stream[lexer->index + 1] == '/') {
                // single line comment
                lexer->index += 2;
                while(lexer->stream[lexer->index++] != '\n');
                lexer->line++;
                lexer->column = 0;
            } else if(c == '/' && lexer->stream[lexer->index + 1] == '*') {
                // multi line comment
                lexer->index += 2;
                u8* at = lexer->stream + lexer->index;
                while(true) {
                    if(*at == '*' && at[1] == '/') break;
                    if(*at == '\n') {
                        lexer->line++;
                        lexer->column = 0;
                    }
                    ++at;
                    ++lexer->index;
                }
                lexer->index += 2;
            } else {
                break;
            }
        }
    }
}

#include <stdio.h>
void
token_print(Token t) {
    /* FOREGROUND */
    #define RST  "\x1B[0m"
    #define KRED  "\x1B[31m"
    #define KGRN  "\x1B[32m"
    #define KYEL  "\x1B[33m"
    #define KBLU  "\x1B[34m"
    #define KMAG  "\x1B[35m"
    #define KCYN  "\x1B[36m"
    #define KWHT  "\x1B[37m"

    if(t.flags & TOKEN_FLAG_KEYWORD) {
        if(t.flags & TOKEN_FLAG_TYPE_KEYWORD) {
            printf("%s%d:%d: %.*s%s\n", KBLU, t.line, t.column, t.length, t.data, RST);
        } else {
            printf("%s%d:%d: %.*s%s\n", KGRN, t.line, t.column, t.length, t.data, RST);
        }
    } else {
        printf("%d:%d: %.*s\n", t.line, t.column, t.length, t.data);
    }
}

Token* 
lexer_cstr(Lexer* lexer, char* str, s32 length, u32 flags) {
    lexer->stream = str;

	Token* tokens = array_new(Token);

    while(true) {
        lexer_eat_whitespace(lexer);
        Token t = token_next(lexer);

        if(t.type == TOKEN_EOF) break;

        // token_print(t);

        // push token
		array_push(tokens, t);
    }

	lexer->tokens = tokens;
	lexer->index = 0;

    return tokens;
}

Token* 
lexer_next(Lexer* lexer) {
	return &lexer->tokens[lexer->index++];
}

Token*
lexer_peek(Lexer* lexer) {
	return &lexer->tokens[lexer->index];
}

const char* 
token_to_str(Token* token) {
	return token_type_to_str(token->type);
}

const char* 
token_type_to_str(Token_Type token_type) {
	switch (token_type) {
		TOKEN_EOF: return "end of stream";
		TOKEN_IDENTIFIER: return "identifier";
		TOKEN_CHAR_LITERAL: return "character literal";

		TOKEN_STRING_LITERAL: return "string literal";
		TOKEN_INT_HEX_LITERAL: return "hexadecimal literal";
		TOKEN_INT_BIN_LITERAL: return "binary literal";
		TOKEN_INT_OCT_LITERAL: return "octal literal";
		TOKEN_INT_U_LITERAL: return "unsigned integer literal";
		TOKEN_INT_UL_LITERAL: return "unsigned long integer literal";
		TOKEN_INT_ULL_LITERAL: return "unsgined long long integer literal";
		TOKEN_INT_LITERAL: return "integer literal";
		TOKEN_INT_L_LITERAL: return "long integer literal";
		TOKEN_INT_LL_LITERAL: return "long long integer literal";
		TOKEN_FLOAT_LITERAL: return "float literal";
		TOKEN_DOUBLE_LITERAL: return "double literal";
		TOKEN_LONG_DOUBLE_LITERAL: return "long double literal";

		TOKEN_ARROW: return "->";
		TOKEN_EQUAL_EQUAL: return "==";
		TOKEN_LESS_EQUAL: return "<=";
		TOKEN_GREATER_EQUAL: return ">=";
		TOKEN_LOGIC_NOT_EQUAL: return "!=";
		TOKEN_LOGIC_OR: return "||";
		TOKEN_LOGIC_AND: return "&&";
		TOKEN_BITSHIFT_LEFT: return "<<";
		TOKEN_BITSHIFT_RIGHT: return ">>";

		TOKEN_PLUS_EQUAL: return "+=";
		TOKEN_MINUS_EQUAL: return "-=";
		TOKEN_TIMES_EQUAL: return "*=";
		TOKEN_DIV_EQUAL: return "/=";
		TOKEN_MOD_EQUAL: return "%=";
		TOKEN_AND_EQUAL: return "&=";
		TOKEN_OR_EQUAL: return "|=";
		TOKEN_XOR_EQUAL: return "^=";
		TOKEN_SHL_EQUAL: return "<<=";
		TOKEN_SHR_EQUAL: return ">>=";
		TOKEN_NOT_EQUAL: return "!=";

		TOKEN_PLUS_PLUS: return "++";
		TOKEN_MINUS_MINUS: return "--";

			// Type keywords
		TOKEN_KEYWORD_INT: return "int";
		TOKEN_KEYWORD_FLOAT: return "float";
		TOKEN_KEYWORD_DOUBLE: return "double";
		TOKEN_KEYWORD_LONG: return "long";
		TOKEN_KEYWORD_VOID: return "void";
		TOKEN_KEYWORD_CHAR: return "char";
		TOKEN_KEYWORD_SHORT: return "short";
		TOKEN_KEYWORD_SIGNED: return "signed";
		TOKEN_KEYWORD_UNSIGNED: return "unsigned";

			// Keywords
		TOKEN_KEYWORD_AUTO: return "auto";
		TOKEN_KEYWORD_BREAK: return "break";
		TOKEN_KEYWORD_CASE: return "case";
		TOKEN_KEYWORD_CONST: return "const";
		TOKEN_KEYWORD_CONTINUE: return "continue";
		TOKEN_KEYWORD_DEFAULT: return "default";
		TOKEN_KEYWORD_DO: return "do";
		TOKEN_KEYWORD_ELSE: return "else";
		TOKEN_KEYWORD_ENUM: return "enum";
		TOKEN_KEYWORD_EXTERN: return "extern";
		TOKEN_KEYWORD_FOR: return "for";
		TOKEN_KEYWORD_GOTO: return "goto";
		TOKEN_KEYWORD_IF: return "if";
		TOKEN_KEYWORD_INLINE: return "inline";
		TOKEN_KEYWORD_REGISTER: return "register";
		TOKEN_KEYWORD_RESTRICT: return "restrict";
		TOKEN_KEYWORD_RETURN: return "return";
		TOKEN_KEYWORD_SIZEOF: return "sizeof";
		TOKEN_KEYWORD_STATIC: return "static";
		TOKEN_KEYWORD_STRUCT: return "struct";
		TOKEN_KEYWORD_SWITCH: return "switch";
		TOKEN_KEYWORD_TYPEDEF: return "typedef";
		TOKEN_KEYWORD_UNION: return "union";
		TOKEN_KEYWORD_VOLATILE: return "volatile";
		TOKEN_KEYWORD_WHILE: return "while";
	}

	return "unknown";
}
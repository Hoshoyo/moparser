#include "common.h"
#include "moparser.h"

typedef MO_Token Token;
typedef MO_Lexer Lexer;

static Token* lexer_file(Lexer* lexer, const char* filename, u32 flags);
static Token* lexer_cstr(Lexer* lexer, char* str, s32 length, u32 flags);
static Token* lexer_next(Lexer* lexer);
static Token* lexer_peek(Lexer* lexer);
static Token* lexer_peek_n(Lexer* lexer, s32 n);
static void   lexer_rewind(Lexer* lexer, s32 count);
static void   lexer_free(Lexer* lexer);

static const char* token_to_str(Token* token);
static const char* token_type_to_str(MO_Token_Type token_type);
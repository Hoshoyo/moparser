#include "common.h"

typedef enum {
    TOKEN_EOF = 0,
    
    TOKEN_IDENTIFIER = 256,
    
    TOKEN_CHAR_LITERAL,
    TOKEN_STRING_LITERAL,
    TOKEN_INT_HEX_LITERAL,
    TOKEN_INT_BIN_LITERAL,
    TOKEN_INT_OCT_LITERAL,
    TOKEN_INT_U_LITERAL,
    TOKEN_INT_UL_LITERAL,
    TOKEN_INT_ULL_LITERAL,
    TOKEN_INT_LITERAL,
    TOKEN_INT_L_LITERAL,
    TOKEN_INT_LL_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_DOUBLE_LITERAL,
    TOKEN_LONG_DOUBLE_LITERAL,

    TOKEN_ARROW,
    TOKEN_EQUAL_EQUAL,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_EQUAL,
    TOKEN_LOGIC_NOT_EQUAL,
    TOKEN_LOGIC_OR,
    TOKEN_LOGIC_AND,
    TOKEN_BITSHIFT_LEFT,
    TOKEN_BITSHIFT_RIGHT,

    TOKEN_PLUS_EQUAL,
	TOKEN_MINUS_EQUAL,
	TOKEN_TIMES_EQUAL,
	TOKEN_DIV_EQUAL,
	TOKEN_MOD_EQUAL,
	TOKEN_AND_EQUAL,
	TOKEN_OR_EQUAL,
	TOKEN_XOR_EQUAL,
	TOKEN_SHL_EQUAL,
	TOKEN_SHR_EQUAL,
    TOKEN_NOT_EQUAL,

    TOKEN_PLUS_PLUS,
    TOKEN_MINUS_MINUS,

    // Type keywords
    TOKEN_KEYWORD_INT,
    TOKEN_KEYWORD_FLOAT,
    TOKEN_KEYWORD_DOUBLE,
    TOKEN_KEYWORD_LONG,
    TOKEN_KEYWORD_VOID,
    TOKEN_KEYWORD_CHAR,
    TOKEN_KEYWORD_SHORT,
    TOKEN_KEYWORD_SIGNED,
    TOKEN_KEYWORD_UNSIGNED,

    // Keywords
    TOKEN_KEYWORD_AUTO,
    TOKEN_KEYWORD_BREAK,
    TOKEN_KEYWORD_CASE,
    TOKEN_KEYWORD_CONST,
    TOKEN_KEYWORD_CONTINUE,
    TOKEN_KEYWORD_DEFAULT,
    TOKEN_KEYWORD_DO,
    TOKEN_KEYWORD_ELSE,
    TOKEN_KEYWORD_ENUM,
    TOKEN_KEYWORD_EXTERN,
    TOKEN_KEYWORD_FOR,
    TOKEN_KEYWORD_GOTO,
    TOKEN_KEYWORD_IF,
    TOKEN_KEYWORD_INLINE,
    TOKEN_KEYWORD_REGISTER,
    TOKEN_KEYWORD_RESTRICT,
    TOKEN_KEYWORD_RETURN,
    TOKEN_KEYWORD_SIZEOF,
    TOKEN_KEYWORD_STATIC,
    TOKEN_KEYWORD_STRUCT,
    TOKEN_KEYWORD_SWITCH,
    TOKEN_KEYWORD_TYPEDEF,
    TOKEN_KEYWORD_UNION,
    TOKEN_KEYWORD_VOLATILE,
    TOKEN_KEYWORD_WHILE,
} Token_Type;

typedef enum {
    TOKEN_FLAG_KEYWORD = FLAG(0),
    TOKEN_FLAG_TYPE_KEYWORD = FLAG(1),
    TOKEN_FLAG_ASSIGNMENT_OPERATOR = FLAG(2),
} Token_Flags;

typedef struct {
    Token_Type type;
    s32 line;
    s32 column;
    u8* data;
    s32 length;
    u32 flags;
} Token;

typedef struct {
    char*  filename;
    s32    line;
    s32    column;
    Token* tokens;
    u8*    stream;
    s32    index;
} Lexer;

Token* lexer_file(Lexer* lexer, const char* filename, u32 flags);
Token* lexer_cstr(Lexer* lexer, char* str, s32 length, u32 flags);
Token* lexer_next(Lexer* lexer);
Token* lexer_peek(Lexer* lexer);
Token* lexer_peek_n(Lexer* lexer, s32 n);
void   lexer_rewind(Lexer* lexer, s32 count);

void   lexer_free(Lexer* lexer);

const char* token_to_str(Token* token);
const char* token_type_to_str(Token_Type token_type);
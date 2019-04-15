#ifndef H_MOPARSER
#define H_MOPARSER

typedef enum {
    MO_TOKEN_FLAG_KEYWORD             = (1 << 0),
    MO_TOKEN_FLAG_TYPE_KEYWORD        = (1 << 1),
    MO_TOKEN_FLAG_ASSIGNMENT_OPERATOR = (1 << 2),
} MO_Token_Flags;

typedef enum {
    MO_TOKEN_EOF = 0,
    
    MO_TOKEN_IDENTIFIER = 256,
    
    MO_TOKEN_CHAR_LITERAL,
    MO_TOKEN_STRING_LITERAL,
    MO_TOKEN_INT_HEX_LITERAL,
    MO_TOKEN_INT_BIN_LITERAL,
    MO_TOKEN_INT_OCT_LITERAL,
    MO_TOKEN_INT_U_LITERAL,
    MO_TOKEN_INT_UL_LITERAL,
    MO_TOKEN_INT_ULL_LITERAL,
    MO_TOKEN_INT_LITERAL,
    MO_TOKEN_INT_L_LITERAL,
    MO_TOKEN_INT_LL_LITERAL,
    MO_TOKEN_FLOAT_LITERAL,
    MO_TOKEN_DOUBLE_LITERAL,
    MO_TOKEN_LONG_DOUBLE_LITERAL,

    MO_TOKEN_ARROW,
    MO_TOKEN_EQUAL_EQUAL,
    MO_TOKEN_LESS_EQUAL,
    MO_TOKEN_GREATER_EQUAL,
    MO_TOKEN_LOGIC_NOT_EQUAL,
    MO_TOKEN_LOGIC_OR,
    MO_TOKEN_LOGIC_AND,
    MO_TOKEN_BITSHIFT_LEFT,
    MO_TOKEN_BITSHIFT_RIGHT,

    MO_TOKEN_PLUS_EQUAL,
	MO_TOKEN_MINUS_EQUAL,
	MO_TOKEN_TIMES_EQUAL,
	MO_TOKEN_DIV_EQUAL,
	MO_TOKEN_MOD_EQUAL,
	MO_TOKEN_AND_EQUAL,
	MO_TOKEN_OR_EQUAL,
	MO_TOKEN_XOR_EQUAL,
	MO_TOKEN_SHL_EQUAL,
	MO_TOKEN_SHR_EQUAL,
    MO_TOKEN_NOT_EQUAL,

    MO_TOKEN_PLUS_PLUS,
    MO_TOKEN_MINUS_MINUS,

    // Type keywords
    MO_TOKEN_KEYWORD_INT,
    MO_TOKEN_KEYWORD_FLOAT,
    MO_TOKEN_KEYWORD_DOUBLE,
    MO_TOKEN_KEYWORD_LONG,
    MO_TOKEN_KEYWORD_VOID,
    MO_TOKEN_KEYWORD_CHAR,
    MO_TOKEN_KEYWORD_SHORT,
    MO_TOKEN_KEYWORD_SIGNED,
    MO_TOKEN_KEYWORD_UNSIGNED,

    // Keywords
    MO_TOKEN_KEYWORD_AUTO,
    MO_TOKEN_KEYWORD_BREAK,
    MO_TOKEN_KEYWORD_CASE,
    MO_TOKEN_KEYWORD_CONST,
    MO_TOKEN_KEYWORD_CONTINUE,
    MO_TOKEN_KEYWORD_DEFAULT,
    MO_TOKEN_KEYWORD_DO,
    MO_TOKEN_KEYWORD_ELSE,
    MO_TOKEN_KEYWORD_ENUM,
    MO_TOKEN_KEYWORD_EXTERN,
    MO_TOKEN_KEYWORD_FOR,
    MO_TOKEN_KEYWORD_GOTO,
    MO_TOKEN_KEYWORD_IF,
    MO_TOKEN_KEYWORD_INLINE,
    MO_TOKEN_KEYWORD_REGISTER,
    MO_TOKEN_KEYWORD_RESTRICT,
    MO_TOKEN_KEYWORD_RETURN,
    MO_TOKEN_KEYWORD_SIZEOF,
    MO_TOKEN_KEYWORD_STATIC,
    MO_TOKEN_KEYWORD_STRUCT,
    MO_TOKEN_KEYWORD_SWITCH,
    MO_TOKEN_KEYWORD_TYPEDEF,
    MO_TOKEN_KEYWORD_UNION,
    MO_TOKEN_KEYWORD_VOLATILE,
    MO_TOKEN_KEYWORD_WHILE,
} MO_Token_Type;

typedef struct {
    MO_Token_Type  type;
    int            line;
    int            column;
    unsigned char* data;
    int            length;
    unsigned int   flags;
} MO_Token;

typedef enum {
	MO_PARSER_STATUS_OK = 0,
	MO_PARSER_STATUS_FATAL,
} MO_Parser_Status;

typedef struct {
	struct Ast_t*    node;
	MO_Parser_Status status;
	const char*       error_message;
} MO_Parser_Result;

typedef struct {
    char*          filename;
    int            line;
    int            column;
    MO_Token*      tokens;
    unsigned char* stream;
    int            index;
} MO_Lexer;

MO_Token*        mop_lexer_cstr(MO_Lexer* lexer, char* str, int length);
MO_Parser_Result mop_parse_expression(MO_Lexer* lexer);
MO_Parser_Result mop_parse_expression_cstr(const char* str);
MO_Parser_Result mop_parse_typename(MO_Lexer* lexer);
MO_Parser_Result mop_parse_typename_cstr(const char* str);
void             mop_print_ast(struct Ast_t* ast);

#endif // H_MOPARSER
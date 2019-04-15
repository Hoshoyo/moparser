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
	struct MO_Ast_t*    node;
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


typedef enum {
	MO_TYPE_NONE = 0,
	MO_TYPE_VOID,
	MO_TYPE_PRIMITIVE,
	MO_TYPE_STRUCT,
	MO_TYPE_UNION,
	MO_TYPE_ENUM,
	MO_TYPE_ALIAS, // typedef
} MO_Type_Kind;

typedef enum {
	MO_TYPE_PRIMITIVE_CHAR = 0,
	MO_TYPE_PRIMITIVE_SHORT,
	MO_TYPE_PRIMITIVE_INT,
	MO_TYPE_PRIMITIVE_LONG,
	MO_TYPE_PRIMITIVE_FLOAT,
	MO_TYPE_PRIMITIVE_DOUBLE,
	MO_TYPE_PRIMITIVE_SIGNED,
	MO_TYPE_PRIMITIVE_UNSIGNED,
} MO_Type_Primitive;

typedef enum {
	STORAGE_CLASS_NONE     = 0,
	STORAGE_CLASS_AUTO     = FLAG(0), // auto
    STORAGE_CLASS_REGISTER = FLAG(1), // register
    STORAGE_CLASS_STATIC   = FLAG(2), // static
    STORAGE_CLASS_EXTERN   = FLAG(3), // extern
    STORAGE_CLASS_TYPEDEF  = FLAG(4), // typedef
} Storage_Class;

typedef enum {
	MO_TYPE_QUALIFIER_CONST = FLAG(0),
	MO_TYPE_QUALIFIER_VOLATILE = FLAG(1),
} MO_Type_Qualifier;

typedef struct {
	MO_Type_Kind kind;
	unsigned int qualifiers;
	unsigned int storage_class;
	union {
		MO_Type_Primitive primitive[8];
		MO_Token*         alias;
		struct {
			struct MO_Ast_t* struct_desc;
			MO_Token*     struct_name;
		};
		struct {
			struct MO_Ast_t* enumerator_list;
			MO_Token*     enum_name;
		};
	};
} MO_Ast_Specifier_Qualifier;

typedef enum {
	MO_POSTFIX_ARRAY_ACCESS = '[',
	MO_POSTFIX_PROC_CALL = '(',
	MO_POSTFIX_DOT = '.',
	MO_POSTFIX_ARROW = MO_TOKEN_ARROW,
	MO_POSTFIX_PLUS_PLUS = MO_TOKEN_PLUS_PLUS,
	MO_POSTFIX_MINUS_MINUS = MO_TOKEN_MINUS_MINUS,
} MO_Postfix_Operator;

typedef enum {
	MO_UNOP_PLUS_PLUS = MO_TOKEN_PLUS_PLUS,
	MO_UNOP_MINUS_MINUS = MO_TOKEN_MINUS_MINUS,
	MO_UNOP_ADDRESS_OF = '&',
	MO_UNOP_DEREFERENCE = '*',
	MO_UNOP_PLUS = '+',
	MO_UNOP_MINUS = '-',
	MO_UNOP_NOT_BITWISE = '~',
	MO_UNOP_NOT_LOGICAL = '!',
} MO_Unary_Operator;

typedef enum {
	MO_DIRECT_ABSTRACT_DECL_NONE = 0,
	MO_DIRECT_ABSTRACT_DECL_NAME,
	MO_DIRECT_ABSTRACT_DECL_ARRAY,
	MO_DIRECT_ABSTRACT_DECL_FUNCTION,
} MO_Direct_Abstract_Decl_Type;

typedef enum {
	MO_BINOP_PLUS  = '+',
	MO_BINOP_MINUS = '-',
	MO_BINOP_MULT  = '*',
	MO_BINOP_DIV   = '/',
	MO_BINOP_MOD   = '%',
	MO_BINOP_SHL   = MO_TOKEN_BITSHIFT_LEFT,
	MO_BINOP_SHR   = MO_TOKEN_BITSHIFT_RIGHT,
	MO_BINOP_LT    = '<',
	MO_BINOP_LE    = MO_TOKEN_LESS_EQUAL,
	MO_BINOP_GT    = '>',
	MO_BINOP_GE    = MO_TOKEN_GREATER_EQUAL,
	MO_BINOP_LOGICAL_EQ = MO_TOKEN_EQUAL_EQUAL,
	MO_BINOP_LOGICAL_NE = MO_TOKEN_NOT_EQUAL,
	MO_BINOP_AND   = '&',
	MO_BINOP_OR    = '|',
	MO_BINOP_XOR   = '^',
	MO_BINOP_EQUAL = '=',
	MO_BINOP_PLUS_EQ = MO_TOKEN_PLUS_EQUAL,
	MO_BINOP_MINUS_EQ = MO_TOKEN_MINUS_EQUAL,
	MO_BINOP_TIMES_EQ = MO_TOKEN_TIMES_EQUAL,
	MO_BINOP_DIV_EQ = MO_TOKEN_DIV_EQUAL,
	MO_BINOP_MOD_EQ = MO_TOKEN_MOD_EQUAL,
	MO_BINOP_XOR_EQ = MO_TOKEN_XOR_EQUAL,
	MO_BINOP_OR_EQ = MO_TOKEN_OR_EQUAL,
	MO_BINOP_AND_EQ = MO_TOKEN_AND_EQUAL,
	MO_BINOP_SHL_EQ = MO_TOKEN_SHL_EQUAL,
	MO_BINOP_SHR_EQ = MO_TOKEN_SHR_EQUAL,
	MO_BINOP_EQUAL_EQUAL = MO_TOKEN_EQUAL_EQUAL,
	MO_BINOP_NOT_EQUAL = MO_TOKEN_NOT_EQUAL,
} MO_Binary_Operator;

typedef enum {
	// Expressions
	MO_AST_EXPRESSION_PRIMARY_IDENTIFIER,
	MO_AST_EXPRESSION_PRIMARY_CONSTANT,
	MO_AST_EXPRESSION_PRIMARY_STRING_LITERAL,
	MO_AST_EXPRESSION_CONDITIONAL,
	MO_AST_EXPRESSION_ASSIGNMENT,
	MO_AST_EXPRESSION_ARGUMENT_LIST,
	MO_AST_EXPRESSION_UNARY,
	MO_AST_EXPRESSION_CAST,
	MO_AST_EXPRESSION_MULTIPLICATIVE,
	MO_AST_EXPRESSION_ADDITIVE,
	MO_AST_EXPRESSION_SHIFT,
	MO_AST_EXPRESSION_RELATIONAL,
	MO_AST_EXPRESSION_EQUALITY,
	MO_AST_EXPRESSION_AND,
	MO_AST_EXPRESSION_EXCLUSIVE_OR,
	MO_AST_EXPRESSION_INCLUSIVE_OR,
	MO_AST_EXPRESSION_LOGICAL_AND,
	MO_AST_EXPRESSION_LOGICAL_OR,
	MO_AST_EXPRESSION_POSTFIX_UNARY,
	MO_AST_EXPRESSION_POSTFIX_BINARY,
	MO_AST_EXPRESSION_TERNARY,
	MO_AST_EXPRESSION_SIZEOF,

	MO_AST_CONSTANT_FLOATING_POINT,
	MO_AST_CONSTANT_INTEGER,
	MO_AST_CONSTANT_ENUMARATION,
	MO_AST_CONSTANT_CHARACTER,

	// Type
	MO_AST_TYPE_NAME,
	MO_AST_TYPE_INFO,
	MO_AST_TYPE_POINTER,
	MO_AST_TYPE_ABSTRACT_DECLARATOR,
	MO_AST_TYPE_DIRECT_ABSTRACT_DECLARATOR,
	MO_AST_TYPE_STRUCT_DECLARATOR,
	MO_AST_TYPE_STRUCT_DECLARATOR_BITFIELD,
	MO_AST_TYPE_STRUCT_DECLARATOR_LIST,

	MO_AST_ENUMERATOR,
	MO_AST_ENUMERATOR_LIST,

	// Params
	MO_AST_PARAMETER_LIST,
	MO_AST_PARAMETER_DECLARATION,
	MO_AST_DIRECT_DECLARATOR,

	// Declaration
	MO_AST_STRUCT_DECLARATION,
	MO_AST_STRUCT_DECLARATION_LIST,
} MO_Node_Kind;

typedef struct {
	struct MO_Ast_t* qualifiers_specifiers;
	struct MO_Ast_t* abstract_declarator;
} MO_Ast_Type_Name;

typedef struct {
	struct MO_Ast_t* qualifiers;
	struct MO_Ast_t* next;
} MO_Ast_Type_Pointer;

typedef struct {
	MO_Binary_Operator bo;
	struct MO_Ast_t* left;
	struct MO_Ast_t* right;
} MO_Ast_Expression_Binary;

typedef struct {
	MO_Token* data;
} MO_Ast_Expression_Primary;

typedef struct {
	struct MO_Ast_t* type_name;
	struct MO_Ast_t* expression;
} MO_Ast_Expression_Cast;

typedef struct {
	MO_Unary_Operator uo;
	struct MO_Ast_t* expr;
} MO_Ast_Expression_Unary;

typedef struct {
	MO_Postfix_Operator po;
	struct MO_Ast_t* expr;
} MO_Ast_Expression_Postfix_Unary;

typedef struct {
	MO_Postfix_Operator po;
	struct MO_Ast_t* left;
	struct MO_Ast_t* right;
} MO_Ast_Expression_Postfix_Binary;

typedef struct {
	struct MO_Ast_t* expr;
	struct MO_Ast_t* next;
} MO_Ast_Expression_Argument_List;

typedef struct {
	struct MO_Ast_t* condition;
	struct MO_Ast_t* case_true;
	struct MO_Ast_t* case_false;
} MO_Ast_Expression_Ternary;

typedef struct {
	struct MO_Ast_t* pointer; // optional
	struct MO_Ast_t* direct_abstract_decl;
} MO_Ast_Abstract_Declarator;

typedef struct {
	MO_Direct_Abstract_Decl_Type type;
	MO_Token* name; // optional
	struct MO_Ast_t* left_opt;
	struct MO_Ast_t* right_opt;
} MO_Ast_Direct_Abstract_Declarator;

typedef struct {
	bool is_vararg;
	struct MO_Ast_t** param_decl;
} MO_Ast_Parameter_List;

typedef struct {
	struct MO_Ast_t* decl_specifiers;
	struct MO_Ast_t* declarator;
} MO_Ast_Parameter_Declaration;

typedef struct {
	bool is_type_name;
	union {
		struct MO_Ast_t* type;
		struct MO_Ast_t* expr;
	};
} MO_Ast_Expression_Sizeof;

typedef struct {
	struct MO_Ast_t* declarator;
} MO_Ast_Struct_Declarator;

typedef struct {
	struct MO_Ast_t* declarator;
	struct MO_Ast_t* const_expr;
} MO_Ast_Struct_Declarator_Bitfield;

typedef struct {
	struct MO_Ast_t** list;
} MO_Ast_Type_Struct_Declarator_List;

typedef struct {
	struct MO_Ast_t* spec_qual;
	struct MO_Ast_t* struct_decl_list;
} MO_Ast_Struct_Declaration;

typedef struct {
	struct MO_Ast_t** list;
} MO_Ast_Struct_Declaration_List;

typedef struct {
	MO_Token* enum_constant;
	struct MO_Ast_t* const_expr;
} MO_Ast_Enumerator;

typedef struct {
	struct MO_Ast_Enumerator** list;
} MO_Ast_Enumerator_List;

typedef struct MO_Ast_t {
	MO_Node_Kind kind;
	union {
		MO_Ast_Expression_Binary expression_binary;
		MO_Ast_Expression_Primary expression_primary;
		MO_Ast_Expression_Cast expression_cast;
		MO_Ast_Expression_Unary expression_unary;
		MO_Ast_Expression_Postfix_Unary expression_postfix_unary;
		MO_Ast_Expression_Postfix_Binary expression_postfix_binary;
		MO_Ast_Expression_Argument_List expression_argument_list;
		MO_Ast_Expression_Ternary expression_ternary;
		MO_Ast_Expression_Sizeof expression_sizeof;
		MO_Ast_Specifier_Qualifier specifier_qualifier;
		MO_Ast_Type_Name type_name;
		MO_Ast_Type_Pointer pointer;
		MO_Ast_Abstract_Declarator abstract_type_decl;
		MO_Ast_Direct_Abstract_Declarator direct_abstract_decl;
		MO_Ast_Parameter_List parameter_list;
		MO_Ast_Parameter_Declaration parameter_decl;
		MO_Ast_Struct_Declarator struct_declarator;
		MO_Ast_Type_Struct_Declarator_List struct_declarator_list;
		MO_Ast_Struct_Declarator_Bitfield struct_declarator_bitfield;
		MO_Ast_Struct_Declaration_List struct_declaration_list;
		MO_Ast_Struct_Declaration struct_declaration;
		MO_Ast_Enumerator enumerator;
		MO_Ast_Enumerator_List enumerator_list;
	};
} MO_Ast;

MO_Token*        mop_lexer_cstr(MO_Lexer* lexer, char* str, int length);
MO_Parser_Result mop_parse_expression(MO_Lexer* lexer);
MO_Parser_Result mop_parse_expression_cstr(const char* str);
MO_Parser_Result mop_parse_typename(MO_Lexer* lexer);
MO_Parser_Result mop_parse_typename_cstr(const char* str);
void             mop_print_ast(struct MO_Ast_t* ast);

#endif // H_MOPARSER
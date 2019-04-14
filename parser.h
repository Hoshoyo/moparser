#include "common.h"
#include "lexer.h"
#include <stdio.h>
#include "light_array.h"

typedef enum {
	TYPE_NONE = 0,
	TYPE_VOID,
	TYPE_PRIMITIVE,
	TYPE_STRUCT,
	TYPE_UNION,
	TYPE_ENUM,
	TYPE_ALIAS, // typedef
} Type_Kind;

typedef enum {
	TYPE_PRIMITIVE_CHAR = 0,
	TYPE_PRIMITIVE_SHORT,
	TYPE_PRIMITIVE_INT,
	TYPE_PRIMITIVE_LONG,
	TYPE_PRIMITIVE_FLOAT,
	TYPE_PRIMITIVE_DOUBLE,
	TYPE_PRIMITIVE_SIGNED,
	TYPE_PRIMITIVE_UNSIGNED,
} Type_Primitive;

typedef enum {
	STORAGE_CLASS_NONE     = 0,
	STORAGE_CLASS_AUTO     = FLAG(0), // auto
    STORAGE_CLASS_REGISTER = FLAG(1), // register
    STORAGE_CLASS_STATIC   = FLAG(2), // static
    STORAGE_CLASS_EXTERN   = FLAG(3), // extern
    STORAGE_CLASS_TYPEDEF  = FLAG(4), // typedef
} Storage_Class;

typedef enum {
	TYPE_QUALIFIER_CONST = FLAG(0),
	TYPE_QUALIFIER_VOLATILE = FLAG(1),
} Type_Qualifier;

typedef struct {
	Type_Kind kind;
	u32       qualifiers;
	u32       storage_class;
	union {
		Type_Primitive primitive[8];
		Token* alias;
		struct {
			struct Ast_t* struct_desc;
			Token* struct_name;
		};
		struct {
			struct Ast_t* enumerator_list;
			Token* enum_name;
		};
	};
} Ast_Specifier_Qualifier;

typedef enum {
	POSTFIX_ARRAY_ACCESS = '[',
	POSTFIX_PROC_CALL = '(',
	POSTFIX_DOT = '.',
	POSTFIX_ARROW = TOKEN_ARROW,
	POSTFIX_PLUS_PLUS = TOKEN_PLUS_PLUS,
	POSTFIX_MINUS_MINUS = TOKEN_MINUS_MINUS,
} Postfix_Operator;

typedef enum {
	UNOP_PLUS_PLUS = TOKEN_PLUS_PLUS,
	UNOP_MINUS_MINUS = TOKEN_MINUS_MINUS,
	UNOP_ADDRESS_OF = '&',
	UNOP_DEREFERENCE = '*',
	UNOP_PLUS = '+',
	UNOP_MINUS = '-',
	UNOP_NOT_BITWISE = '~',
	UNOP_NOT_LOGICAL = '!',
} Unary_Operator;

typedef enum {
	DIRECT_ABSTRACT_DECL_NONE = 0,
	DIRECT_ABSTRACT_DECL_NAME,
	DIRECT_ABSTRACT_DECL_ARRAY,
	DIRECT_ABSTRACT_DECL_FUNCTION,
} Direct_Abstract_Decl_Type;

typedef enum {
	BINOP_PLUS  = '+',
	BINOP_MINUS = '-',
	BINOP_MULT  = '*',
	BINOP_DIV   = '/',
	BINOP_MOD   = '%',
	BINOP_SHL   = TOKEN_BITSHIFT_LEFT,
	BINOP_SHR   = TOKEN_BITSHIFT_RIGHT,
	BINOP_LT    = '<',
	BINOP_LE    = TOKEN_LESS_EQUAL,
	BINOP_GT    = '>',
	BINOP_GE    = TOKEN_GREATER_EQUAL,
	BINOP_LOGICAL_EQ = TOKEN_EQUAL_EQUAL,
	BINOP_LOGICAL_NE = TOKEN_NOT_EQUAL,
	BINOP_AND   = '&',
	BINOP_OR    = '|',
	BINOP_XOR   = '^',
	BINOP_EQUAL = '=',
	BINOP_PLUS_EQ = TOKEN_PLUS_EQUAL,
	BINOP_MINUS_EQ = TOKEN_MINUS_EQUAL,
	BINOP_TIMES_EQ = TOKEN_TIMES_EQUAL,
	BINOP_DIV_EQ = TOKEN_DIV_EQUAL,
	BINOP_MOD_EQ = TOKEN_MOD_EQUAL,
	BINOP_XOR_EQ = TOKEN_XOR_EQUAL,
	BINOP_OR_EQ = TOKEN_OR_EQUAL,
	BINOP_AND_EQ = TOKEN_AND_EQUAL,
	BINOP_SHL_EQ = TOKEN_SHL_EQUAL,
	BINOP_SHR_EQ = TOKEN_SHR_EQUAL,
	BINOP_EQUAL_EQUAL = TOKEN_EQUAL_EQUAL,
	BINOP_NOT_EQUAL = TOKEN_NOT_EQUAL,
} Binary_Operator;

typedef enum {
	// Expressions
	AST_EXPRESSION_PRIMARY_IDENTIFIER,
	AST_EXPRESSION_PRIMARY_CONSTANT,
	AST_EXPRESSION_PRIMARY_STRING_LITERAL,
	AST_EXPRESSION_CONDITIONAL,
	AST_EXPRESSION_ASSIGNMENT,
	AST_EXPRESSION_ARGUMENT_LIST,
	AST_EXPRESSION_UNARY,
	AST_EXPRESSION_CAST,
	AST_EXPRESSION_MULTIPLICATIVE,
	AST_EXPRESSION_ADDITIVE,
	AST_EXPRESSION_SHIFT,
	AST_EXPRESSION_RELATIONAL,
	AST_EXPRESSION_EQUALITY,
	AST_EXPRESSION_AND,
	AST_EXPRESSION_EXCLUSIVE_OR,
	AST_EXPRESSION_INCLUSIVE_OR,
	AST_EXPRESSION_LOGICAL_AND,
	AST_EXPRESSION_LOGICAL_OR,
	AST_EXPRESSION_POSTFIX_UNARY,
	AST_EXPRESSION_POSTFIX_BINARY,
	AST_EXPRESSION_TERNARY,
	AST_EXPRESSION_SIZEOF,

	AST_CONSTANT_FLOATING_POINT,
	AST_CONSTANT_INTEGER,
	AST_CONSTANT_ENUMARATION,
	AST_CONSTANT_CHARACTER,

	// Operators
	//AST_OPERATOR_UNARY,
	//AST_OPERATOR_ADDITIVE,
	//AST_OPERATOR_MULTIPLICATIVE,
	//AST_OPERATOR_COMPARISON,
	//AST_OPERATOR_SHIFT,

	// Type
	AST_TYPE_NAME,
	AST_TYPE_INFO,
	AST_TYPE_POINTER,
	AST_TYPE_ABSTRACT_DECLARATOR,
	AST_TYPE_DIRECT_ABSTRACT_DECLARATOR,
	AST_TYPE_STRUCT_DECLARATOR,
	AST_TYPE_STRUCT_DECLARATOR_BITFIELD,
	AST_TYPE_STRUCT_DECLARATOR_LIST,

	AST_ENUMERATOR,
	AST_ENUMERATOR_LIST,

	// Params
	AST_PARAMETER_LIST,
	AST_PARAMETER_DECLARATION,
	AST_DIRECT_DECLARATOR,

	// Declaration
	AST_STRUCT_DECLARATION,
	AST_STRUCT_DECLARATION_LIST,
} Node_Kind;

typedef struct {
	struct Ast_t* qualifiers_specifiers;
	struct Ast_t* abstract_declarator;
} Ast_Type_Name;

typedef struct {
	struct Ast_t* qualifiers;
	struct Ast_t* next;
} Ast_Type_Pointer;

typedef struct {
	Binary_Operator bo;
	struct Ast_t* left;
	struct Ast_t* right;
} Ast_Expression_Binary;

typedef struct {
	Token* data;
} Ast_Expression_Primary;

typedef struct {
	struct Ast_t* type_name;
	struct Ast_t* expression;
} Ast_Expression_Cast;

typedef struct {
	Unary_Operator uo;
	struct Ast_t* expr;
} Ast_Expression_Unary;

typedef struct {
	Postfix_Operator po;
	struct Ast_t* expr;
} Ast_Expression_Postfix_Unary;

typedef struct {
	Postfix_Operator po;
	struct Ast_t* left;
	struct Ast_t* right;
} Ast_Expression_Postfix_Binary;

typedef struct {
	struct Ast_t* expr;
	struct Ast_t* next;
} Ast_Expression_Argument_List;

typedef struct {
	struct Ast_t* condition;
	struct Ast_t* case_true;
	struct Ast_t* case_false;
} Ast_Expression_Ternary;

typedef struct {
	struct Ast_t* pointer; // optional
	struct Ast_t* direct_abstract_decl;
} Ast_Abstract_Declarator;

typedef struct {
	Direct_Abstract_Decl_Type type;
	Token* name; // optional
	struct Ast_t* left_opt;
	struct Ast_t* right_opt;
} Ast_Direct_Abstract_Declarator;

typedef struct {
	bool is_vararg;
	struct Ast_t** param_decl;
} Ast_Parameter_List;

typedef struct {
	struct Ast_t* decl_specifiers;
	struct Ast_t* declarator;
} Ast_Parameter_Declaration;

typedef struct {
	bool is_type_name;
	union {
		struct Ast_t* type;
		struct Ast_t* expr;
	};
} Ast_Expression_Sizeof;

typedef struct {
	struct Ast_t* declarator;
} Ast_Struct_Declarator;

typedef struct {
	struct Ast_t* declarator;
	struct Ast_t* const_expr;
} Ast_Struct_Declarator_Bitfield;

typedef struct {
	struct Ast_t** list;
} Ast_Type_Struct_Declarator_List;

typedef struct {
	struct Ast_t* spec_qual;
	struct Ast_t* struct_decl_list;
} Ast_Struct_Declaration;

typedef struct {
	struct Ast_t** list;
} Ast_Struct_Declaration_List;

typedef struct {
	Token* enum_constant;
	struct Ast_t* const_expr;
} Ast_Enumerator;

typedef struct {
	struct Ast_Enumerator** list;
} Ast_Enumerator_List;

typedef struct Ast_t {
	Node_Kind kind;
	union {
		Ast_Expression_Binary expression_binary;
		Ast_Expression_Primary expression_primary;
		Ast_Expression_Cast expression_cast;
		Ast_Expression_Unary expression_unary;
		Ast_Expression_Postfix_Unary expression_postfix_unary;
		Ast_Expression_Postfix_Binary expression_postfix_binary;
		Ast_Expression_Argument_List expression_argument_list;
		Ast_Expression_Ternary expression_ternary;
		Ast_Expression_Sizeof expression_sizeof;
		Ast_Specifier_Qualifier specifier_qualifier;
		Ast_Type_Name type_name;
		Ast_Type_Pointer pointer;
		Ast_Abstract_Declarator abstract_type_decl;
		Ast_Direct_Abstract_Declarator direct_abstract_decl;
		Ast_Parameter_List parameter_list;
		Ast_Parameter_Declaration parameter_decl;
		Ast_Struct_Declarator struct_declarator;
		Ast_Type_Struct_Declarator_List struct_declarator_list;
		Ast_Struct_Declarator_Bitfield struct_declarator_bitfield;
		Ast_Struct_Declaration_List struct_declaration_list;
		Ast_Struct_Declaration struct_declaration;
		Ast_Enumerator enumerator;
		Ast_Enumerator_List enumerator_list;
	};
} Ast;

typedef enum {
	PARSER_STATUS_OK = 0,
	PARSER_STATUS_FATAL,
} Parser_Status;

typedef struct {
	struct Ast_t* node;
	Parser_Status status;
	const char*   error_message;
} Parser_Result;

Parser_Result parse_type_name(Lexer* lexer);
Parser_Result parse_postfix_expression(Lexer* lexer);
Parser_Result parse_argument_expression_list(Lexer* lexer);
Parser_Result parse_unary_expression(Lexer* lexer);
Parser_Result parse_cast_expression(Lexer* lexer);
Parser_Result parse_multiplicative_expression(Lexer* lexer);
Parser_Result parse_additive_expression(Lexer* lexer);
Parser_Result parse_shift_expression(Lexer* lexer);
Parser_Result parse_relational_expression(Lexer* lexer);
Parser_Result parse_equality_expression(Lexer* lexer);
Parser_Result parse_and_expression(Lexer* lexer);
Parser_Result parse_exclusive_or_expression(Lexer* lexer);
Parser_Result parse_inclusive_or_expression(Lexer* lexer);
Parser_Result parse_logical_and_expression(Lexer* lexer);
Parser_Result parse_logical_or_expression(Lexer* lexer);
Parser_Result parse_conditional_expression(Lexer* lexer);
Parser_Result parse_assignment_expression(Lexer* lexer);
Parser_Result parse_primary_expression(Lexer* lexer);
Parser_Result parse_expression(Lexer* lexer);
Parser_Result parse_constant(Lexer* lexer);
Parser_Result parse_identifier(Lexer* lexer);
Parser_Result parse_pointer(Lexer* lexer);
Parser_Result parse_abstract_declarator(Lexer* lexer, bool require_name);
Parser_Result parse_struct_declaration_list(Lexer* lexer);
Parser_Result parse_constant_expression(Lexer* lexer);

void parser_print_ast(FILE* out, Ast* ast);
#include "common.h"
#include "lexer.h"
#include <stdio.h>

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
} Binary_Operator;

typedef enum {
	// Expressions
	AST_EXPRESSION,
	AST_EXPRESSION_PRIMARY,
	AST_EXPRESSION_CONSTANT,
	AST_EXPRESSION_CONDITIONAL,
	AST_EXPRESSION_ASSIGNMENT,
	AST_EXPRESSION_POSTFIX,
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

	AST_CONSTANT,
	AST_CONSTANT_FLOATING_POINT,
	AST_CONSTANT_INTEGER,
	AST_CONSTANT_ENUMARATION,
	AST_CONSTANT_CHARACTER,

	// Operators
	AST_OPERATOR_UNARY,
	AST_OPERATOR_ADDITIVE,
	AST_OPERATOR_MULTIPLICATIVE,
	AST_OPERATOR_COMPARISON,
	AST_OPERATOR_SHIFT,

} Node_Kind;

typedef struct {
	Binary_Operator bo;
	struct Ast_t* left;
	struct Ast_t* right;
} Ast_Expression_Binary;

typedef struct {
	Token* data;
} Ast_Expression_Primary;

typedef struct Ast_t {
	Node_Kind kind;
	union {
		Ast_Expression_Binary expression_binary;
		Ast_Expression_Primary expression_primary;
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

void parser_print_ast(FILE* out, Ast* ast);
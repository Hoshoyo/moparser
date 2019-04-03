#define _CRT_SECURE_NO_WARNINGS
#include "parser.h"
#include "light_array.h"
#include <stdlib.h>
#include <assert.h>

static char parser_error_buffer[1024];

// unary-operator: one of
// & * + - ~ !

// assignment-operator: one of
// = *= /= %= += -= <<= >>= &= ^= |=

static bool
is_type_name(Token* t) {
	return false;
}

static bool
is_assignment_operator(Token* t) {
	return t->flags & TOKEN_FLAG_ASSIGNMENT_OPERATOR;
}

static void*
allocate_node() {
	return calloc(1, sizeof(Ast));
}

static void
free_node(Ast* node) {
	free(node);
}

static const char*
parser_error_message(Lexer* lexer, const char* fmt, ...) {
	return 0;
}

static Parser_Result 
require_token(Lexer* lexer, Token_Type tt) {
	Parser_Result result = { 0 };
	Token* n = lexer_next(lexer);
	if (n->type != tt) {
		result.status = PARSER_STATUS_FATAL;
		sprintf(parser_error_buffer, 
			"%s:%d:%d: Syntax error: Required '%s', but got '%s'\n", 
			lexer->filename, n->line, n->column, token_type_to_str(tt), token_to_str(n));
		result.error_message = parser_error_buffer;
	} else {
		result.status = PARSER_STATUS_OK;
	}

	return result;
}

Parser_Result 
parse_type_name(Lexer* lexer) {
	// TODO(psv): implement
	assert(0);
	return (Parser_Result) { 0 };
}

// postfix-expression:
// primary-expression
// postfix-expression [ expression ]
// postfix-expression ( argument-expression-list_opt )
// postfix-expression . identifier
// postfix-expression -> identifier
// postfix-expression ++
// postfix-expression --
Parser_Result 
parse_postfix_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	s32 start = lexer->index;
	res = parse_primary_expression(lexer);

	if (res.status == PARSER_STATUS_FATAL) {
		lexer->index = start;

		res = parse_postfix_expression(lexer);
		if (res.status == PARSER_STATUS_FATAL)
			return res;
		Token* next = lexer_peek(lexer);
		switch (next->type) {
			case '[': {
				lexer_next(lexer);
				res = parse_expression(lexer);
				if (res.status == PARSER_STATUS_FATAL)
					return res;
				res = require_token(lexer, ']');
				if (res.status == PARSER_STATUS_FATAL)
					return res;
			} break;
			case '(': {
				lexer_next(lexer);
				if (lexer_peek(lexer)->type != ')') {
					res = parse_argument_expression_list(lexer);
				}
				res = require_token(lexer, ')');
				if (res.status == PARSER_STATUS_FATAL)
					return res;
			} break;
			case '.': {
				lexer_next(lexer);
				res = require_token(lexer, TOKEN_IDENTIFIER);
				if (res.status == PARSER_STATUS_FATAL)
					return res;
			} break;
			case TOKEN_ARROW: {
				lexer_next(lexer);
				res = require_token(lexer, TOKEN_IDENTIFIER);
				if (res.status == PARSER_STATUS_FATAL)
					return res;
			} break;
			case TOKEN_PLUS_PLUS:
				lexer_next(lexer);
				break;
			case TOKEN_MINUS_MINUS:
				lexer_next(lexer);
				break;
			default:
				break;
		}
	}

	return res;
}

// argument-expression-list:
// assignment-expression
// argument-expression-list , assignment-expression
Parser_Result 
parse_argument_expression_list(Lexer* lexer) {
	// TODO(psv): implement argument expression list
	return parse_assignment_expression(lexer);
}

// unary-expression:
// postfix-expression
// ++ unary-expression
// -- unary-expression
// unary-operator
// cast-expression 
// sizeof unary-expression
// sizeof ( type-name )
Parser_Result 
parse_unary_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	Token* next = lexer_peek(lexer);

	switch(next->type) {
		case TOKEN_PLUS_PLUS:{
			lexer_next(lexer);
			Parser_Result expr = parse_unary_expression(lexer);
			if(expr.status == PARSER_STATUS_FATAL)
				return res;

			res.node = allocate_node();
			res.node->kind = AST_EXPRESSION_UNARY;
			res.node->expression_unary.expr = expr.node;
			res.node->expression_unary.uo = UNOP_PLUS_PLUS;
		}break;
		case TOKEN_MINUS_MINUS: {
			lexer_next(lexer);
			Parser_Result expr = parse_unary_expression(lexer);
			if(expr.status == PARSER_STATUS_FATAL)
				return res;

			res.node = allocate_node();
			res.node->kind = AST_EXPRESSION_UNARY;
			res.node->expression_unary.expr = expr.node;
			res.node->expression_unary.uo = UNOP_MINUS_MINUS;
		} break;
		// & * + - ~ !
		case '&': 
		case '*':
		case '+':
		case '-':
		case '~':
		case '!': {
			lexer_next(lexer);
			Parser_Result expr = parse_cast_expression(lexer);
			if(expr.status == PARSER_STATUS_FATAL)
				return expr;

			res.node = allocate_node();
			res.node->kind = AST_EXPRESSION_UNARY;
			res.node->expression_unary.expr = expr.node;
			res.node->expression_unary.uo = (Unary_Operator)next->type;
		} break;

		case TOKEN_KEYWORD_SIZEOF: {
			// TODO(psv):
		}break;
		default:
			res = parse_postfix_expression(lexer);
		break;
	}

	return res;
}

// cast-expression:
// unary-expression
// ( type-name ) cast-expression
Parser_Result 
parse_cast_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	Token* next = lexer_peek(lexer);
	if(next->type == '(' && is_type_name(lexer_peek_n(lexer, 1))) {
		lexer_next(lexer); // eat '('
		Parser_Result type_name = parse_type_name(lexer);
		if(type_name.status == PARSER_STATUS_FATAL)
			return res;

		Parser_Result expr = parse_cast_expression(lexer);
		if(expr.status == PARSER_STATUS_FATAL)
			return res;

		res.node = allocate_node();
		res.node->kind = AST_EXPRESSION_CAST;
		res.node->expression_cast.expression = expr.node;
		res.node->expression_cast.type_name = type_name.node;
	} else {
		res = parse_unary_expression(lexer);
	}

	return res;
}

// multiplicative-expression:
// cast-expression
// multiplicative-expression * cast-expression
// multiplicative-expression / cast-expression
// multiplicative-expression % cast-expression
Parser_Result
parse_multiplicative_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_cast_expression(lexer);

	if (res.status == PARSER_STATUS_OK) {
		while(true) {
			Token* op = lexer_peek(lexer);
			if (op->type == '*' || op->type == '/' || op->type == '%') {
				lexer_next(lexer);
				Parser_Result right = parse_cast_expression(lexer);

				// Construct the node
				Ast* node = allocate_node();
				node->kind = AST_EXPRESSION_MULTIPLICATIVE;
				node->expression_binary.bo = (Binary_Operator)op->type;
				node->expression_binary.left = res.node;
				node->expression_binary.right = right.node;
				res.node = node;
			} else {
				break;
			}
		}
	}

	return res;
}

// additive-expression:
// multiplicative-expression
// additive-expression + multiplicative-expression
// additive-expression - multiplicative-expression
Parser_Result 
parse_additive_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_multiplicative_expression(lexer);

	if (res.status == PARSER_STATUS_OK) {
		while(true) {
			Token* op = lexer_peek(lexer);
			if (op->type == '+' || op->type == '-') {
				lexer_next(lexer);
				Parser_Result right = parse_multiplicative_expression(lexer);

				// Construct the node
				Ast* node = allocate_node();
				node->kind = AST_EXPRESSION_ADDITIVE;
				node->expression_binary.bo = (Binary_Operator)op->type;
				node->expression_binary.left = res.node;
				node->expression_binary.right = right.node;
				res.node = node;
			} else {
				break;
			}
		}
	}

	return res;
}

// shift-expression:
// additive-expression
// shift-expression << additive-expression
// shift-expression >> additive-expression
Parser_Result 
parse_shift_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_additive_expression(lexer);

	if (res.status == PARSER_STATUS_OK) {
		while(true) {
			Token* op = lexer_peek(lexer);
			if (op->type == TOKEN_BITSHIFT_LEFT || op->type == TOKEN_BITSHIFT_RIGHT) {
				lexer_next(lexer);
				Parser_Result right = parse_additive_expression(lexer);

				// Construct the node
				Ast* node = allocate_node();
				node->kind = AST_EXPRESSION_SHIFT;
				node->expression_binary.bo = (Binary_Operator)op->type;
				node->expression_binary.left = res.node;
				node->expression_binary.right = right.node;
				res.node = node;
			} else {
				break;
			}
		}
	}

	return res;
}

// relational-expression:
// shift-expression
// relational-expression < shift-expression
// relational-expression > shift-expression
// relational-expression <= shift-expression
// relational-expression >= shift-expression
Parser_Result 
parse_relational_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_shift_expression(lexer);

	if (res.status == PARSER_STATUS_OK) {
		while(true) {
			Token* op = lexer_peek(lexer);
			if (op->type == '<' || op->type == '>' || op->type == TOKEN_LESS_EQUAL || op->type == TOKEN_GREATER_EQUAL) {
				lexer_next(lexer);
				Parser_Result right = parse_shift_expression(lexer);

				// Construct the node
				Ast* node = allocate_node();
				node->kind = AST_EXPRESSION_RELATIONAL;
				node->expression_binary.bo = (Binary_Operator)op->type;
				node->expression_binary.left = res.node;
				node->expression_binary.right = right.node;
				res.node = node;
			} else {
				break;
			}
		}
	}
	return res;
}

// equality-expression:
// relational-expression
// equality-expression == relational-expression
// equality-expression != relational-expression
Parser_Result 
parse_equality_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_relational_expression(lexer);

	if (res.status == PARSER_STATUS_OK) {
		while(true) {
			Token* op = lexer_peek(lexer);
			if (op->type == TOKEN_EQUAL_EQUAL || op->type == TOKEN_NOT_EQUAL) {
				lexer_next(lexer);
				Parser_Result right = parse_relational_expression(lexer);

				// Construct the node
				Ast* node = allocate_node();
				node->kind = AST_EXPRESSION_EQUALITY;
				node->expression_binary.bo = (Binary_Operator)op->type;
				node->expression_binary.left = res.node;
				node->expression_binary.right = right.node;
				res.node = node;
			} else {
				break;
			}
		}
	}

	return res;
}

// AND-expression:
// equality-expression
// AND-expression & equality-expression
Parser_Result 
parse_and_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_equality_expression(lexer);

	if (res.status == PARSER_STATUS_OK) {
		while(true) {
			Token* op = lexer_peek(lexer);
			if (op->type == '&') {
				lexer_next(lexer);
				Parser_Result right = parse_equality_expression(lexer);

				// Construct the node
				Ast* node = allocate_node();
				node->kind = AST_EXPRESSION_AND;
				node->expression_binary.bo = (Binary_Operator)op->type;
				node->expression_binary.left = res.node;
				node->expression_binary.right = right.node;
				res.node = node;
			} else {
				break;
			}
		}
	}

	return res;
}

// exclusive-OR-expression:
// AND-expression
// exclusive-OR-expression ^ AND-expression
Parser_Result 
parse_exclusive_or_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_and_expression(lexer);

	if (res.status == PARSER_STATUS_OK) {
		while(true) {
			Token* op = lexer_peek(lexer);
			if (op->type == '^') {
				lexer_next(lexer);
				Parser_Result right = parse_and_expression(lexer);

				// Construct the node
				Ast* node = allocate_node();
				node->kind = AST_EXPRESSION_EXCLUSIVE_OR;
				node->expression_binary.bo = (Binary_Operator)op->type;
				node->expression_binary.left = res.node;
				node->expression_binary.right = right.node;
				res.node = node;
			} else {
				break;
			}
		}
	}

	return res;
}

// inclusive-OR-expression:
// exclusive-OR-expression
// inclusive-OR-expression | exclusive-OR-expression
Parser_Result parse_inclusive_or_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_exclusive_or_expression(lexer);

	if (res.status == PARSER_STATUS_OK) {
		while(true) {
			Token* op = lexer_peek(lexer);
			if (op->type == '|') {
				lexer_next(lexer);
				Parser_Result right = parse_exclusive_or_expression(lexer);

				// Construct the node
				Ast* node = allocate_node();
				node->kind = AST_EXPRESSION_INCLUSIVE_OR;
				node->expression_binary.bo = (Binary_Operator)op->type;
				node->expression_binary.left = res.node;
				node->expression_binary.right = right.node;
				res.node = node;
			} else {
				break;
			}
		}
	}

	return res;
}

// logical-AND-expression:
// inclusive-OR-expression
// logical-AND-expression && inclusive-OR-expression
Parser_Result 
parse_logical_and_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_inclusive_or_expression(lexer);

	if (res.status == PARSER_STATUS_OK) {
		while(true) {
			Token* op = lexer_peek(lexer);
			if (op->type == TOKEN_LOGIC_AND) {
				lexer_next(lexer);
				Parser_Result right = parse_inclusive_or_expression(lexer);

				// Construct the node
				Ast* node = allocate_node();
				node->kind = AST_EXPRESSION_LOGICAL_AND;
				node->expression_binary.bo = (Binary_Operator)op->type;
				node->expression_binary.left = res.node;
				node->expression_binary.right = right.node;
				res.node = node;
			} else {
				break;
			}
		}
	}

	return res;
}

// logical-OR-expression:
// logical-AND-expression
// logical-OR-expression || logical-AND-expression
Parser_Result parse_logical_or_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_logical_and_expression(lexer);

	if (res.status == PARSER_STATUS_OK) {
		while(true) {
			Token* op = lexer_peek(lexer);
			if (op->type == TOKEN_LOGIC_AND) {
				lexer_next(lexer);
				Parser_Result right = parse_logical_and_expression(lexer);

				// Construct the node
				Ast* node = allocate_node();
				node->kind = AST_EXPRESSION_LOGICAL_OR;
				node->expression_binary.bo = (Binary_Operator)op->type;
				node->expression_binary.left = res.node;
				node->expression_binary.right = right.node;
				res.node = node;
			} else {
				break;
			}
		}
	}

	return res;
}

// conditional-expression:
// logical-OR-expression
// logical-OR-expression ? expression : conditional-expression
Parser_Result 
parse_conditional_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_logical_or_expression(lexer);

	if (res.status == PARSER_STATUS_FATAL)
		return res;

	// ternary operator
	Token* next = lexer_peek(lexer);
	if (next->type == '?') {
		lexer_next(lexer);
		res = parse_expression(lexer);
		if (res.status == PARSER_STATUS_FATAL)
			return res;
		res = require_token(lexer, ':');
		if (res.status == PARSER_STATUS_FATAL)
			return res;
		res = parse_conditional_expression(lexer);
		if (res.status == PARSER_STATUS_FATAL)
			return res;

		// TODO(psv): build node
	}

	return res;
}

// assignment-expression:
// conditional-expression
// unary-expression assignment-operator assignment-expression
Parser_Result 
parse_assignment_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	res = parse_conditional_expression(lexer);

	if (res.status == PARSER_STATUS_OK) {
		while(true) {
			Token* op = lexer_peek(lexer);
			if (is_assignment_operator(op)) {
				lexer_next(lexer);
				Parser_Result right = parse_conditional_expression(lexer);

				// Construct the node
				Ast* node = allocate_node();
				node->kind = AST_EXPRESSION_ASSIGNMENT;
				node->expression_binary.bo = (Binary_Operator)op->type;
				node->expression_binary.left = res.node;
				node->expression_binary.right = right.node;
				res.node = node;
			} else {
				break;
			}
		}
	}
	return res;
}

// primary-expression:
// identifier
// constant
// string-literal
// ( expression )
Parser_Result 
parse_primary_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	Token* next = lexer_peek(lexer);

	switch (next->type) {
		case TOKEN_IDENTIFIER: {
			lexer_next(lexer);
			res.node = allocate_node();
			res.node->kind = AST_EXPRESSION_PRIMARY_IDENTIFIER;
			res.node->expression_primary.data = next;
		}break;
		case TOKEN_STRING_LITERAL: {
			lexer_next(lexer);
			res.node = allocate_node();
			res.node->kind = AST_EXPRESSION_PRIMARY_STRING_LITERAL;
			res.node->expression_primary.data = next;
		}break;
		case '(': {
			lexer_next(lexer);
			res = parse_expression(lexer);
			if (res.status == PARSER_STATUS_FATAL)
				return res;
			Parser_Result endexpr = require_token(lexer, ')');
			if(endexpr.status == PARSER_STATUS_FATAL) {
				return endexpr;
			}
		}break;
		default: {
			res = parse_constant(lexer);
		}break;
	}

	return res;
}

// expression:
// assignment-expression
// expression , assignment-expression
Parser_Result 
parse_expression(Lexer* lexer) {
	Parser_Result res = { 0 };

	s32 start = lexer->index;
	res = parse_assignment_expression(lexer);

	if (res.status == PARSER_STATUS_FATAL) {
		lexer->index = start;
		res = parse_expression(lexer);
		if (res.status == PARSER_STATUS_FATAL)
			return res;
		res = require_token(lexer, ',');
		if (res.status == PARSER_STATUS_FATAL)
			return res;
		res = parse_assignment_expression(lexer);

		// build node
	}

	return res;
}

// constant:
// floating-point-constant
// integer-constant
// enumeration-constant (identifier)
// character-constant
Parser_Result
parse_constant(Lexer* lexer) {
	Parser_Result result = { 0 };

	Ast* node = allocate_node();
	result.node = node;

	Token* n = lexer_next(lexer);
	switch (n->type) {
		case TOKEN_FLOAT_LITERAL: {
			node->kind = AST_CONSTANT_FLOATING_POINT;
			node->expression_primary.data = n;
		} break;
		case TOKEN_INT_HEX_LITERAL:
		case TOKEN_INT_BIN_LITERAL:
		case TOKEN_INT_OCT_LITERAL:
		case TOKEN_INT_U_LITERAL:
		case TOKEN_INT_UL_LITERAL:
		case TOKEN_INT_ULL_LITERAL:
		case TOKEN_INT_LITERAL:
		case TOKEN_INT_L_LITERAL:
		case TOKEN_INT_LL_LITERAL: {
			node->kind = AST_CONSTANT_INTEGER;
			node->expression_primary.data = n;
		} break;
		case TOKEN_IDENTIFIER: {
			// enumeration-constant
			node->kind = AST_CONSTANT_ENUMARATION;
			node->expression_primary.data = n;
		} break;
		case TOKEN_CHAR_LITERAL: {
			// character-constant
			node->kind = AST_CONSTANT_CHARACTER;
			node->expression_primary.data = n;
		}break;
		default: {
			result.status = PARSER_STATUS_FATAL;
			result.error_message = parser_error_message(lexer, "Syntax Error: expected constant, but got '%s'\n", token_to_str(n));
			result.node = 0;
			free_node(node);
		}break;
	}

	return result;
}











void
parser_print_typename(FILE* out, Ast* typename) {
	fprintf(out, "<typename>");
}

void
parser_print_ast(FILE* out, Ast* ast) {
	switch (ast->kind) {

		case AST_EXPRESSION_PRIMARY_CONSTANT:
		case AST_EXPRESSION_PRIMARY_STRING_LITERAL:
		case AST_EXPRESSION_PRIMARY_IDENTIFIER: {
			fprintf(out, "%.*s", ast->expression_primary.data->length, ast->expression_primary.data->data);
			fflush(out);
		}break;
		case AST_EXPRESSION_CONDITIONAL:
		case AST_EXPRESSION_ASSIGNMENT:
			break;
		case AST_EXPRESSION_POSTFIX:
			fprintf(out, "<postfix>");
			break;
		case AST_EXPRESSION_ARGUMENT_LIST:
			break;
		case AST_EXPRESSION_UNARY:{
			fprintf(out, "(%c", ast->expression_unary.uo);
			parser_print_ast(out, ast->expression_unary.expr);
			fprintf(out, ")");
		}break;
		case AST_EXPRESSION_CAST:
			fprintf(out, "(");
			parser_print_typename(out, ast->expression_cast.type_name);
			fprintf(out, ")");
			parser_print_ast(out, ast->expression_cast.expression);
			break;
		case AST_EXPRESSION_ADDITIVE: 
		case AST_EXPRESSION_MULTIPLICATIVE: {
			fprintf(out, "(");
			fflush(out);
			parser_print_ast(out, ast->expression_binary.left);
			fflush(out);
			fprintf(out, " %c ", ast->expression_binary.bo);
			fflush(out);
			parser_print_ast(out, ast->expression_binary.right);
			fflush(out);
			fprintf(out, ")");
			fflush(out);
		}break;
		case AST_EXPRESSION_SHIFT:
		case AST_EXPRESSION_RELATIONAL:
		case AST_EXPRESSION_EQUALITY:
		case AST_EXPRESSION_AND:
		case AST_EXPRESSION_EXCLUSIVE_OR:
		case AST_EXPRESSION_INCLUSIVE_OR:
		case AST_EXPRESSION_LOGICAL_AND:
		case AST_EXPRESSION_LOGICAL_OR:

		case AST_CONSTANT_FLOATING_POINT:
		case AST_CONSTANT_INTEGER: {
			fprintf(out, "%.*s", ast->expression_primary.data->length, ast->expression_primary.data->data);
			fflush(out);
		}break;
		case AST_CONSTANT_ENUMARATION:
		case AST_CONSTANT_CHARACTER:

		// Operators
		case AST_OPERATOR_UNARY:
		case AST_OPERATOR_ADDITIVE:
		case AST_OPERATOR_MULTIPLICATIVE:
		case AST_OPERATOR_COMPARISON:
		case AST_OPERATOR_SHIFT:
			break;
		default: {
			printf("<unknown>");
			fflush(out);
		}break;
	}
}
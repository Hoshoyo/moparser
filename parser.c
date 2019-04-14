#define _CRT_SECURE_NO_WARNINGS
#include "parser.h"
#include "light_array.h"
#include <stdlib.h>
#include <assert.h>

// Declarations
// https://docs.microsoft.com/en-us/cpp/c-language/summary-of-declarations?view=vs-2017

// https://docs.microsoft.com/en-us/cpp/c-language/c-floating-point-constants?view=vs-2017

static char parser_error_buffer[1024];

// unary-operator: one of
// & * + - ~ !

// assignment-operator: one of
// = *= /= %= += -= <<= >>= &= ^= |=

static bool
is_type_name(Token* t) {
	// TODO(psv): implement
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

// struct-or-union-specifier:
//     struct-or-union identifier_opt { struct-declaration-list }
//     struct-or-union identifier

Type_Primitive
primitive_from_token(Token* t) {
	switch (t->type) {
		case TOKEN_KEYWORD_UNSIGNED:
			return TYPE_PRIMITIVE_UNSIGNED;
		case TOKEN_KEYWORD_SIGNED:
			return TYPE_PRIMITIVE_SIGNED;
		case TOKEN_KEYWORD_DOUBLE:
			return TYPE_PRIMITIVE_DOUBLE;
		case TOKEN_KEYWORD_FLOAT:
			return TYPE_PRIMITIVE_FLOAT;
		case TOKEN_KEYWORD_LONG:
			return TYPE_PRIMITIVE_LONG;
		case TOKEN_KEYWORD_INT:
			return TYPE_PRIMITIVE_INT;
		case TOKEN_KEYWORD_SHORT:
			return TYPE_PRIMITIVE_SHORT;
		case TOKEN_KEYWORD_CHAR: 
			return TYPE_PRIMITIVE_CHAR;
		default:
			assert(0); // Invalid code path
			break;
	}
	return -1;
}

static Ast* 
parser_type_primitive_get_info(Type_Primitive p) {
	Ast* node = allocate_node();

	node = allocate_node();
	node->kind = AST_TYPE_INFO;
	node->specifier_qualifier.primitive[p] = 1;
	node->specifier_qualifier.kind = p;

	return node;
}

// enumerator:
//     enumeration-constant
//     enumeration-constant = constant-expression
Parser_Result
parse_enumerator(Lexer* lexer) {
	Parser_Result res = {0};

	Token* enum_const = lexer_next(lexer);
	if(enum_const->type != TOKEN_IDENTIFIER) {
		res.status = PARSER_STATUS_FATAL;
		// TODO(psv): raise error
		return res;
	}
	
	Parser_Result const_expr = {0};
	if(lexer_peek(lexer)->type == '=') {
		lexer_next(lexer);
		const_expr = parse_constant_expression(lexer);
	}

	res.node = allocate_node();
	res.node->kind = AST_ENUMERATOR;
	res.node->enumerator.const_expr = const_expr.node;
	res.node->enumerator.enum_constant = enum_const;

	return res;
}

// enumerator-list:
//     enumerator
//     enumerator-list , enumerator
Parser_Result
parse_enumerator_list(Lexer* lexer) {
	Parser_Result res = {0};

	Parser_Result enumerator = parse_enumerator(lexer);
	if(enumerator.status == PARSER_STATUS_FATAL)
		return res;
	Ast_Enumerator* node = (Ast_Enumerator*)enumerator.node;

	Ast_Enumerator** list = array_new(Ast*);
	array_push(list, node);
	
	while(lexer_peek(lexer)->type == ',') {
		lexer_next(lexer); // eat ,
		Parser_Result e = parse_enumerator(lexer);
		if(e.status == PARSER_STATUS_FATAL)
			break;
		Ast_Enumerator* en = (Ast_Enumerator*)e.node;
		array_push(list, en);
	}

	res.node = allocate_node();
	res.node->kind = AST_ENUMERATOR_LIST;
	res.node->enumerator_list.list = (struct Ast_Enumerator**)list;

	return res;
}

// type-specifier:
//     void
//     char
//     short
//     int
//     long
//     float
//     double
//     signed
//     unsigned
//     struct-or-union-specifier
//     enum-specifier
//     typedef-name
Parser_Result
parse_type_specifier(Lexer* lexer, Ast* type) {
	Parser_Result res = {0};
	Token* s = lexer_peek(lexer);
	Ast* node = 0;

	assert((type) ? type->kind == AST_TYPE_INFO : true);

	switch(s->type) {
		case TOKEN_KEYWORD_VOID:
			lexer_next(lexer);
			node = allocate_node();
			node->kind = AST_TYPE_INFO;
			node->specifier_qualifier.kind = TYPE_VOID;
			break;

		case TOKEN_KEYWORD_UNSIGNED:
		case TOKEN_KEYWORD_SIGNED:
		case TOKEN_KEYWORD_DOUBLE:
		case TOKEN_KEYWORD_FLOAT:
		case TOKEN_KEYWORD_LONG:
		case TOKEN_KEYWORD_INT:
		case TOKEN_KEYWORD_SHORT:
		case TOKEN_KEYWORD_CHAR: {
			lexer_next(lexer);
			Type_Primitive primitive = primitive_from_token(s);
			assert(primitive != -1);
			if (type) {
				node = type;
				node->specifier_qualifier.primitive[primitive]++;
			} else {
				node = allocate_node();
				node->kind = AST_TYPE_INFO;
				node->specifier_qualifier.primitive[primitive] = 1;
			}
			node->specifier_qualifier.kind = TYPE_PRIMITIVE;
		} break;

		case TOKEN_KEYWORD_UNION:
		case TOKEN_KEYWORD_STRUCT: {
			Token* s_or_u = lexer_next(lexer);
			if(type) {
				node = type;
			} else {
				node = allocate_node();
				node->kind = AST_TYPE_INFO;
			}
			if(node->specifier_qualifier.kind != TYPE_NONE){
				// TODO(psv): raise error, type specifier together with struct specifier
				// (gcc): two or more data types in declaration specifiers
				assert(0);
			}
			// struct-or-union-specifier:
			// 		struct-or-union identifier_opt { struct-declaration-list }
			// 		struct-or-union identifier
			Token* id = lexer_peek(lexer);
			if(id->type == TOKEN_IDENTIFIER){
				lexer_next(lexer); // eat identifier
			} else {
				id = 0;
			}
			if(lexer_peek(lexer)->type == '{') {
				lexer_next(lexer);

				// struct-declaration-list
				Parser_Result decl_list = parse_struct_declaration_list(lexer);
				if(decl_list.status == PARSER_STATUS_FATAL)
					return decl_list;

				Parser_Result r = require_token(lexer, '}');
				if(r.status == PARSER_STATUS_FATAL)
					return r;

				node->specifier_qualifier.struct_desc = decl_list.node;
				node->specifier_qualifier.struct_name = id;
			}
			if(s_or_u->type == TOKEN_KEYWORD_STRUCT) {
				node->specifier_qualifier.kind = TYPE_STRUCT;
			} else if(s_or_u->type == TOKEN_KEYWORD_UNION) {
				node->specifier_qualifier.kind = TYPE_UNION;
			} else {
				assert(0);
				// TODO(psv): invalid code path
			}
		} break;
		case TOKEN_KEYWORD_ENUM: {
			// enum
			lexer_next(lexer); // eat enum
			Token* id = lexer_peek(lexer);
			if(id->type == TOKEN_IDENTIFIER) {
				lexer_next(lexer);
			} else {
				id = 0;
			}
			
			Parser_Result enum_list = {0};
			if(lexer_peek(lexer)->type == '{') {
				lexer_next(lexer);
				enum_list = parse_enumerator_list(lexer);
				if(enum_list.status == PARSER_STATUS_FATAL)
					return enum_list;
				Parser_Result r = require_token(lexer, '}');
				if(r.status == PARSER_STATUS_FATAL)
					return r;
			}

			node = allocate_node();
			node->kind = AST_TYPE_INFO;
			node->specifier_qualifier.kind = TYPE_ENUM;
			node->specifier_qualifier.enumerator_list = enum_list.node;
			node->specifier_qualifier.enum_name = id;
		} break;
		case TOKEN_IDENTIFIER:{
		    // typedef-name
			if(is_type_name(s)) {
				lexer_next(lexer);
				node = allocate_node();
				node->kind = AST_TYPE_INFO;
				node->specifier_qualifier.kind = TYPE_ALIAS;
				node->specifier_qualifier.alias = s;
			} else {
				res.status = PARSER_STATUS_FATAL;
				// TODO(psv): raise error
				break;
			}
		} break;
		default:
			// TODO(psv): error message
			res.status = PARSER_STATUS_FATAL;
			break;
	}

	res.node = node;

	return res;
}

// type-qualifier:
//     const
//     volatile
Parser_Result
parse_type_qualifier(Lexer* lexer, Ast* type) {
	Parser_Result res = {0};
	Token* q = lexer_peek(lexer);

	switch(q->type) {
		case TOKEN_KEYWORD_CONST: {
			lexer_next(lexer);
			if(type) {
				type->specifier_qualifier.qualifiers |= TYPE_QUALIFIER_CONST;
				res.node = type;
			} else {
				Ast* node = allocate_node();
				node->kind = AST_TYPE_INFO;
				node->specifier_qualifier.kind = TYPE_NONE;
				node->specifier_qualifier.qualifiers = TYPE_QUALIFIER_CONST;
				res.node = node;
			}
		} break;
		case TOKEN_KEYWORD_VOLATILE: {
			lexer_next(lexer);
			if(type) {
				type->specifier_qualifier.qualifiers |= TYPE_QUALIFIER_VOLATILE;
				res.node = type;
			} else {
				Ast* node = allocate_node();
				node->kind = AST_TYPE_INFO;
				node->specifier_qualifier.kind = TYPE_NONE;
				node->specifier_qualifier.qualifiers = TYPE_QUALIFIER_VOLATILE;
				res.node = node;
			}
		} break;
		default:{
			// TODO(psv): error message
			res.status = PARSER_STATUS_FATAL;
		}break;
	}
	return res;
}

// specifier-qualifier-list:
// 	type-specifier specifier-qualifier-list_opt
// 	type-qualifier specifier-qualifier-list_opt
Parser_Result 
parse_specifier_qualifier_list(Lexer* lexer) {
	Parser_Result res = parse_type_qualifier(lexer, 0);
	if(res.status == PARSER_STATUS_FATAL) {
		// try specifier
		res = parse_type_specifier(lexer, 0);
	}

	if(res.status == PARSER_STATUS_FATAL) {
		// TODO(psv): Error, expecting type qualifier or specifier but got something else
		return res;
	}

	while(true) {
		Parser_Result next = parse_type_qualifier(lexer, res.node);
		if(next.status == PARSER_STATUS_FATAL) {
			// try specifier
			next = parse_type_specifier(lexer, res.node);
		}

		// no more type qualifiers or specifiers
		if(next.status == PARSER_STATUS_FATAL)
			break;

		res = next;
	}

	return res;
}

// type-qualifier-list:
//     type-qualifier
//     type-qualifier-list type-qualifier
Parser_Result
parse_type_qualifier_list(Lexer* lexer) {
	Parser_Result res = {0};

	res = parse_type_qualifier(lexer, 0);
	while(true) {
		Parser_Result next = parse_type_qualifier(lexer, res.node);
		if(next.status == PARSER_STATUS_FATAL)
			break;
		res.node = next.node;
	}

	return res;
}

Parser_Result
parse_constant_expression(Lexer* lexer) {
	return parse_conditional_expression(lexer);
}

// struct-declarator:
//     declarator
//     declarator_opt : constant-expression
Parser_Result
parse_struct_declarator(Lexer* lexer) {
	Parser_Result res = {0};

	Parser_Result decl = {0};
	Parser_Result const_expr = {0};

	bool is_bitfield = false;

	if(lexer_peek(lexer)->type != ':') {
		decl = parse_abstract_declarator(lexer, true);
		if(decl.status == PARSER_STATUS_FATAL)
			return decl;
	}

	if(lexer_peek(lexer)->type == ':') {
		is_bitfield = true;
		lexer_next(lexer);
		const_expr = parse_constant_expression(lexer);
		if(const_expr.status == PARSER_STATUS_FATAL)
			return const_expr;
	}
	
	res.node = allocate_node();
	if(is_bitfield) {
		res.node->kind = AST_TYPE_STRUCT_DECLARATOR_BITFIELD;
		res.node->struct_declarator_bitfield.const_expr = const_expr.node;
		res.node->struct_declarator_bitfield.declarator = decl.node;
	} else {
		res.node->kind = AST_TYPE_STRUCT_DECLARATOR;
		res.node->struct_declarator.declarator = decl.node;
	}

	return res;
}


// struct-declarator-list:
//     struct-declarator 
//     struct-declarator-list , struct-declarator
Parser_Result
parse_struct_declarator_list(Lexer* lexer) {
	Parser_Result res = {0};

	Ast** list = 0;

	while(true) {
		Parser_Result r = parse_struct_declarator(lexer);
		if(r.status == PARSER_STATUS_FATAL)
			return r;

		if(!list) list = array_new(Ast*);
		array_push(list, r.node);

		if(lexer_peek(lexer)->type != ',') break;
	}

	res.node = allocate_node();
	res.node->kind = AST_TYPE_STRUCT_DECLARATOR_LIST;
	res.node->struct_declarator_list.list = list;

	return res;
}

Parser_Result
parse_struct_declaration(Lexer* lexer) {
	Parser_Result spec_qual = parse_specifier_qualifier_list(lexer);
	if(spec_qual.status == PARSER_STATUS_FATAL)
		return spec_qual;
	Parser_Result struct_decl_list = parse_struct_declarator_list(lexer);
	if(struct_decl_list.status == PARSER_STATUS_FATAL)
		return struct_decl_list;
	Parser_Result n = require_token(lexer, ';');
	if(n.status == PARSER_STATUS_FATAL)
		return n;
	
	Parser_Result res = {0};
	res.node = allocate_node();
	res.node->kind = AST_STRUCT_DECLARATION;
	res.node->struct_declaration.spec_qual = spec_qual.node;
	res.node->struct_declaration.struct_decl_list = struct_decl_list.node;

	return res;
}

// struct-declaration-list:
//     struct-declaration
//     struct-declaration-list struct-declaration
//
// struct-declaration:
//     specifier-qualifier-list struct-declarator-list ;
Parser_Result
parse_struct_declaration_list(Lexer* lexer) {

	Parser_Result r = parse_struct_declaration(lexer);
	if(r.status == PARSER_STATUS_FATAL)
		return r;

	Ast** list = array_new(Ast*);
	array_push(list, r.node);

	while(true) {
		Parser_Result r = parse_struct_declaration(lexer);
		if(r.status == PARSER_STATUS_FATAL) break;
		array_push(list, r.node);
	}
	
	Parser_Result res = {0};
	res.node = allocate_node();
	res.node->kind = AST_STRUCT_DECLARATION_LIST;
	res.node->struct_declaration_list.list = list;

	return res;
}

// storage-class-specifier:
//     auto
//     register
//     static
//     extern
//     typedef
//     __declspec ( extended-decl-modifier-seq ) /* Microsoft Specific */
Storage_Class
parse_storage_class_specifier(Lexer* lexer) {
	Storage_Class res = STORAGE_CLASS_NONE;
	Token* next = lexer_peek(lexer);
	
	switch(next->type) {
		case TOKEN_KEYWORD_AUTO:
			res = STORAGE_CLASS_AUTO; break;
		case TOKEN_KEYWORD_STATIC:
			res = STORAGE_CLASS_STATIC; break;
		case TOKEN_KEYWORD_REGISTER:
			res = STORAGE_CLASS_REGISTER; break;
		case TOKEN_KEYWORD_EXTERN:
			res = STORAGE_CLASS_EXTERN; break;
		case TOKEN_KEYWORD_TYPEDEF:
			res = STORAGE_CLASS_TYPEDEF; break;
		default: return res;
	}
	lexer_next(lexer);
	return res;
}

// declaration-specifiers:
//     storage-class-specifier declaration-specifiers_opt
//     type-specifier declaration-specifiers_opt
//     type-qualifier declaration-specifiers_opt
Parser_Result
parse_declaration_specifiers(Lexer* lexer) {
	Parser_Result res = {0};

	Ast* type = 0;
	Storage_Class sc = 0;

	while(true) {
		Storage_Class s = parse_storage_class_specifier(lexer);
		sc |= s;
		if(s == STORAGE_CLASS_NONE) {
			Parser_Result type_spec = parse_type_specifier(lexer, type);
			if(type_spec.status != PARSER_STATUS_FATAL) {
				// it was a type specifier
				type = type_spec.node;
			} else {
				Parser_Result type_qual = parse_type_qualifier(lexer, type);
				if(type_qual.status != PARSER_STATUS_FATAL) {
					// it was a type qualifier
					type = type_qual.node;
				} else {
					break;
				}
			}
		}
	}

	if(!type){
		type = parser_type_primitive_get_info(TYPE_PRIMITIVE_INT);
	} else if(type->specifier_qualifier.kind == TYPE_NONE) {
		type->specifier_qualifier.kind = TYPE_PRIMITIVE;
		type->specifier_qualifier.primitive[TYPE_PRIMITIVE_INT] = 1;
	}
	type->specifier_qualifier.storage_class = sc;

	res.node = type;

	return res;
}

// direct-declarator:
//     identifier
//     ( declarator )
//     direct-declarator [ constant-expression_opt ]
//     direct-declarator ( parameter-type-list ) /* New-style declarator */
//     direct-declarator ( identifier-list_opt ) /* Obsolete-style declarator */

// declarator:
//     pointer_opt direct-declarator

// parameter-declaration:
//     declaration-specifiers declarator /* Named declarator */
//     declaration-specifiers abstract-declarator_opt /* Anonymous declarator */
Parser_Result
parse_parameter_declaration(Lexer* lexer, bool require_name) {
	Parser_Result res = {0};

	Parser_Result decl_spec = parse_declaration_specifiers(lexer);
	if(decl_spec.status == PARSER_STATUS_FATAL)
		return decl_spec;

	Parser_Result declarator = parse_abstract_declarator(lexer, require_name);
	if(res.status == PARSER_STATUS_FATAL)
		return res;

	res.node = allocate_node();
	res.node->kind = AST_PARAMETER_DECLARATION;
	res.node->parameter_decl.decl_specifiers = decl_spec.node;
	res.node->parameter_decl.declarator = declarator.node;

	return res;
}

// parameter-list:
//     parameter-declaration
//     parameter-list , parameter-declaration
Parser_Result
parse_parameter_list(Lexer* lexer, bool require_name) {
	Parser_Result list = { 0 };

	while (true) {
		if(lexer_peek(lexer)->type == '.')
			break;

		Parser_Result res = parse_parameter_declaration(lexer, require_name);
		if (res.status == PARSER_STATUS_FATAL)
			break;

		if (!list.node) {
			list.node = allocate_node();
			list.node->kind = AST_PARAMETER_LIST;
			list.node->parameter_list.param_decl = array_new(struct Ast_t*);
			list.node->parameter_list.is_vararg = false;
		}
		array_push(list.node->parameter_list.param_decl, res.node);

		Token* next = lexer_peek(lexer);
		if (next->type != ',') {
			break;
		}
		lexer_next(lexer); // eat ','
	}

	return list;
}

// parameter-type-list:            /* The parameter list */
//     parameter-list
//     parameter-list , ...
// 
Parser_Result
parse_parameter_type_list(Lexer* lexer, bool require_name) {
	Parser_Result res = {0};

	res = parse_parameter_list(lexer, require_name);
	if (res.status == PARSER_STATUS_FATAL) {
		return res;
	}

	if (lexer_peek(lexer)->type == '.') {
		// Require three '.'
		lexer_next(lexer);
		Parser_Result status = require_token(lexer, '.');
		if (status.status == PARSER_STATUS_FATAL)
			return status;
		status = require_token(lexer, '.');
		if (status.status == PARSER_STATUS_FATAL)
			return status;

		if (res.node) {
			res.node->parameter_list.is_vararg = true;
		} else {
			res.node = allocate_node();
			res.node->parameter_list.is_vararg = true;
		}
	}

	return res;
}

// direct-abstract-declarator:
//     ( abstract-declarator )
//     direct-abstract-declarator_opt [ constant-expression_opt ]
//     direct-abstract-declarator_opt ( parameter-type-list_opt )
Parser_Result
parse_direct_abstract_declarator(Lexer* lexer, bool require_name) {
	Parser_Result res = {0};
	Ast* node = 0;

	while (true) {
		Token* name = 0;
		Token* next = lexer_peek(lexer);
		if (next->type == TOKEN_IDENTIFIER) {
			name = lexer_next(lexer);
		} else {
			if(require_name) {
				// TODO(psv): raise error here, name required
				res.status = PARSER_STATUS_FATAL;
				return res;
			}
		}
		if (next->type == '[') {
			// direct-abstract-declarator_opt is empty
			lexer_next(lexer);
			Parser_Result const_expr = parse_constant_expression(lexer);
			Parser_Result cbracket = require_token(lexer, ']');
			if (cbracket.status == PARSER_STATUS_FATAL) {
				// TODO(psv): raise error
				return cbracket;
			}
			Ast* new_node = allocate_node();
			new_node->kind = AST_TYPE_DIRECT_ABSTRACT_DECLARATOR;
			new_node->direct_abstract_decl.type = DIRECT_ABSTRACT_DECL_ARRAY;
			new_node->direct_abstract_decl.right_opt = const_expr.node;
			new_node->direct_abstract_decl.name = name;

			if (!node) {
				node = new_node;
			} else {
				new_node->direct_abstract_decl.left_opt = node;
				node = new_node;
			}
		} else if (next->type == '(') {
			lexer_next(lexer);
			// could be a parameter-list_opt or another abstract-declarator
			next = lexer_peek(lexer);
			if (next->type == '*' || next->type == '(' || next->type == '[') {
				// it is another abstract-declarator
				Parser_Result abst_decl = parse_abstract_declarator(lexer, false);
				Parser_Result r = require_token(lexer, ')');
				if (r.status == PARSER_STATUS_FATAL) {
					// TODO(psv): raise error
					return r;
				}

				Ast* new_node = allocate_node();
				new_node->kind = AST_TYPE_DIRECT_ABSTRACT_DECLARATOR;
				new_node->direct_abstract_decl.left_opt = abst_decl.node;
				new_node->direct_abstract_decl.right_opt = 0;
				new_node->direct_abstract_decl.type = DIRECT_ABSTRACT_DECL_NONE;
				new_node->direct_abstract_decl.name = name;

				if (!node) {
					node = new_node;
				} else {
					node->direct_abstract_decl.left_opt = new_node;
					node = new_node;
				}
			} else {
				// it is a parameter-type-list_opt
				Parser_Result params = parse_parameter_type_list(lexer, false);
				Parser_Result r = require_token(lexer, ')'); // end of parameter list
				if (r.status == PARSER_STATUS_FATAL) {
					// TODO(psv): raise error
					return r;
				}

				Ast* new_node = allocate_node();
				new_node->kind = AST_TYPE_DIRECT_ABSTRACT_DECLARATOR;
				new_node->direct_abstract_decl.type = DIRECT_ABSTRACT_DECL_FUNCTION;
				new_node->direct_abstract_decl.right_opt = params.node;
				new_node->direct_abstract_decl.name = name;

				if (!node) {
					node = new_node;
				} else {
					new_node->direct_abstract_decl.left_opt = node;
					node = new_node;
				}
			}
		} else {
			if(name) {
				node = allocate_node();
				node->kind = AST_TYPE_DIRECT_ABSTRACT_DECLARATOR;
				node->direct_abstract_decl.name = name;
				node->direct_abstract_decl.type = DIRECT_ABSTRACT_DECL_NAME;
			}
			break;
		}
	}

	res.node = node;

	return res;
}

// pointer:
//     * type-qualifier-list_opt
//     * type-qualifier-list_opt pointer
Parser_Result
parse_pointer(Lexer* lexer) {
	Parser_Result res = require_token(lexer, '*');
	if(res.status == PARSER_STATUS_FATAL)
		return res;

	Parser_Result type_qual_list = parse_type_qualifier_list(lexer);

	Ast* node = allocate_node();
	node->kind = AST_TYPE_POINTER;
	node->pointer.qualifiers = type_qual_list.node;

	if(lexer_peek(lexer)->type == '*') {
		Parser_Result ptr = parse_pointer(lexer);
		if(ptr.status == PARSER_STATUS_FATAL)
			return ptr;
		node->pointer.next = ptr.node;
	}

	res.node = node;

	return res;
}

// abstract-declarator: /* Used with anonymous declarators */
//    pointer
//    pointer_opt direct-abstract-declarator
Parser_Result
parse_abstract_declarator(Lexer* lexer, bool require_name) {
	Parser_Result res = {0};

	if(lexer_peek(lexer)->type == '*') {
		res = parse_pointer(lexer);
	}

	// direct-abstract-declarator
	Parser_Result dabstd = parse_direct_abstract_declarator(lexer, require_name);
	if (dabstd.status == PARSER_STATUS_FATAL)
		return dabstd;

	Ast* node = allocate_node();
	node->kind = AST_TYPE_ABSTRACT_DECLARATOR;
	node->abstract_type_decl.pointer = res.node;
	node->abstract_type_decl.direct_abstract_decl = dabstd.node;

	res.node = node;

	return res;
}

// type-name:
//    specifier-qualifier-list abstract-declarator_opt
Parser_Result 
parse_type_name(Lexer* lexer) {
	Parser_Result spec_qual = parse_specifier_qualifier_list(lexer);
	if(spec_qual.status == PARSER_STATUS_FATAL)
		return spec_qual;
	
	Parser_Result abst_decl = parse_abstract_declarator(lexer, false);
	if(abst_decl.status == PARSER_STATUS_FATAL)
		return abst_decl;

	Parser_Result res = {0};
	res.node = allocate_node();
	res.node->kind = AST_TYPE_NAME;
	res.node->type_name.qualifiers_specifiers = spec_qual.node;
	res.node->type_name.abstract_declarator = abst_decl.node;

	return res;
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

	res = parse_primary_expression(lexer);

	bool finding = true;
	while (finding) {
		Token* next = lexer_peek(lexer);
		Ast* left = res.node;

		switch (next->type) {
		case '[': {
			lexer_next(lexer);
			res = parse_expression(lexer);
			Ast* right = res.node;
			if (res.status == PARSER_STATUS_FATAL)
				return res;
			res = require_token(lexer, ']');
			if (res.status == PARSER_STATUS_FATAL)
				return res;
			Ast* node = allocate_node();
			node->kind = AST_EXPRESSION_POSTFIX_BINARY;
			node->expression_postfix_binary.left = left;
			node->expression_postfix_binary.right = right;
			node->expression_postfix_binary.po = POSTFIX_ARRAY_ACCESS;
			res.node = node;
		} break;
		case '(': {
			lexer_next(lexer);
			Ast* right = 0;
			if (lexer_peek(lexer)->type != ')') {
				res = parse_argument_expression_list(lexer);
				right = res.node;
			}
			res = require_token(lexer, ')');
			if (res.status == PARSER_STATUS_FATAL)
				return res;
			Ast* node = allocate_node();
			node->kind = AST_EXPRESSION_POSTFIX_BINARY;
			node->expression_postfix_binary.left = left;
			node->expression_postfix_binary.right = right;
			node->expression_postfix_binary.po = POSTFIX_PROC_CALL;
			res.node = node;
		} break;
		case '.': {
			lexer_next(lexer);
			res = parse_identifier(lexer);
			if (res.status == PARSER_STATUS_FATAL)
				return res;

			Ast* node = allocate_node();
			node->kind = AST_EXPRESSION_POSTFIX_BINARY;
			node->expression_postfix_binary.left = left;
			node->expression_postfix_binary.right = res.node;
			node->expression_postfix_binary.po = POSTFIX_DOT;
			res.node = node;
		} break;
		case TOKEN_ARROW: {
			lexer_next(lexer);
			res = parse_identifier(lexer);
			if (res.status == PARSER_STATUS_FATAL)
				return res;
			Ast* node = allocate_node();
			node->kind = AST_EXPRESSION_POSTFIX_BINARY;
			node->expression_postfix_binary.left = left;
			node->expression_postfix_binary.right = res.node;
			node->expression_postfix_binary.po = POSTFIX_ARROW;
			res.node = node;
		} break;
		case TOKEN_PLUS_PLUS: {
			lexer_next(lexer);
			Ast* node = allocate_node();
			node->kind = AST_EXPRESSION_POSTFIX_UNARY;
			node->expression_postfix_unary.expr = left;
			node->expression_postfix_unary.po = POSTFIX_PLUS_PLUS;
			res.node = node;
		} break;
		case TOKEN_MINUS_MINUS: {
			lexer_next(lexer);
			Ast* node = allocate_node();
			node->kind = AST_EXPRESSION_POSTFIX_UNARY;
			node->expression_postfix_unary.expr = left;
			node->expression_postfix_unary.po = POSTFIX_MINUS_MINUS;
			res.node = node;
		} break;
		default:
			finding = false;
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
	Parser_Result res = { 0 };
	Ast* last_node = 0;

	res = parse_assignment_expression(lexer);
	if (res.status == PARSER_STATUS_FATAL)
		return res;

	Ast* left = res.node;

	while(lexer_peek(lexer)->type == ',')
	{
		lexer_next(lexer);

		Parser_Result right = parse_assignment_expression(lexer);

		Ast* node = allocate_node();
		node->kind = AST_EXPRESSION_ARGUMENT_LIST;
		node->expression_argument_list.next = right.node;
		node->expression_argument_list.expr = left;
		left = node;
		res.node = node;
	}

	return res;
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
			lexer_next(lexer);
			Token* next = lexer_peek(lexer);
			if(next->type == '(') {
				lexer_next(lexer); // eat (
				Parser_Result r = parse_type_name(lexer);
				if(r.status == PARSER_STATUS_FATAL)
					return r;
				
				Parser_Result n = require_token(lexer, ')');
				if(n.status == PARSER_STATUS_FATAL)
					return n;
					
				res.node = allocate_node();
				res.node->kind = AST_EXPRESSION_SIZEOF;
				res.node->expression_sizeof.is_type_name = true;
				res.node->expression_sizeof.type = r.node;
			} else {
				Parser_Result r = parse_unary_expression(lexer);
				if(r.status == PARSER_STATUS_FATAL)
					return r;
				res.node = allocate_node();
				res.node->kind = AST_EXPRESSION_SIZEOF;
				res.node->expression_sizeof.is_type_name = false;
				res.node->expression_sizeof.expr;
			}
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
		res = require_token(lexer, ')');
		if (res.status == PARSER_STATUS_FATAL)
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
			if (op->type == TOKEN_LOGIC_OR) {
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
	Ast* condition = res.node;

	if (res.status == PARSER_STATUS_FATAL)
		return res;

	// ternary operator
	Token* next = lexer_peek(lexer);
	if (next->type == '?') {
		lexer_next(lexer);
		res = parse_expression(lexer);
		Ast* case_true = res.node;
		if (res.status == PARSER_STATUS_FATAL)
			return res;
		res = require_token(lexer, ':');
		if (res.status == PARSER_STATUS_FATAL)
			return res;
		res = parse_conditional_expression(lexer);
		Ast* case_false = res.node;
		if (res.status == PARSER_STATUS_FATAL)
			return res;

		Ast* node = allocate_node();
		node->kind = AST_EXPRESSION_TERNARY;
		node->expression_ternary.condition = condition;
		node->expression_ternary.case_true = case_true;
		node->expression_ternary.case_false = case_false;
		res.node = node;
	}

	return res;
}

// assignment-expression:
// conditional-expression (unary-expression is a more specific conditional-expression)
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

	// TODO(psv): maybe implement , operator

	return res;
}

Parser_Result
parse_identifier(Lexer* lexer) {
	Parser_Result res = { 0 };
	Token* t = lexer_next(lexer);
	if (t->type != TOKEN_IDENTIFIER) {
		// TODO(psv): write error here
		res.status = PARSER_STATUS_FATAL;
		return res;
	}

	res.node = allocate_node();
	res.node->kind = AST_EXPRESSION_PRIMARY_IDENTIFIER;
	res.node->expression_primary.data = t;

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


#define fprintf(...) fprintf(__VA_ARGS__); fflush(stdout)

void parser_print_abstract_declarator(FILE*, Ast*);
void parser_print_specifiers_qualifiers(FILE* out, Ast* sq);

void
parser_print_token(FILE* out, Token* t) {
	fprintf(out, "%.*s", t->length, t->data);
}

void
parser_print_type_qualifier_list(FILE* out, Ast* q) {
	if (!q) return;
	assert(q->kind == AST_TYPE_INFO);
	if (q->specifier_qualifier.qualifiers & TYPE_QUALIFIER_CONST) {
		fprintf(out, "const ");
	}
	if (q->specifier_qualifier.qualifiers & TYPE_QUALIFIER_VOLATILE) {
		fprintf(out, "volatile ");
	}
}

void
parser_print_pointer(FILE* out, Ast* pointer) {
	fprintf(out, "*");
	parser_print_type_qualifier_list(out, pointer->pointer.qualifiers);
	if (pointer->pointer.next) {
		parser_print_pointer(out, pointer->pointer.next);
	}
}

void
parser_print_param_decl(FILE* out, Ast* decl) {
	assert(decl->kind == AST_PARAMETER_DECLARATION);
	parser_print_specifiers_qualifiers(out, decl->parameter_decl.decl_specifiers);
	parser_print_abstract_declarator(out, decl->parameter_decl.declarator);
}

void
parser_print_parameter_list(FILE* out, Ast* p) {
	for (u64 i = 0; i < array_length(p->parameter_list.param_decl); ++i) {
		if (i != 0) fprintf(out, ",");
		parser_print_param_decl(out, p->parameter_list.param_decl[i]);
	}
	if (p->parameter_list.is_vararg)
		fprintf(out, ", ...");
}

void
parser_print_direct_abstract_declarator(FILE* out, Ast* ast) {
	
	if (ast->direct_abstract_decl.type == DIRECT_ABSTRACT_DECL_NONE) {
		fprintf(out, "(");
		parser_print_abstract_declarator(out, ast->direct_abstract_decl.left_opt);
		assert(ast->direct_abstract_decl.right_opt == 0);
		fprintf(out, ")");
		return;
	} else if(ast->direct_abstract_decl.left_opt) {
		parser_print_direct_abstract_declarator(out, ast->direct_abstract_decl.left_opt);
	}

	switch (ast->direct_abstract_decl.type) {
		case DIRECT_ABSTRACT_DECL_FUNCTION: {
			fprintf(out, "(");
			if (ast->direct_abstract_decl.right_opt) {
				parser_print_parameter_list(out, ast->direct_abstract_decl.right_opt);
			}
			fprintf(out, ")");
		} break;
		case DIRECT_ABSTRACT_DECL_ARRAY: {
			fprintf(out, "[");
			if (ast->direct_abstract_decl.right_opt) {
				parser_print_ast(out, ast->direct_abstract_decl.right_opt);
			}
			fprintf(out, "]");
		} break;
		case DIRECT_ABSTRACT_DECL_NAME: {
			parser_print_token(out, ast->direct_abstract_decl.name);
		} break;
		default: fprintf(out, "<invalid direct abstract declarator>"); break;
	}
}

void
parser_print_abstract_declarator(FILE* out, Ast* a) {
	if (a->abstract_type_decl.pointer) {
		parser_print_pointer(out, a->abstract_type_decl.pointer);
	}
	if (a->abstract_type_decl.direct_abstract_decl) {
		parser_print_direct_abstract_declarator(out, a->abstract_type_decl.direct_abstract_decl);
	}
}

void parser_print_struct_declaration(FILE* out, Ast* s);

void
parser_print_struct_declaration_list(FILE* out, Ast* l) {
	for(u64 i = 0; i < array_length(l->struct_declaration_list.list); ++i) {
		if(i > 0) fprintf(out, "\n");
		parser_print_struct_declaration(out, l->struct_declaration_list.list[i]);
	}
}

void
parser_print_struct_declarator(FILE* out, Ast* d) {
	assert(d->kind == AST_TYPE_STRUCT_DECLARATOR);
	parser_print_abstract_declarator(out, d->struct_declarator.declarator);
}

void
parser_print_struct_declarator_bitfield(FILE* out, Ast* d) {
	assert(d->kind == AST_TYPE_STRUCT_DECLARATOR_BITFIELD);
	if(d->struct_declarator_bitfield.declarator) {
		parser_print_ast(out, d->struct_declarator_bitfield.declarator);
	}
	fprintf(out, " : ");
	parser_print_ast(out, d->struct_declarator_bitfield.const_expr);
}

void
parser_print_struct_declarator_list(FILE* out, Ast* d) {
	assert(d->kind == AST_TYPE_STRUCT_DECLARATOR_LIST);
	for(u64 i = 0; i < array_length(d->struct_declarator_list.list); ++i) {
		if(i > 0) fprintf(out, ", ");
		Node_Kind kind = d->struct_declarator_list.list[i]->kind;
		Ast* node = d->struct_declarator_list.list[i];
		if(kind == AST_TYPE_STRUCT_DECLARATOR){
			parser_print_struct_declarator(out, node);
		} else if(kind == AST_TYPE_STRUCT_DECLARATOR_BITFIELD) {
			parser_print_struct_declarator_bitfield(out, d->struct_declaration_list.list[i]);
		}
	}
}

void
parser_print_struct_declaration(FILE* out, Ast* s) {
	assert(s->kind == AST_STRUCT_DECLARATION);
	parser_print_specifiers_qualifiers(out, s->struct_declaration.spec_qual);
	fprintf(out, " ");
	parser_print_struct_declarator_list(out, s->struct_declaration.struct_decl_list);
	fprintf(out, ";");
}

void
parser_print_enumerator(FILE* out, Ast* e) {
	assert(e->kind == AST_ENUMERATOR);
	parser_print_token(out, e->enumerator.enum_constant);
	fprintf(out, " ");
	if(e->enumerator.const_expr) {
		fprintf(out, " = ");
		parser_print_ast(out, e->enumerator.const_expr);
	}
}

void
parser_print_enumerator_list(FILE* out, Ast* el) {
	assert(el->kind == AST_ENUMERATOR_LIST);
	for(u64 i = 0; i < array_length(el->enumerator_list.list); ++i) {
		if(i > 0) fprintf(out, ", ");
		parser_print_enumerator(out, (Ast*)el->enumerator_list.list[i]);
	}
}

void
parser_print_struct_description(FILE* out, Ast* sd) {
	assert(sd->kind == AST_STRUCT_DECLARATION_LIST);
	for(u64 i = 0; i < array_length(sd->struct_declaration_list.list); ++i) {
		if(i > 0) fprintf(out, " ");
		parser_print_struct_declaration(out, sd->struct_declaration_list.list[i]);
	}
}

void
parser_print_specifiers_qualifiers(FILE* out, Ast* sq) {
	parser_print_type_qualifier_list(out, sq);
	switch (sq->specifier_qualifier.kind) {
		case TYPE_VOID:
			fprintf(out, "void");
			break;
		case TYPE_PRIMITIVE:{
			for (s32 i = 0; i < ARRAY_LENGTH(sq->specifier_qualifier.primitive); ++i) {
				for (s32 c = 0; c < sq->specifier_qualifier.primitive[i]; ++c) {
					if (c != 0) fprintf(out, " ");
					switch (i) {
						case TYPE_PRIMITIVE_CHAR:		fprintf(out, "char "); break;
						case TYPE_PRIMITIVE_DOUBLE:		fprintf(out, "double "); break;
						case TYPE_PRIMITIVE_FLOAT:		fprintf(out, "float "); break;
						case TYPE_PRIMITIVE_INT:		fprintf(out, "int "); break;
						case TYPE_PRIMITIVE_LONG:		fprintf(out, "long "); break;
						case TYPE_PRIMITIVE_SHORT:		fprintf(out, "short "); break;
						case TYPE_PRIMITIVE_SIGNED:		fprintf(out, "signed "); break;
						case TYPE_PRIMITIVE_UNSIGNED:	fprintf(out, "unsigned "); break;
						default: fprintf(out, "<invalid primitive type>"); break;
					}
				}
			}
		}break;
		case TYPE_STRUCT: {
			fprintf(out, "struct ");
			if(sq->specifier_qualifier.struct_name)
				parser_print_token(out, sq->specifier_qualifier.struct_name);
			if(sq->specifier_qualifier.struct_desc) {
				fprintf(out, " { ");
				parser_print_struct_description(out, sq->specifier_qualifier.struct_desc);
				fprintf(out, " } ");
			}
		}break;
		case TYPE_UNION: {
			fprintf(out, "union ");
			if(sq->specifier_qualifier.struct_name)
				parser_print_token(out, sq->specifier_qualifier.struct_name);
			if(sq->specifier_qualifier.enumerator_list) {
				fprintf(out, " { ");
				parser_print_enumerator_list(out, sq->specifier_qualifier.enumerator_list);
				fprintf(out, " } ");
			}
		}break;
		case TYPE_ALIAS: {
			parser_print_token(out, sq->specifier_qualifier.alias);
		}break;
		case TYPE_ENUM: {
			fprintf(out, "enum ");
			if(sq->specifier_qualifier.enum_name)
				parser_print_token(out, sq->specifier_qualifier.enum_name);

			fprintf(out, "{");
			parser_print_enumerator_list(out, sq->specifier_qualifier.enumerator_list);
			fprintf(out, "}");
		}break;
		default: fprintf(out, "<invalid type specifier or qualifier>"); break;
	}
}

void
parser_print_typename(FILE* out, Ast* node) {
	parser_print_specifiers_qualifiers(out, node->type_name.qualifiers_specifiers);
	parser_print_abstract_declarator(out, node->type_name.abstract_declarator);
}

void
parser_print_ast(FILE* out, Ast* ast) {
	if (!ast) return;

	switch (ast->kind) {

		case AST_EXPRESSION_PRIMARY_CONSTANT:
		case AST_EXPRESSION_PRIMARY_STRING_LITERAL:
		case AST_EXPRESSION_PRIMARY_IDENTIFIER: {
			fprintf(out, "%.*s", ast->expression_primary.data->length, ast->expression_primary.data->data);
			fflush(out);
		}break;
		case AST_EXPRESSION_CONDITIONAL:
			break;
		case AST_EXPRESSION_ASSIGNMENT: {
			parser_print_ast(out, ast->expression_binary.left);
			fprintf(out, " ");
			switch (ast->expression_binary.bo) {
				case BINOP_EQUAL: fprintf(out, "="); break;
				case BINOP_AND_EQ: fprintf(out, "&="); break;
				case BINOP_OR_EQ: fprintf(out, "|="); break;
				case BINOP_MINUS_EQ: fprintf(out, "-="); break;
				case BINOP_PLUS_EQ: fprintf(out, "+="); break;
				case BINOP_MOD_EQ: fprintf(out, "%%="); break;
				case BINOP_TIMES_EQ: fprintf(out, "*="); break;
				case BINOP_DIV_EQ: fprintf(out, "/="); break;
				case BINOP_XOR_EQ: fprintf(out, "^="); break;
				case BINOP_SHL_EQ: fprintf(out, "<<="); break;
				case BINOP_SHR_EQ: fprintf(out, ">>="); break;
				default: fprintf(out, "<invalid assignment op>"); break;
			}
			fprintf(out, " ");
			parser_print_ast(out, ast->expression_binary.right);
		} break;
		case AST_EXPRESSION_ARGUMENT_LIST: {
			parser_print_ast(out, ast->expression_argument_list.expr);
			fprintf(out, ", ");
			parser_print_ast(out, ast->expression_argument_list.next);
		} break;
		case AST_EXPRESSION_UNARY:{
			switch (ast->expression_unary.uo) {
				case UNOP_ADDRESS_OF: fprintf(out, "&"); break;
				case UNOP_DEREFERENCE: fprintf(out, "*"); break;
				case UNOP_MINUS: fprintf(out, "-"); break;
				case UNOP_PLUS: fprintf(out, "+"); break;
				case UNOP_MINUS_MINUS: fprintf(out, "--"); break;
				case UNOP_PLUS_PLUS: fprintf(out, "++"); break;
				case UNOP_NOT_BITWISE: fprintf(out, "~"); break;
				case UNOP_NOT_LOGICAL: fprintf(out, "!"); break;
				default: fprintf(out, "<unknown expression unary>"); break;
			}
			parser_print_ast(out, ast->expression_unary.expr);
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
			fprintf(out, "(");
			parser_print_ast(out, ast->expression_binary.left);
			fprintf(out, " ");
				switch (ast->expression_binary.bo) {
				case BINOP_SHL: fprintf(out, "<<"); break;
				case BINOP_SHR: fprintf(out, ">>"); break;
				default: fprintf(out, "<invalid shift operator>"); break;
			}
			fprintf(out, " ");
			parser_print_ast(out, ast->expression_binary.right);
			fprintf(out, ")");
			break;
		case AST_EXPRESSION_RELATIONAL:
			fprintf(out, "(");
			parser_print_ast(out, ast->expression_binary.left);
			fprintf(out, " ");
			switch (ast->expression_binary.bo) {
				case '<': fprintf(out, "<"); break;
				case '>': fprintf(out, ">"); break;
				case BINOP_LE: fprintf(out, "<="); break;
				case BINOP_GE: fprintf(out, ">="); break;
				default: fprintf(out, "<invalid relational operator>"); break;
			}
			fprintf(out, " ");
			parser_print_ast(out, ast->expression_binary.right);
			fprintf(out, ")");
			break;
		case AST_EXPRESSION_EQUALITY:
			fprintf(out, "(");
			parser_print_ast(out, ast->expression_binary.left);
			switch (ast->expression_binary.bo) {
				case BINOP_EQUAL_EQUAL: fprintf(out, " == "); break;
				case BINOP_NOT_EQUAL: fprintf(out, " != "); break;
				default: fprintf(out, "<invalid equality operator>"); break;
			}
			parser_print_ast(out, ast->expression_binary.right);
			fprintf(out, ")");
			break;
		case AST_EXPRESSION_AND: {
			fprintf(out, "(");
			parser_print_ast(out, ast->expression_binary.left);
			fprintf(out, " & ");
			parser_print_ast(out, ast->expression_binary.right);
			fprintf(out, ")");
		} break;
		case AST_EXPRESSION_EXCLUSIVE_OR: {
			fprintf(out, "(");
			parser_print_ast(out, ast->expression_binary.left);
			fprintf(out, " ^ ");
			parser_print_ast(out, ast->expression_binary.right);
			fprintf(out, ")");
		}break;
		case AST_EXPRESSION_INCLUSIVE_OR: {
			fprintf(out, "(");
			parser_print_ast(out, ast->expression_binary.left);
			fprintf(out, " | ");
			parser_print_ast(out, ast->expression_binary.right);
			fprintf(out, ")");
		}break;
		case AST_EXPRESSION_LOGICAL_AND: {
			fprintf(out, "(");
			parser_print_ast(out, ast->expression_binary.left);
			fprintf(out, " && ");
			parser_print_ast(out, ast->expression_binary.right);
			fprintf(out, ")");
		} break;
		case AST_EXPRESSION_LOGICAL_OR: {
			fprintf(out, "(");
			parser_print_ast(out, ast->expression_binary.left);
			fprintf(out, " || ");
			parser_print_ast(out, ast->expression_binary.right);
			fprintf(out, ")");
		}break;
		case AST_EXPRESSION_SIZEOF: {
			fprintf(out, "sizeof (");
			if(ast->expression_sizeof.is_type_name) {
				parser_print_typename(out, ast->expression_sizeof.type);
			} else {
				parser_print_ast(out, ast->expression_sizeof.expr);
			}
			fprintf(out, ")");
		}break;

		case AST_EXPRESSION_POSTFIX_UNARY: {
			if (ast->expression_postfix_unary.expr) {
				parser_print_ast(out, ast->expression_postfix_unary.expr);
				switch (ast->expression_postfix_unary.po) {
					case POSTFIX_MINUS_MINUS: fprintf(out, "--"); break;
					case POSTFIX_PLUS_PLUS: fprintf(out, "++"); break;
					default: fprintf(out, "<unknown postfix unary>"); break;
				}
			}
		}break;
		case AST_EXPRESSION_POSTFIX_BINARY: {
			parser_print_ast(out, ast->expression_postfix_binary.left);
			switch (ast->expression_postfix_binary.po) {
				case POSTFIX_ARROW: fprintf(out, "->"); break;
				case POSTFIX_DOT: fprintf(out, "."); break;
				case POSTFIX_ARRAY_ACCESS: fprintf(out, "["); break;
				case POSTFIX_PROC_CALL: fprintf(out, "("); break;
				default: fprintf(out, "<unknown postfix binary>"); break;
			}
			if (ast->expression_postfix_binary.right) {
				parser_print_ast(out, ast->expression_postfix_binary.right);
			}
			switch (ast->expression_postfix_binary.po) {
				case POSTFIX_ARROW: break;
				case POSTFIX_DOT: break;
				case POSTFIX_ARRAY_ACCESS: fprintf(out, "]"); break;
				case POSTFIX_PROC_CALL: fprintf(out, ")"); break;
				default: fprintf(out, "<unknown postfix binary>"); break;
			}
		}break;
		case AST_EXPRESSION_TERNARY: {
			fprintf(out, "(");
			parser_print_ast(out, ast->expression_ternary.condition);
			fprintf(out, ") ? (");
			parser_print_ast(out, ast->expression_ternary.case_true);
			fprintf(out, ") : (");
			parser_print_ast(out, ast->expression_ternary.case_false);
			fprintf(out, ")");
			fflush(out);
		}break;

		case AST_CONSTANT_FLOATING_POINT:
		case AST_CONSTANT_INTEGER: {
			fprintf(out, "%.*s", ast->expression_primary.data->length, ast->expression_primary.data->data);
			fflush(out);
		}break;
		case AST_CONSTANT_ENUMARATION: {
			fprintf(out, "%.*s", ast->expression_primary.data->length, ast->expression_primary.data->data);
			fflush(out);
		}break;
		case AST_CONSTANT_CHARACTER:
			fprintf(out, "'%.*s'", ast->expression_primary.data->length, ast->expression_primary.data->data);
			fflush(out);
			break;
		case AST_TYPE_NAME:
			parser_print_typename(out, ast);
			break;

		case AST_TYPE_ABSTRACT_DECLARATOR: {
			if (ast->abstract_type_decl.pointer)
				parser_print_pointer(out, ast->abstract_type_decl.pointer);
			if (ast->abstract_type_decl.direct_abstract_decl) {
				parser_print_direct_abstract_declarator(out, ast->abstract_type_decl.direct_abstract_decl);
			}
		}break;
		case AST_TYPE_DIRECT_ABSTRACT_DECLARATOR: {
			parser_print_direct_abstract_declarator(out, ast);
		} break;

		case AST_PARAMETER_LIST:
			break;

		default: {
			printf("<unknown ast node>");
			fflush(out);
		}break;
	}
}
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "parser.h"

typedef struct {
    u8* data;
    s32 size_bytes;
} File_Info;

File_Info load_file(const char* filename) {
    File_Info result = {0};
    FILE* f = fopen(filename, "rb");

    if(!f) {
        printf("could not open file %s\n", filename);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    result.size_bytes = ftell(f);
    fseek(f, 0, SEEK_SET);

    result.data = calloc(1, result.size_bytes + 1);
    fread(result.data, result.size_bytes, 1, f);

    fclose(f);
    return result;
}

int main(int argc, char** argv) {
    File_Info finfo= { 0 };
    if(argc < 2) {
        finfo = load_file("./test/test.h");
    } else {
        finfo = load_file(argv[1]);
    }

    Lexer lexer = {0};
    Token* tokens = lexer_cstr(&lexer, finfo.data, finfo.size_bytes, 0);
	Parser_Result res = parse_expression(&lexer);
	//Parser_Result res = parse_type_name(&lexer);

    if(res.status == PARSER_STATUS_FATAL) {
        fprintf(stderr, "%s", res.error_message);
    }
	parser_print_ast(stdout, res.node);

    return 0;
}
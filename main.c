#include <stdio.h>
#include "lexer.h"

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
    File_Info finfo = load_file("lexer.c");

    Lexer lexer = {0};
    lexer_cstr(&lexer, finfo.data, finfo.size_bytes, 0);
    return 0;
}
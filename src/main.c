#include "lexer/lexer.h"
#include "parser/parser.h"
#include "symantic/analyz.h"
#include "MIR/convert.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static char* read_file_contents(const char*);

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "\033[34m[help]\033[0m use >%s <file.gr>\n", argv[0]);
        return 1;
    }

    char* source = read_file_contents(argv[1]);
    if (source == NULL) {
        return 1;
    }

    struct lexer* lexer = new_lexer(source);
    if (lexer == NULL) {
        free(source);
        return 1;
    }

    tokenize(lexer);
    struct parser* parser = new_parser(lexer->tokens, lexer);
    if (parser == NULL) {
        free_lexer(lexer);
        free(source);
        return 1;
    }

    bool success = parse(parser);
    if (success && parser->node_count > INT_MAX) {
        fprintf(stderr, "[error] too many top-level nodes for semantic analysis\n");
        success = false;
    }
    if (success) {
        success = analyz(parser->root, (int)parser->node_count);
    }
    if (success) {
        struct converter* converter = create_converter();
        if (converter == NULL) {
            fprintf(stderr, "[error] could not allocate MIR converter\n");
            success = false;
        } else {
            convert(converter, parser->root, (int)parser->node_count);
            print_MIR(converter);
            free_MIR(converter);
        }
    }

    free_parser(parser);
    free_lexer(lexer);
    free(source);

    return success ? 0 : 1;
}

static char* read_file_contents(const char* file_path) {
    FILE* file = fopen(file_path, "rb");
    if (file == NULL) {
        fprintf(stderr, "\033[31m[error]\033[0m cant open file: %s\n", file_path);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "\033[31m[error]\033[0m cant seek file: %s\n", file_path);
        fclose(file);
        return NULL;
    }

    long file_size = ftell(file);
    if (file_size < 0) {
        fprintf(stderr, "\033[31m[error]\033[0m cant get file size: %s\n", file_path);
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "\033[31m[error]\033[0m cant seek file: %s\n", file_path);
        fclose(file);
        return NULL;
    }

    char* buffer = malloc((size_t)file_size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "\033[31m[error]\033[0m cant allocate memory for file: %s\n", file_path);
        fclose(file);
        return NULL;
    }

    size_t read_count = fread(buffer, 1, (size_t)file_size, file);
    if (read_count != (size_t)file_size) {
        fprintf(stderr, "\033[31m[error]\033[0m cant read file: %s\n", file_path);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[file_size] = '\0';
    fclose(file);
    return buffer;
}

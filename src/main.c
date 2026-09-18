#include "lexer/lexer.h"

int main(int argc, char** argv) {
    
    char* source = "def main() : int { var sum = 3 + 5; return 42; }";
    struct lexer* lexer = new_lexer(source);
    tokenize(lexer);
    print_lexer(lexer);
    free_lexer(lexer);

    return 0;

}
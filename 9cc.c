#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

int pos = 0;
Token tokens[100];
char *user_input;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の数が正しくありません");
        return 1;
    }

    tokenize(argv[1]);
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}

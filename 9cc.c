#include<stdio.h>
#include<stdlib.h>

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    char *p = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // 数字以外の文字を見つけるまで数字を出力する
    printf("    mov rax, %ld\n", strtol(p, &p, 10));

    // pが終端文字に達するまで行う
    while(*p){
        if(*p == '+'){
            p++;
            printf("    add rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        if(*p == '-'){
            p++;
            printf("    sub rax, %ld\n", strtol(p, &p, 10));
            continue;
        }

        fprintf(stderr, "予期しない文字です: '%c'\n", *p);
        return EXIT_FAILURE;
    }

    printf("    ret\n");
    return 0;
}
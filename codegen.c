#include "9cc.h"

static int depth;

static void push(){
    printf("    push rax\n");
    depth++;
}

static void pop(char *arg){
    printf("    pop %s\n", arg);
    depth--;
}

// ノードに対応するローカル変数のアドレスを取得してraxにコピーする
static void gen_addr(Node *node){
    if(node->kind == ND_LVAR){
        int offset = (node->name - 'a' + 1) * 8;
        printf("    lea rax, [rbp-%d]\n", offset);
        return;
    }

    error("ローカル変数ではありません");
}

static void gen_expr(Node *node){

    switch(node->kind){
        case ND_NUM:
            printf("    mov rax, %d\n", node->val);
            return;
        case ND_NEG:
            gen_expr(node->lhs);
            printf("    neg rax\n");
            return;
        
        // ND_ASSIGNとND_LVARはアドレス計算が必要
        case ND_LVAR:
            gen_addr(node); // raxにローカル変数のアドレスをコピー
            printf("    mov rax, [rax]\n"); // raxにローカル変数の値をコピー
            return;
        case ND_ASSIGN:
            gen_addr(node->lhs); // raxに左辺値のアドレスをコピー
            push(); // スタックに積み込む
            gen_expr(node->rhs); // 右辺値を計算しraxにコピー
            pop("rdi"); // スタックに積み込んでいた左辺値のアドレスをrdiにpopする
            printf("    mov [rdi], rax\n"); // 左辺値に右辺値を代入
            return;
    }

    gen_expr(node->rhs);
    push();
    gen_expr(node->lhs);
    pop("rdi");

    switch(node->kind){

        case ND_ADD:
            printf("    add rax, rdi\n");
            return;
        case ND_SUB:
            printf("    sub rax, rdi\n");
            return;
        case ND_MUL:
            printf("    imul rax, rdi\n");
            return;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            return;

        case ND_EQ:
        case ND_NE:
        case ND_L:
        case ND_LE:
            printf("    cmp rax, rdi\n");

            if(node->kind == ND_EQ) printf("    sete al\n");
            else if(node->kind == ND_NE) printf("   setne al\n");
            else if(node->kind == ND_L) printf("    setl al\n");
            else if(node->kind == ND_LE) printf("   setle al\n");

            printf("    movzb rax, al\n");
            return;
    }

    error("構文が正しくありません");
}

static void gen_stmt(Node *node){
    if(node->kind == ND_EXPR){
        gen_expr(node->lhs);
        return;
    }

    error("不正な(正しくない)文です");
}

// 抽象構文木を下りながら(再帰的に)アセンブリを出力する
void codegen(Node *node){
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // prologue
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n"); // 8*26 = 208バイト下げる(スタックを積む)

    // 構文木を具体的に下る動作を行う
    for(Node *n = node; n; n = n->next){
        gen_stmt(n);
        assert(depth == 0);
    }

    // epilogue
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");

}

/* TODO

・ND_ASSIGN, ND_EXPR, ND_VARの場合のコード生成
・gen_stmtの作成

*/
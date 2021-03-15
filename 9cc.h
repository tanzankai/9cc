#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>
#include<assert.h>

// tokenize.c

// トークンの種類
typedef enum{
    TK_PUNCT, // 区切り文字
    TK_IDENT, // 識別子
    TK_NUM, // 整数トークン
    TK_EOF, // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token{
    TokenKind kind; // トークンの型(種類)
    Token *next; // 次の入力トークン
    int val; // kindがTK_NUMの場合、その数値
    char *loc; // トークンの位置
    int len; // トークンの長さ
};

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
Token *tokenize(char *input);


// parse.c

// 抽象構文木のノードの種類
typedef enum{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NEG, // 単項-
    ND_ASSIGN, // =
    ND_EXPR, // expression
    ND_LVAR, // ローカル変数
    ND_EQ, // ==
    ND_NE, // !=
    ND_L, // <
    ND_LE, // <=
    ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node{
    NodeKind kind; // ノードの種類
    Node *lhs; // 左の子
    Node *rhs; // 右の子
    Node *next; // 次のノード
    int val; // (kindがND_NUMのとき) 数値
    int offset; // ローカル変数のベースポインタからのオフセット
    char name; // (kindがND_LVARのとき) 変数名
};

Node *parse(Token *tok);

// codegen.c

void codegen(Node *node);
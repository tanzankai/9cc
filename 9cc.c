#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>

// トークンの種類
typedef enum{
    TK_RESERVED, // 記号
    TK_NUM, // 整数トークン
    TK_EOF, // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token{
    TokenKind kind; // トークンの型(種類)
    Token *next; // 次の入力トークン
    int val; // kindがTK_NUMの場合、その数値
    char *str; // トークン文字列
    int len; // トークンの長さ
};

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;


// エラーを報告するための関数
// printfと同じ引数をとる
void error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input; // エラーが発生した位置
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

// 次のトークンが"期待している" "記号" の時にはトークンを1つ読み進める
// 正常に処理を行うことができれば真を返す そうでなければ偽を返す
bool consume(char *op){
    if(token->kind != TK_RESERVED || strlen(op) != token->len ||
    strncmp(token->str, op, token->len)) return false;
    token = token->next;
    return true;
}

// 次のトークンが"期待している" "記号" の時にはトークンを1つ読み進める
// それ以外の場合にはエラーを報告する
void expect(char *op){
    if(token->kind != TK_RESERVED || strlen(op) != token->len ||
    strncmp(token->str, op, token->len)){
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
}

// 次のトークンが"数値"ならばトークンを1つ読み進めてその数値を返す
// それ以外の場合にはエラーを報告する
int expect_number(){
    if(token->kind != TK_NUM) error_at(token->str, "数ではありません");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof(){
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str, int len){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(){
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while(*p){
        if(isspace(*p)){
            p++;
            continue;
        }

        // 比較演算子 長いトークンから優先的にトークナイズできるようにする
        if(!strncmp("==", p, 2) ||
           !strncmp("!=", p, 2) ||
           !strncmp("<=", p, 2) ||
           !strncmp(">=", p, 2)){
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        // 四則演算と括弧と大小
        if(strchr("+-*/()<>", *p)){
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        if(isdigit(*p)){
            cur = new_token(TK_NUM, cur, p, 0); // 0は仮置き(実際の数字列の長さは以下で求める)
            char *base = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - base;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 0); // 0は仮置き

    // 入力文字列をトークナイズした結果生成されたトークン列の先頭を返す
    return head.next;
}

// 抽象構文木のノードの種類
typedef enum{
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
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
    int val; // (kindがND_NUMのとき) 数値
};

// 抽象構文木の新しいノードを作成する(二項演算子用)
Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 抽象構文木の新しいノードを作成する(数値用)
// 数値の場合は必ずノードの型はND_NUMであるうえ,子は存在しない
Node *new_node_num(int val){
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// 再帰下降構文解析用の関数expr,equality,relational,add,mul,unary,primary
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

/* 生成規則
優先順位は生成文法によって表現可能

expr = equality
equality = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add = mul ("+" mul | "-"  mul)*
mul = unary ("*" unary | "/" unary)*
unary = ("+" | "-")? primary
primary = num | "(" expr ")"

*/

// 生成規則を満たすように書き換える TODO

Node *expr(){
    return equality(); 
}

Node *equality(){
    Node *node = relational();
    for(;;){
        if(consume("==")){
            node = new_node(ND_EQ, node, relational());
        }
        else if(consume("!=")){
            node = new_node(ND_NE, node, relational());
        }
        else return node;
    }
}

Node *relational(){
    Node *node = add();
    for(;;){
        // >,>= の場合はlhsとrhsを入れ替えればよい
        if(consume("<=")){
            node = new_node(ND_LE, node, add());
        }
        else if(consume(">=")){
            node = new_node(ND_LE, add(), node);
        }
        else if(consume("<")){
            node = new_node(ND_L, node, add());
        }
        else if(consume(">")){
            node = new_node(ND_L, add(), node);
        }
        else return node;
    }
}

Node *add(){
    Node *node = mul();
    for(;;){
        if(consume("+")){
            node = new_node(ND_ADD, node, mul());
        }
        else if(consume("-")){
            node = new_node(ND_SUB, node, mul());
        }
        else return node;
    }
}

Node *mul(){
    Node *node = unary();

    for(;;){
        if(consume("*")){
            node = new_node(ND_MUL, node, unary());
        }
        else if(consume("/")){
            node = new_node(ND_DIV, node, unary());
        }
        else return node;
    }
}

Node *unary(){
    if(consume("+")){
        return primary();
    }
    else if(consume("-")){
        // 符号反転を表現するために -x を 0-x (二項の引き算)に見立てる
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    else return primary();
}

Node *primary(){
    // 1文字読んで "("  が来れば "(" expr ")" であることが確定
    if(consume("(")){
        Node *node = expr();
        expect(")"); // ()で閉じているかの確認
        return node;
    }

    // そうでなければ num (数値,かつ終端記号)が確定
    return new_node_num(expect_number());
}

// 比較演算子に対応するアセンブリを追加する TODO
// 抽象構文木を下りながら(再帰的に)アセンブリを出力する
void gen(Node *node){
    if(node->kind == ND_NUM){
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);
    // この時点で左の子、右の子は再帰的に処理が済んでいる


    // 自ノードの処理
    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch(node->kind){
        case ND_ADD:
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            printf("    sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("    imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            break;
        case ND_EQ:
            printf("    cmp rax, rdi\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_NE:
            printf("    cmp rax, rdi\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_L:
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LE:
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
    }

    // 計算結果をスタックに積み込む
    printf("    push rax\n");
}


int main(int argc, char **argv){
    if(argc != 2){
        error("引数の個数が正しくありません");
        return EXIT_FAILURE;
    }

    // トークナイズしたのちトークン列から抽象構文木を構築する
    user_input = argv[1];
    token = tokenize(argv[1]);
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコードを生成する
    gen(node);

    // スタックに残った値(計算結果)をraxにロードして返り値とする
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
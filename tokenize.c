#include "9cc.h"

static char *current_input;


void error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void verror_at(char *loc, char *fmt, va_list ap){
    int pos = loc - current_input;
    fprintf(stderr, "%s\n", current_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

void error_at(char *loc, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    verror_at(loc, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    verror_at(tok->loc, fmt, ap);
}



// 現在見ているトークンが記号opと一致しているかどうかを返す
bool equal(Token *tok, char *op){
    return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

Token *skip(Token *tok, char *op){
    if(!equal(tok, op)){
        error_tok(tok, "'%s'ではありません", op);
    }
    return tok->next;
}

// 新しいトークンを作成する
Token *new_token(TokenKind kind, char *start, char *end){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->len = end - start;
    return tok;
}

// pを先頭から見てqと一致しているかどうかを返す
static bool startswith(char *p, char *q){
    return strncmp(p, q, strlen(q)) == 0;
}

// 区切り文字を読み取り、その長さを返す
static int read_punct(char *p){
    // 長い記号から優先的に読み取る
    if(startswith(p, "==") || startswith(p, "!=") ||
       startswith(p, "<=") || startswith(p, ">=")){
           return 2;
       }
    
    return ispunct(*p) ? 1 : 0; // <ctype.h>
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p){
    current_input = p;
    Token head = {};
    Token *cur = &head;
    while(*p){
        if(isspace(*p)){
            p++;
            continue;
        }

        // 数値
        if(isdigit(*p)){
            cur = cur->next = new_token(TK_NUM, p, p);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        // 識別子(アルファベット小文字1文字)
        if('a'<=*p && *p <= 'z'){
            cur = cur->next = new_token(TK_IDENT, p, p+1);
            cur->len = 1;
            p += cur->len;
            continue;
        }

        // 区切り文字
        int punct_len = read_punct(p);
        if(punct_len){
            cur = cur->next = new_token(TK_PUNCT, p, p+punct_len);
            p += cur->len;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    cur = cur->next = new_token(TK_EOF, p, p);

    // 入力文字列をトークナイズした結果生成されたトークン列の先頭を返す
    return head.next;
}
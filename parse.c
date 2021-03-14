#include "9cc.h"

// 再帰下降構文解析用の関数はstaticにする
static Node *expr(Token **rest, Token *tok);
static Node *equality(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

static Node *new_node(NodeKind kind){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

// 子を2つもつノードを生成
static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs){
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

// 子を1つもつノードを生成
static Node *new_unary(NodeKind kind, Node *expr){
    Node *node = new_node(kind);
    node->lhs = expr;
    return node;
}

// 子が数値(終端記号)であるノードを生成
static Node *new_num(int val){
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

/* 生成規則
優先順位は生成文法によって表現可能

expr = equality
equality = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add = mul ("+" mul | "-"  mul)*
mul = unary ("*" unary | "/" unary)*
unary = ("+" | "-") unary | primary
primary = num | "(" expr ")"

*/
 
static Node *expr(Token **rest, Token *tok){
    return equality(&tok, tok);
}

static Node *equality(Token **rest, Token *tok){
    Node *node = relational(&tok, tok);

    for(;;){
        if(equal(tok, "==")){
            node = new_binary(ND_EQ, node, relational(&tok, tok->next));
            continue;
        }

        if(equal(tok, "!=")){
            node = new_binary(ND_NE, node, relational(&tok, tok->next));
            continue;
        }

        *rest = tok;
        return node;
    }
}

static Node *relational(Token **rest, Token *tok){
    Node *node = add(&tok, tok);

    for(;;){
        if(equal(tok, "<")){
            node = new_binary(ND_L, node, add(&tok, tok->next));
            continue;
        }

        if(equal(tok, "<=")){
            node = new_binary(ND_LE, node, add(&tok, tok->next));
            continue;
        }

        if(equal(tok, ">")){
            node = new_binary(ND_L, add(&tok, tok->next), node);
            continue;
        }

        if(equal(tok, ">=")){
            node = new_binary(ND_LE, add(&tok, tok->next), node);
            continue;
        }

        *rest = tok;
        return node;
    }
}

static Node *add(Token **rest, Token *tok){
    Node *node = mul(&tok, tok);

    for(;;){
        if(equal(tok, "+")){
            node = new_binary(ND_ADD, node, mul(&tok, tok->next));
            continue;
        }

        if(equal(tok, "-")){
            node = new_binary(ND_SUB, node, mul(&tok, tok->next));
            continue;
        }

        *rest = tok;
        return node;
    }
}

static Node *mul(Token **rest, Token *tok){
    Node *node = unary(&tok, tok);

    for(;;){
        if(equal(tok, "*")){
            node = new_binary(ND_MUL, node, unary(&tok, tok->next));
            continue;
        }

        if(equal(tok, "/")){
            node = new_binary(ND_DIV, node, unary(&tok, tok->next));
            continue;
        }

        *rest = tok;
        return node;
    }
}

static Node *unary(Token **rest, Token *tok){
    if(equal(tok, "+")) return unary(rest, tok->next);

    if(equal(tok, "-")) return new_unary(ND_NEG, unary(rest, tok->next));

    return primary(rest, tok);
}

static Node *primary(Token **rest, Token *tok){
    if(equal(tok, "(")){
        Node *node = expr(&tok, tok->next);
        *rest = skip(tok, ")");
        return node;
    }

    if(tok->kind == TK_NUM){
        Node *node = new_num(tok->val);
        *rest = tok->next;
        return node;
    }

    error_tok(tok, "構文が正しくありません");
}

// パースの結果生成された構文木を返す
Node *parse(Token *tok){
    Node *node = expr(&tok, tok);
    if(tok->kind != TK_EOF) error_tok(tok, "パースが完全には終了していません");

    return node;
}
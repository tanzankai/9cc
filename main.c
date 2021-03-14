#include "9cc.h"

int main(int argc, char **argv){
    if(argc != 2){
        error("引数の個数が正しくありません");
    }

    // トークナイズしたのちトークン列から抽象構文木を構築する
    Token *tok = tokenize(argv[1]);
    Node *node = parse(tok);

    codegen(node);
    
    return 0;
}
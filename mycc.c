#include "mycc.h"
#include <stdio.h>
#include <string.h>

// トークナイズした結果のトークン列はこのVectorに保存する．
Vector *tokens;
int pos;

// 複数の式を保存するための配列
Vector *code;

// 変数を保存するための配列
Map *variables;

void init()
{
  pos = 0;
  tokens = new_vector();
  code = new_vector();
  variables = new_map();
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  if (strcmp(argv[1], "-test") == 0)
  {
    runtest();
    return 0;
  }

  // トークナイズしてパースする．
  init();
  tokenize(argv[1]);
  program();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // プロローグ
  // 使った個数分の変数領域を確保する
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", 8 * variables->keys->len);

  // 先頭の式から順にコード生成
  for (int i = 0; (Node *)code->data[i]; i++)
  {
    gen(((Node *)code->data[i])->expr);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので，スタックが溢れないようにポップ
    printf("  pop rax\n");
  }

  // エピローグ
  // 最後の式の結果がRAXに残っているのでそれが返り値になる
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}

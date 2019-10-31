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
  debug_out("initialize start\n");
  pos = 0;
  tokens = new_vector();
  code = new_vector();
  variables = new_map();
  debug_out("initialized!\n");
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
  mycc_out(".intel_syntax noprefix\n");
  mycc_out(".global main\n");

  // 先頭の関数から順にコード生成
  for (int i = 0; (Node *)code->data[i]; i++)
  {
    debug_out("%d 個目の関数のアセンブリ生成 start\n", i);
    gen(((Node *)code->data[i]));
    debug_out("%d 個目の関数のアセンブリ生成 done\n", i);
  }
  return 0;
}

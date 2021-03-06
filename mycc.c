#include "mycc.h"

#include <stdio.h>
#include <string.h>

Token *tok_cur;
char *user_input;
// 複数の式を保存するための配列
Node *code;
// 変数を保存するための配列
VarList *variables;

void init() {
  debug_out("initialize start\n");
  // tokens = new_token();
  // code = new_node();
  variables = (VarList *)malloc(sizeof(VarList));
  debug_out("initialized!\n");
}

char *read_file(char *path) {
  FILE *fp = fopen(path, "r");
  if (fp == NULL) error("cant not open %s.", path);
  int filemax = 10 * 1024 * 1024;
  char *buf = malloc(sizeof(char) * filemax);
  int size = fread(buf, 1, filemax - 2, fp);
  if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
  buf[size] = '\0';
  return buf;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  Token *tokens;
  // トークナイズしてパースする．
  init();
  user_input = read_file(argv[1]);
  tok_cur = tokenize(user_input);
  code = program();

  // アセンブリの前半部分を出力
  mycc_out(".intel_syntax noprefix\n");
  mycc_out(".global main\n");

  int i = 1;
  // 先頭の関数から順にコード生成
  do {
    debug_out("%d 個目の関数のアセンブリ生成 start\n", i);
    code = gen(code);
    debug_out("%d 個目の関数のアセンブリ生成 done\n", i);
    i++;
  } while (code != NULL);
  return 0;
}

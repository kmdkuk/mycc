#ifndef MYCC_H
#define MYCC_H

#include <stdarg.h>

void error(char *fmt, ...) __attribute__((format(printf, 1, 2)));

// トークンの方を表す値
enum
{
  TK_NUM = 256, // 整数トークン
  TK_IDENT,     // 識別子
  TK_EOF,       // 入力の終わりを表すトークン
};

// トークンの型
typedef struct
{
  int ty;      // トークンの型
  int val;     // tyがTK_NUMの場合，その数値
  char *input; // トークン文字列(エラーメッセージ用)
} Token;

typedef struct
{
  Token *data;
  int capacity;
  int len;
} Vector;

typedef struct Node
{
  int ty;           // 演算子がND_NUM, ND_IDENT
  struct Node *lhs; // 左辺
  struct Node *rhs; // 右辺
  int val;          // tyがND_NUMの場合のみ使う
  char name;        // tyがND_IDENTの場合のみ使う
} Node;

Vector *new_vector();
void vec_push(Vector *vec, Token elem);

void program();
Node *stmt();
Node *assign();
Node *add();
Node *mul();
Node *term();

#endif

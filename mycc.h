#ifndef MYCC_H
#define MYCC_H

typedef struct
{
  void **data;
  int capacity;
  int len;
} Vector;

// トークンの型
typedef struct
{
  int ty;      // トークンの型
  int val;     // tyがTK_NUMの場合，その数値
  char *input; // トークン文字列(エラーメッセージ用)
} Token;

typedef struct Node
{
  int ty;           // 演算子がND_NUM
  struct Node *lhs; // 左辺
  struct Node *rhs; // 右辺
  int val;          // tyがND_NUMの場合のみ使う
} Node;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);

Node *add();
Node *mul();
Node *term();

#endif

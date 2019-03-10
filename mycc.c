#include "mycc.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int expect(int line, int expected, int actual)
{
  if (expected == actual)
    return 0;
  fprintf(stderr, "%d: %d expected, but got %d\n",
          line, expected, actual);
  exit(1);
}

void runtest()
{
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++)
  {
    Token *token = (Token *)malloc(sizeof(Token));
    token->ty = TK_NUM;
    token->val = i;
    vec_push(vec, *token);
  }

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, vec->data[0].val);
  expect(__LINE__, 50, vec->data[50].val);
  expect(__LINE__, 99, vec->data[99].val);

  printf("OK\n");
}

// トークナイズした結果のトークン列はこの配列に保存する．
// 100個以上のトークンは来ないものとする
Vector *tokens;
int pos = 0;

// エラーを報告するための関数
void error(char *s, char *p)
{
  fprintf(stderr, s, p);
  exit(1);
}

enum
{
  ND_NUM = 256, // 整数ノードの型
};

Node *new_node(int ty, Node *lhs, Node *rhs)
{
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val)
{
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

int consume(int ty)
{
  if (tokens->data[pos].ty != ty) // ここでセグフォ
  {
    return 0;
  }
  pos++;
  return 1;
}

Node *term()
{
  if (consume('('))
  {
    Node *node = add();
    if (!consume(')'))
      error("開きカッコに対応する閉じカッコがありません： %s\n", tokens->data[pos].input);
    return node;
  }

  if (tokens->data[pos].ty == TK_NUM)
    return new_node_num(tokens->data[pos++].val);

  error("数値でも開きカッコでもないトークンです： %s\n", tokens->data[pos].input);
}

Node *mul()
{
  Node *node = term();

  for (;;)
  {
    if (consume('*'))
      node = new_node('*', node, term());
    else if (consume('/'))
      node = new_node('/', node, term());
    else
      return node;
  }
}

Node *add()
{
  Node *node = mul();

  for (;;)
  {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Vector *new_vector()
{
  Vector *vec = (Vector *)malloc(sizeof(Vector));
  vec->data = (Token *)malloc(sizeof(Token) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, Token elem)
{
  if (vec->capacity == vec->len)
  {
    vec->capacity *= 2;
    vec->data = (Token *)realloc(vec->data, sizeof(Token) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

Token *new_token()
{
  Token *token = (Token *)malloc(sizeof(Token));
  return token;
}

// pが指している文字列をトークンに分割してtokensに保存する
void *tokenize(char *p)
{
  tokens = new_vector();
  while (*p)
  {
    // 空白文字をスキップ
    if (isspace(*p))
    {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
    {
      Token *token = new_token();
      token->ty = *p;
      token->input = p;
      vec_push(tokens, *token);
      p++;
      continue;
    }

    if (isdigit(*p))
    {
      Token *token = new_token();
      token->ty = TK_NUM;
      token->input = p;
      token->val = strtol(p, &p, 10);
      vec_push(tokens, *token);
      continue;
    }

    error("トークナイズできません： %s\n", p);
  }

  Token *token = new_token();
  token->ty = TK_EOF;
  token->input = p;
  vec_push(tokens, *token);
}

void gen(Node *node)
{
  if (node->ty == ND_NUM)
  {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty)
  {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
  }

  printf(" push rax\n");
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
  tokenize(argv[1]);
  Node *node = add();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成
  gen(node);

  // スタックトップに式全体の値が残っているはずなので
  // それをRAXにロードして関数からの返り値とする．
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

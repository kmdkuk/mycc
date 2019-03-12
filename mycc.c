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
    vec_push(vec, (void *)i);
  }

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (int)vec->data[0]);
  expect(__LINE__, 50, (int)vec->data[50]);
  expect(__LINE__, 99, (int)vec->data[99]);

  printf("OK\n");
}

// トークナイズした結果のトークン列はこのVectorに保存する．
Vector *tokens;
int pos = 0;

// 複数の式を保存するための配列
Node *code[100];

// エラーを報告するための関数
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

enum
{
  ND_NUM = 256, // 整数ノードの型
  ND_IDENT,     // 識別子のノードの型
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

Node *new_node_name(char *name)
{
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name[0];
  return node;
}

// (Token *)tokens->data[pos])->ty と 引数が等しい場合posを進める．
int consume(int ty)
{
  if (((Token *)tokens->data[pos])->ty != ty)
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
      error("開きカッコに対応する閉じカッコがありません： %s\n", ((Token *)tokens->data[pos])->input);
    return node;
  }

  if (((Token *)tokens->data[pos])->ty == TK_NUM)
    return new_node_num(((Token *)tokens->data[pos++])->val);

  if (((Token *)tokens->data[pos])->ty == TK_IDENT)
    return new_node_name(((Token *)tokens->data[pos++])->input);

  error("a-zの変数，数値，開きカッコでもないトークンです：%s\n", ((Token *)tokens->data[pos])->input);
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

void program()
{
  int i = 0;
  while (((Token *)tokens->data[pos])->ty != TK_EOF)
    code[i++] = stmt();
  code[i] = NULL;
}

Node *stmt()
{
  Node *node = assign();
  if (!consume(';'))
    error("';'ではないトークンです: %s", ((Token *)tokens->data[pos])->input);
  return node;
}

Node *assign()
{
  Node *node = add();
  for (;;)
  {
    if (consume('='))
      node = new_node('=', node, assign());
    else
      return node;
  }
}

Vector *new_vector()
{
  Vector *vec = (Vector *)malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elem)
{
  if (vec->capacity == vec->len)
  {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
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

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '=' || *p == ';')
    {
      Token *token = new_token();
      token->ty = *p;
      token->input = p;
      vec_push(tokens, (void *)token);
      p++;
      continue;
    }

    if (isdigit(*p))
    {
      Token *token = new_token();
      token->ty = TK_NUM;
      token->input = p;
      token->val = strtol(p, &p, 10);
      vec_push(tokens, (void *)token);
      continue;
    }

    if ('a' <= *p && *p <= 'z')
    {
      Token *token = new_token();
      token->ty = TK_IDENT;
      token->input = p;
      vec_push(tokens, (void *)token);
      p++;
      continue;
    }

    error("トークナイズできません： %s\n", p);
  }

  Token *token = new_token();
  token->ty = TK_EOF;
  token->input = p;
  vec_push(tokens, (void *)token);
}

void gen_lval(Node *node)
{
  if (node->ty != ND_IDENT)
    error("代入の左辺値が変数ではありません．");

  int offset = ('z' - node->name + 1) * 8;
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", offset);
  printf("  push rax\n");
}

void gen(Node *node)
{
  if (node->ty == ND_NUM)
  {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_IDENT)
  {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == '=')
  {
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
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
  program();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // プロローグ
  // 変数26個分の領域を確保する
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", 8 * 26);

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++)
  {
    gen(code[i]);

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

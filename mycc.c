#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの方を表す値
enum
{
  TK_NUM = 256, // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
};

// トークンの型
typedef struct
{
  int ty;      // トークンの型
  int val;     // tyがTK_NUMの場合，その数値
  char *input; // トークン文字列(エラーメッセージ用)
} Token;

// トークナイズした結果のトークン列はこの配列に保存する．
// 100個以上のトークンは来ないものとする
Token tokens[100];
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

typedef struct Node
{
  int ty;           // 演算子がND_NUM
  struct Node *lhs; // 左辺
  struct Node *rhs; // 右辺
  int val;          // tyがND_NUMの場合のみ使う
} Node;

Node *new_node(int ty, Node *lhs, Node *rhs)
{
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val)
{
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

int consume(int ty)
{
  if (tokens[pos].ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *add();
Node *mul();
Node *term();

Node *term()
{
  if (consume('('))
  {
    Node *node = add();
    if (!consume(')'))
      error("開きカッコに対応する閉じカッコがありません： %s\n", tokens[pos].input);
    return node;
  }

  if (tokens[pos].ty == TK_NUM)
    return new_node_num(tokens[pos++].val);

  error("数値でも開きカッコでもないトークンです： %s\n", tokens[pos].input);
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

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p)
{
  int i = 0;
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
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p))
    {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    error("トークナイズできません： %s\n", p);
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
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

#include "mycc.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>

Map *keywords;

static void expect(int ty)
{
  Token *t = tokens->data[pos];
  if (t->ty != ty)
    error("%c (%d) expected, but got %c (%d)", ty, ty, t->ty, t->ty);
  pos++;
}

Token *new_token()
{
  Token *token = (Token *)malloc(sizeof(Token));
  return token;
}

static struct
{
  char *name;
  int ty;
} symbols[] = {
    {"==", TK_EQ},
    {"!=", TK_NE},
    {NULL, 0},
};

// Returns true if s1 starts with s2.
bool startswith(char *s1, char *s2)
{
  return !strncmp(s1, s2, strlen(s2));
}

int check_symbols(char *p)
{
  int i;
  for (i = 0; symbols[i].name; i++)
  {
    char *name = symbols[i].name;
    if (!startswith(p, name))
      continue;
    return i;
  }
  return i;
}

// pが指している文字列をトークンに分割してtokensに保存する
void *tokenize(char *p)
{
  debug_out("tokenize start\n");
  keywords = new_map();
  debug_out("create keywords map\n");
  map_put(keywords, "return", (void *)TK_RETURN);
  debug_out("set reserved word\n");
  // require init tokens;
  while (*p)
  {
    // 空白文字をスキップ
    if (isspace(*p))
    {
      p++;
      continue;
    }

    // Multi-letter symbox etc. == !=
    int sym = check_symbols(p);
    if (symbols[sym].name != NULL)
    {
      debug_out("create Multi-letter symbol : %s\n", symbols[sym].name);
      Token *token = new_token();
      token->ty = symbols[sym].ty;
      token->input = p;
      vec_push(tokens, (void *)token);
      p += strlen(symbols[sym].name);
      continue;
    }

    // Single-letter symbol
    if (strchr("+-*/()=;,{}", *p))
    {
      debug_out("create Single-letter symbol : %c\n", *p);
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
      debug_out("create digit token : %d\n", token->val);
      vec_push(tokens, (void *)token);
      continue;
    }

    if (isalpha(*p))
    {
      int var_len = 0;
      while (isalpha(*(p + var_len)))
        var_len++;
      Token *token = new_token();
      token->input = strndup(p, var_len);
      debug_out("create TK_IDENT token : %s\n", token->input);
      token->ty = (intptr_t)map_get(keywords, token->input);
      if (!token->ty)
        token->ty = TK_IDENT;
      vec_push(tokens, (void *)token);
      p += var_len;
      continue;
    }

    error("トークナイズできません： %s\n", p);
  }

  Token *token = new_token();
  token->ty = TK_EOF;
  token->input = p;
  vec_push(tokens, (void *)token);
  debug_out("tokenize done\n");
}

void program()
{
  debug_out("抽象構文木生成 start\n");
  // require variables init
  while (((Token *)tokens->data[pos])->ty != TK_EOF)
    vec_push(code, (void *)function());
  vec_push(code, (void *)NULL);
  debug_out("抽象構文木生成　done\n");
}

Node *function()
{
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FUNC;
  node->args = new_vector();

  Token *token = (Token *)tokens->data[pos];
  if (token->ty != TK_IDENT)
    error("function name expected, but got %s", token->input);
  node->name = token->input;
  pos++;

  expect('(');
  if (!consume(')'))
  {
    vec_push(node->args, term());
    if (((Node *)(node->args->data[node->args->len - 1]))->ty == ND_IDENT)
    {
    }
    while (consume(','))
      vec_push(node->args, term());
    expect(')');
  }
  expect('{');
  node->expr = stmt();
  return node;
}

// 複数の式を抽象構文木にパース
Node *stmt()
{
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ND_COMP_STMT;
  node->stmts = new_vector();

  while (!consume('}'))
  {
    Token *token = tokens->data[pos];
    if (token->ty == TK_EOF)
      return node;

    Node *expr = (Node *)malloc(sizeof(Node));

    if (token->ty == TK_RETURN)
    {
      pos++;
      expr->ty = ND_RETURN;
      expr->expr = assign();
    }
    else
    {
      expr->ty = ND_EXPR_STMT;
      expr->expr = assign();
    }

    vec_push(node->stmts, (void *)expr);
    expect(';');
  }
  return node;
}

// 一つの式
Node *assign()
{
  Node *node = equality();
  for (;;)
  {
    if (consume('='))
      node = new_node('=', node, assign());
    else
      return node;
  }
}

Node *equality()
{
  Node *node = add();
  for (;;)
  {
    if (consume(TK_EQ))
      node = new_node(ND_EQ, node, equality());
    else if (consume(TK_NE))
      node = new_node(ND_NE, node, equality());
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
  {
    Token *token = (Token *)tokens->data[pos++];
    if (!consume('('))
      return new_node_name(token->input);
    Vector *args = new_vector();
    if (consume(')'))
      return new_node_call(token->input, args);
    vec_push(args, (void *)assign());
    while (consume(','))
      vec_push(args, (void *)assign());
    if (consume(')'))
      return new_node_call(token->input, args);
  }

  error("a-zの変数，数値，開きカッコでもないトークンです：%s\n", ((Token *)tokens->data[pos])->input);
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
  node->name = malloc(sizeof(char) * strlen(name));
  strcpy(node->name, name);
  map_put(variables, name, (void *)variables->keys->len);
  return node;
}

Node *new_node_call(char *name, Vector *args)
{
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ND_CALL;
  node->name = malloc(sizeof(char) * strlen(name));
  strcpy(node->name, name);
  node->args = args;
  return node;
}

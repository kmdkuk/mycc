#include "mycc.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

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

// pが指している文字列をトークンに分割してtokensに保存する
void *tokenize(char *p)
{
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
    for (int i = 0; symbols[i].name; i++)
    {
      char *name = symbols[i].name;
      if (!startswith(p, name))
        continue;
      Token *token = new_token();
      token->ty = symbols[i].ty;
      token->input = p;
      vec_push(tokens, (void *)token);
      p += strlen(name);
      break;
    }

    // Single-letter symbol
    if (strchr("+-*/()=;", *p))
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

    if (isalpha(*p))
    {
      int var_len = 0;
      while (isalpha(*(p + var_len)))
        var_len++;
      Token *token = new_token();
      token->ty = TK_IDENT;
      token->input = malloc(sizeof(char) * (var_len + 2));
      strncpy(token->input, p, var_len);
      token->input[var_len] = '\0';
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
}

void program()
{
  // require variables init
  while (((Token *)tokens->data[pos])->ty != TK_EOF)
    vec_push(code, (void *)stmt());
  vec_push(code, (void *)NULL);
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
    return new_node_name(((Token *)tokens->data[pos++])->input);

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

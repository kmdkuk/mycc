#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mycc.h"

static void expect(int ty) {
  /*
  Token *t = tokens->data[pos];
  if (t->ty != ty)
    error("%c (%d) expected, but got %c (%d)", ty, ty, t->ty, t->ty);
  pos++;
  */
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *token = calloc(1, sizeof(Token));
  token->kind = kind;
  token->str = str;
  cur->next = token;
  return token;
}

// Returns true if s1 starts with s2.
bool startswith(char *s1, char *s2) { return strncmp(s1, s2, strlen(s2)) == 0; }

char *starts_with_reserved(char *p) {
  debug_out("\"%s\" starts with reserved keywords?\n", p);
  static char *keywords[] = {"return"};
  int keywords_length = sizeof(keywords) / sizeof(*keywords);
  for (int i = 0; i < keywords_length; i++) {
    if (startswith(p, keywords[i])) return keywords[i];
  }

  static char *ops[] = {"==", "!="};
  int ops_length = sizeof(ops) / sizeof(*ops);
  for (int i = 0; i < ops_length; i++) {
    if (startswith(p, ops[i])) return ops[i];
  }
  return NULL;
}

// pが指している文字列をトークンに分割してtokensに保存する
void *tokenize(char *p) {
  Token tokens = {};
  Token *cur = &tokens;
  debug_out("tokenize start\n");
  // require init tokens;
  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    char *keyword = starts_with_reserved(p);
    if (keyword) {
      debug_out("create reserved token : %s\n", keyword);
      int len = strlen(keyword);
      cur = new_token(TK_RESERVED, cur, keyword);
      p += len;
      continue;
    }

    // Single-letter symbol
    if (strchr("+-*/()=;,{}", *p)) {
      debug_out("create Single-letter symbol : %c\n", *p);
      cur = new_token(TK_RESERVED, cur, &p[0]);
      p++;
      continue;
    }

    if (isdigit(*p)) {
      int num_len = 0;
      while (isdigit(*(p + num_len))) num_len++;
      char *num = strndup(p, num_len);
      debug_out("create digit token : %s\n", num);
      cur = new_token(TK_NUM, cur, num);
      p += num_len;
      continue;
    }

    if (isalpha(*p)) {
      int var_len = 0;
      while (isalpha(*(p + var_len))) var_len++;
      char *ident = strndup(p, var_len);
      debug_out("create TK_IDENT token : %s\n", ident);
      cur = new_token(TK_IDENT, cur, ident);
      p += var_len;
      continue;
    }

    error("トークナイズできません： %s\n", p);
  }

  cur = new_token(TK_EOF, cur, p);
  debug_out("tokenize done\n");
}

void program() {
  debug_out("抽象構文木生成 start\n");
  // require variables init
  while (tokens->kind != TK_EOF) break;  // vec_push(code, (void *)function());
  debug_out("抽象構文木生成　done\n");
}

Node *function() {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_FUNC;
  // node->args = new_vector();
  /*
    Token *token = (Token *)tokens->data[pos];
    if (token->ty != TK_IDENT)
      error("function name expected, but got %s", token->input);
    node->name = token->input;
    pos++;

    expect('(');
    if (!consume(')')) {
      vec_push(node->args, term());
      if (((Node *)(node->args->data[node->args->len - 1]))->ty == ND_IDENT) {
      }
      while (consume(',')) vec_push(node->args, term());
      expect(')');
    }
    expect('{');
    node->expr = stmt();
  */
  return node;
}

// 複数の式を抽象構文木にパース
Node *stmt() {
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ND_COMP_STMT;
  // node->stmts = new_vector();

  /*
  while (!consume('}')) {
    Token *token = tokens->data[pos];
    if (token->ty == TK_EOF) return node;

    Node *expr = (Node *)malloc(sizeof(Node));

    if (token->ty == TK_RETURN) {
      pos++;
      expr->ty = ND_RETURN;
      expr->expr = assign();
    } else {
      expr->ty = ND_EXPR_STMT;
      expr->expr = assign();
    }

    vec_push(node->stmts, (void *)expr);
    expect(';');
  }
  */
  return node;
}

// 一つの式
Node *assign() {
  Node *node = equality();
  for (;;) {
    if (consume('='))
      node = new_node('=', node, assign());
    else
      return node;
  }
}

Node *equality() {
  Node *node = add();
  /*
  for (;;) {
    if (consume(TK_EQ))
      node = new_node(ND_EQ, node, equality());
    else if (consume(TK_NE))
      node = new_node(ND_NE, node, equality());
    else
      return node;
  }*/
  return node;
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume('+'))
      node = new_node('+', node, mul());
    else if (consume('-'))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = term();

  for (;;) {
    if (consume('*'))
      node = new_node('*', node, term());
    else if (consume('/'))
      node = new_node('/', node, term());
    else
      return node;
  }
}

Node *term() {
  if (consume('(')) {
    Node *node = add();
    if (!consume(')'))
      error("開きカッコに対応する閉じカッコがありません： %s\n", tokens->str);
    return node;
  }

  if (tokens->kind == TK_NUM) return new_node_num(tokens->val);

  if (tokens->kind == TK_IDENT) {
    Token *token = tokens;
    if (!consume('(')) return new_node_name(token->str);
    // Vector *args = new_vector();
    if (consume(')')) return new_node_call(token->str, NULL);
    // vec_push(args, (void *)assign());
    while (consume(',')) break;  // vec_push(args, (void *)assign());
    if (consume(')')) return new_node_call(token->str, NULL);
  }

  error("a-zの変数，数値，開きカッコでもないトークンです：%s\n", tokens->str);
}

// (Token *)tokens->data[pos])->ty と 引数が等しい場合posを進める．
int consume(int ty) {
  if (tokens->kind != ty) {
    return 0;
  }
  pos++;
  return 1;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_name(char *name) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = malloc(sizeof(char) * strlen(name));
  strcpy(node->name, name);
  // map_put(variables, name, (void *)variables->keys->len);
  return node;
}

Node *new_node_call(char *name, VarList *args) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ND_CALL;
  node->name = malloc(sizeof(char) * strlen(name));
  strcpy(node->name, name);
  // node->args = args;
  return node;
}

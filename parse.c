#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mycc.h"

static void expect(int ty) {
  if (tok_cur->str[0] != ty)
    error("%c (%d) expected, but got %c (%d)", ty, ty, tok_cur->str[0],
          tok_cur->str[0]);
  tok_cur = tok_cur->next;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *token = calloc(1, sizeof(Token));
  token->kind = kind;
  token->str = str;
  if (kind == TK_NUM) {
    token->val = atoi(str);
  }
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
Token *tokenize(char *p) {
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
  return tokens.next;
}

void push_node(Node *node, Node *item) {
  Node *cur = node;
  while (cur->next != NULL) {
    cur = cur->next;
  }
  cur->next = item;
}

Node *program() {
  Node head = {};
  Node *cur = &head;
  debug_out("create AST\n");
  // require variables init
  while (tok_cur->kind != TK_EOF) {
    cur->next = function();  // vec_push(code, (void *)function());
    cur = cur->next;
  }
  debug_out("created AST\n");
  return head.next;
}

Node *function() {
  Node *func = calloc(1, sizeof(Node));
  func->next = NULL;
  func->ty = ND_FUNC;
  // node->args = new_vector();
  if (tok_cur->kind != TK_IDENT)
    error("function name expected, but got %s", tok_cur->str);
  func->name = tok_cur->str;
  tok_cur = tok_cur->next;
  debug_out("create function node : %s\n", func->name);
  debug_out("get args\n");
  expect('(');
  if (!consume(")")) {
    /*
    vec_push(node->args, term());
    if (((Node *)(node->args->data[node->args->len - 1]))->ty == ND_IDENT) {
    }
    while (consume(",")) vec_push(node->args, term());
    */
    expect(')');
  }
  expect('{');

  Node head = {};
  Node *cur = &head;
  while (!consume("}")) {
    cur->next = stmt();
    cur = cur->next;
  }
  func->expr = head.next;
  debug_out("created function node\n");
  return func;
}

// 複数の式を抽象構文木にパース
Node *stmt() {
  Node *expr = (Node *)malloc(sizeof(Node));

  if (tok_cur->kind == TK_RESERVED && strcmp(tok_cur->str, "return") == 0) {
    debug_out("create return statement\n");
    tok_cur = tok_cur->next;
    expr->ty = ND_RETURN;
    expr->expr = assign();
    debug_out("created return statement\n");
  } else {
    debug_out("create statement\n");
    expr->ty = ND_EXPR_STMT;
    expr->expr = assign();
    debug_out("created statement\n");
  }

  expect(';');
  return expr;
}

// 一つの式
Node *assign() {
  Node *node = equality();
  for (;;) {
    if (consume("="))
      node = new_node('=', node, assign());
    else
      return node;
  }
}

Node *equality() {
  Node *node = add();
  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, equality());
    else if (consume("!="))
      node = new_node(ND_NE, node, equality());
    else
      return node;
  }
  return node;
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node('+', node, mul());
    else if (consume("-"))
      node = new_node('-', node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = term();

  for (;;) {
    if (consume("*"))
      node = new_node('*', node, term());
    else if (consume("/"))
      node = new_node('/', node, term());
    else
      return node;
  }
}

Node *term() {
  if (consume("(")) {
    Node *node = add();
    if (!consume(")"))
      error("開きカッコに対応する閉じカッコがありません： %s\n", tok_cur->str);
    return node;
  }

  if (tok_cur->kind == TK_NUM) {
    int val = tok_cur->val;
    tok_cur = tok_cur->next;
    return new_node_num(val);
  }
  if (tok_cur->kind == TK_IDENT) {
    Token *token = tok_cur;
    tok_cur = tok_cur->next;
    if (consume("(") == 0) return new_node_name(token->str);
    // Vector *args = new_vector();
    if (consume(")")) return new_node_call(token->str, NULL);
    Node *args = malloc(sizeof(Node));
    // vec_push(args, (void *)assign());
    push_node(args, assign());
    while (consume(",")) {
      // vec_push(args, (void *)assign());
      push_node(args, assign());
    }
    if (consume(")")) return new_node_call(token->str, args->next);
  }

  error("a-zの変数，数値，開きカッコでもないトークンです：%s\n", tok_cur->str);
  return NULL;
}

// tok_cur と 引数が等しい場合posを進める．
// 違った場合0
// 等しかったら1進めて1
int consume(char *keyword) {
  if (strncmp(tok_cur->str, keyword, strlen(keyword))) {
    return 0;
  }
  tok_cur = tok_cur->next;
  return 1;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  debug_out("new_node %d (%c)\n", ty, ty);
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  debug_out("new_node_num %d\n", val);
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_name(char *name) {
  debug_out("new_node_name %s\n", name);
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = malloc(sizeof(char) * strlen(name));
  strcpy(node->name, name);
  // Varlistに追加
  add_var(name);
  return node;
}

Node *new_node_call(char *name, Node *args) {
  debug_out("new_node_call %s\n", name);
  Node *node = (Node *)malloc(sizeof(Node));
  node->ty = ND_CALL;
  node->name = malloc(sizeof(char) * strlen(name));
  strcpy(node->name, name);
  node->args = args;
  return node;
}

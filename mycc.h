#ifndef MYCC_H
#define MYCC_H

#include <stdarg.h>

// container.c

void error(char *fmt, ...) __attribute__((format(printf, 1, 2)));
void error_at(char *loc, char *fmt, ...);
void mycc_out(char *fmt, ...) __attribute__((format(printf, 1, 2)));
void debug_out(char *fmt, ...) __attribute__((format(printf, 1, 2)));

// parse.c

// for tokenize

// トークンの方を表す値
typedef enum {
  TK_NUM = 256,  // 整数トークン
  TK_RESERVED,   // 予約語
  TK_IDENT,      // 識別子
  TK_EOF,        // 入力の終わりを表すトークン
} TokenKind;

// トークンの型
typedef struct Token Token;
struct Token {
  TokenKind kind;  // トークンの種類
  Token *next;
  int val;  // tyがTK_NUMの場合，その数値
  char *str;
};

Token *tokenize(char *p);

// for AST

// variables
typedef struct Var Var;
struct Var {
  char *name;
};

typedef struct VarList VarList;
struct VarList {
  VarList *next;
  Var *var;
};

void add_var(char *name);
int search_vars(char *name);
int count_vars();

typedef enum {
  ND_NUM = 256,  // 整数ノードの型
  ND_RETURN,     // Return statement
  ND_COMP_STMT,  // Compound statement
  ND_EXPR_STMT,  // Expressions statement
  ND_IDENT,      // 識別子のノードの型
  ND_EQ,         // ==
  ND_NE,         // !=
  ND_CALL,       // function call
  ND_FUNC,       // funxtion definition
} NodeKind;

typedef struct Node Node;
struct Node {
  int ty;             // 演算子がND_NUM, ND_IDENT
  struct Node *lhs;   // 左辺
  struct Node *rhs;   // 右辺
  int val;            // tyがND_NUMの場合のみ使う
  struct Node *expr;  // "return" or expression statement

  Node *next;

  char *name;  // tyがND_IDENT,ND_CALLの場合に使う
  // Function call
  Node *args;  // 関数の引数
};

Node *program();
Node *function();
Node *stmt();
Node *assign();
Node *equality();
Node *add();
Node *mul();
Node *term();

int consume(int ty);
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_name(char *name);
// Node *new_node_call(char *name, Node *args);
Node *new_node_call(char *name, VarList *args);

// codegen.c

void gen_lval(Node *node);
Node *gen(Node *node);

// main.c
extern char *user_input;
extern Token *tok_cur;

// 複数の関数を保存するための配列
extern Node *code;

// 変数を保存するための配列
extern VarList *variables;

void init();

#endif

#ifndef MYCC_H
#define MYCC_H

#include <stdarg.h>

// container.c

void runtest();
void error(char *fmt, ...) __attribute__((format(printf, 1, 2)));
void mycc_out(char *fmt, ...);
void debug_out(char *fmt, ...);
typedef struct
{
  void **data;
  int capacity;
  int len;
} Vector;
Vector *new_vector();
void vec_push(Vector *vec, void *elem);

typedef struct
{
  Vector *keys;
  Vector *vals;
} Map;
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);

// parse.c

// トークンの方を表す値
enum
{
  TK_NUM = 256, // 整数トークン
  TK_RETURN,    // return
  TK_IDENT,     // 識別子
  TK_EQ,        // ==
  TK_NE,        // !=
  TK_EOF,       // 入力の終わりを表すトークン
};

enum
{
  ND_NUM = 256, // 整数ノードの型
  ND_RETURN,    // Return statement
  ND_COMP_STMT, // Compound statement
  ND_EXPR_STMT, // Expressions statement
  ND_IDENT,     // 識別子のノードの型
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_CALL,      // function call
  ND_FUNC,      // funxtion definition
};

// トークンの型
typedef struct
{
  int ty;      // トークンの型
  int val;     // tyがTK_NUMの場合，その数値
  char *input; // トークン文字列(エラーメッセージ用)
} Token;

typedef struct Node
{
  int ty;            // 演算子がND_NUM, ND_IDENT
  struct Node *lhs;  // 左辺
  struct Node *rhs;  // 右辺
  int val;           // tyがND_NUMの場合のみ使う
  struct Node *expr; // "return" or expression statement
  Vector *stmts;

  char *name; // tyがND_IDENT,ND_CALLの場合に使う

  // Function call
  Vector *args; // 関数の引数
} Node;

void *tokenize(char *p);

void program();
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
Node *new_node_call(char *name, Vector *args);

// codegen.c

void gen_lval(Node *node);
void gen(Node *node);

// main.c

// トークナイズした結果のトークン列はこのVectorに保存する．
extern Vector *tokens;
extern int pos;

// 複数の式を保存するための配列
extern Vector *code;

// 変数を保存するための配列
extern Map *variables;

void init();

#endif

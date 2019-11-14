#include "mycc.h"
#include "stdio.h"

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT) error("代入の左辺値が変数ではありません．");

  // Varlistから検索してくる
  int offset = search_vars(node->name) * 8;
  mycc_out("  mov rax, rbp\n");
  mycc_out("  sub rax, %d\n", offset);
  mycc_out("  push rax\n");
}

Node *gen(Node *node) {
  if (node->ty == ND_FUNC) {
    // プロローグを書く
    mycc_out("%s:\n", node->name);

    // プロローグ
    // 使った個数分の変数領域を確保する
    mycc_out("  push rbp\n");
    mycc_out("  mov rbp, rsp\n");
    mycc_out("  sub rsp, %d\n", 8 * count_vars());
    gen(node->expr);
    return node->next;
  }
  if (node->ty == ND_COMP_STMT) {
    // 関数の中の先頭の式から順にコード生成
    // for (int i = 0; i < node->stmts->len; i++)
    {
      // gen((Node *)node->stmts->data[i]);
    }
    return node->next;
  }
  if (node->ty == ND_EXPR_STMT) {
    gen(node->expr);
    return node->next;
  }
  if (node->ty == ND_RETURN) {
    gen(node->expr);

    // 式の評価結果としてスタックに一つの値が残っている
    // はずなので，スタックが溢れないようにポップ
    mycc_out("  pop rax\n");

    // エピローグを書く
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    mycc_out("  mov rsp, rbp\n");
    mycc_out("  pop rbp\n");
    mycc_out("  ret\n");
    return node->next;
  }

  if (node->ty == ND_NUM) {
    mycc_out("  push %d\n", node->val);
    return node->next;
  }

  if (node->ty == ND_IDENT) {
    gen_lval(node);
    mycc_out("  pop rax\n");
    mycc_out("  mov rax, [rax]\n");
    mycc_out("  push rax\n");
    return node->next;
  }

  if (node->ty == ND_CALL) {
    // 引数を置くx86-64のABIで規定されている順番
    char *arg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    // for (int i = 0; i < node->args->len; i++) {
    // gen((Node *)node->args->data[i]);
    //}

    /*for (int i = 0; i < node->args->len; i++) {
      mycc_out("  pop rax\n");
      mycc_out("  mov %s, rax\n", arg[i]);
    }*/
    mycc_out("  call %s\n", node->name);
    mycc_out("  push rax\n");
    return node->next;
  }

  if (node->ty == '=') {
    gen_lval(node->lhs);
    gen(node->rhs);

    mycc_out("  pop rdi\n");
    mycc_out("  pop rax\n");
    mycc_out("  mov [rax], rdi\n");
    mycc_out("  push rdi\n");
    return node->next;
  }

  // 以下オペレーターの場合
  // 左辺と右辺を展開
  gen(node->lhs);
  gen(node->rhs);

  // それぞれ展開したあとの数値がスタックに積まれているので
  // rdiとtaxにポップ
  mycc_out("  pop rdi\n");
  mycc_out("  pop rax\n");

  // それぞれのオペレーターに応じた計算
  switch (node->ty) {
    case ND_EQ:
      mycc_out("  cmp rdi, rax\n");
      mycc_out("  sete al\n");
      mycc_out("  movzb rax, al\n");
      break;
    case ND_NE:
      mycc_out("  cmp rdi, rax\n");
      mycc_out("  setne al\n");
      mycc_out("  movzb rax, al\n");
      break;
    case '+':
      mycc_out("  add rax, rdi\n");
      break;
    case '-':
      mycc_out("  sub rax, rdi\n");
      break;
    case '*':
      mycc_out("  mul rdi\n");
      break;
    case '/':
      mycc_out("  mov rdx, 0\n");
      mycc_out("  div rdi\n");
  }

  mycc_out("  push rax\n");
  return node->next;
}

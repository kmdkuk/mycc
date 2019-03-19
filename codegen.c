#include "mycc.h"
#include "stdio.h"

void gen_lval(Node *node)
{
  if (node->ty != ND_IDENT)
    error("代入の左辺値が変数ではありません．");

  int offset = ((int)map_get(variables, node->name)) * 8;
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", offset);
  printf("  push rax\n");
}

void gen(Node *node)
{
  if (node->ty == ND_RETURN)
  {
    // エピローグ
    // 最後の式の結果がRAXに残っているのでそれが返り値になる
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
  }

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

  if (node->ty == ND_CALL)
  {
    // 引数を置くx86-64のABIで規定されている順番
    char *arg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    for (int i = 0; i < node->args->len; i++)
    {
      gen((Node *)node->args->data[i]);
    }

    for (int i = 0; i < node->args->len; i++)
    {
      printf("  pop rax\n");
      printf("  mov %s, rax\n", arg[i]);
    }
    printf("  call %s\n", node->name);
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
  case ND_EQ:
    printf("  cmp rdi, rax\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rdi, rax\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
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

  printf("  push rax\n");
}

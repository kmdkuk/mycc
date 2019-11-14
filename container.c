#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mycc.h"

static void expect(int line, int expected, int actual) {
  if (expected == actual) return;
  fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
  exit(1);
}

// エラーを報告するための関数
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void mycc_out(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const char *debug;
  debug = getenv("DEBUG");
  // printf("debug: %s\n", debug);
  if (debug == NULL) {
    vprintf(fmt, ap);
  }
  va_end(ap);
}

void debug_out(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const char *debug;
  debug = getenv("DEBUG");
  if (debug) {
    vprintf(fmt, ap);
  }
  va_end(ap);
}

// varlistに追加する関数
void add_var(char *name) {
  VarList *var_cur = variables;
  while (var_cur->next != NULL) {
    if (strcmp(var_cur->var->name, name) == 0) {
      debug_out("すでに登録されている変数でした。\n");
      return;
    }
    var_cur = var_cur->next;
  }
  Var *var = (Var *)malloc(sizeof(Var));
  var->name = (char *)malloc(sizeof(char) * strlen(name));
  strcpy(var->name, name);
  VarList *new_var = (VarList *)malloc(sizeof(VarList));
  var_cur->var = var;
  var_cur->next = new_var;
  debug_out("変数 %s を登録しました。\n", name);
}

int search_vars(char *name) {
  VarList *var_cur = variables;
  int counter = 1;
  while (strcmp(var_cur->var->name, name) != 0) {
    if (var_cur->next == NULL) return -1;
    counter++;
    var_cur = var_cur->next;
  }
  return counter;
}
int count_vars() {
  VarList *var_cur = variables;
  int counter = 0;
  while (var_cur->next != NULL) {
    counter++;
    var_cur = var_cur->next;
  }
  return counter;
}

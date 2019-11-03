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

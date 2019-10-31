#include "mycc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void expect(int line, int expected, int actual)
{
  if (expected == actual)
    return;
  fprintf(stderr, "%d: %d expected, but got %d\n",
          line, expected, actual);
  exit(1);
}

static void test_vec()
{
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++)
  {
    vec_push(vec, (void *)i);
  }

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (int)vec->data[0]);
  expect(__LINE__, 50, (int)vec->data[50]);
  expect(__LINE__, 99, (int)vec->data[99]);
}

static void test_map()
{
  Map *map = new_map();
  expect(__LINE__, 0, (int)map_get(map, "foo"));

  map_put(map, "foo", (void *)2);
  expect(__LINE__, 2, (int)map_get(map, "foo"));

  map_put(map, "bar", (void *)4);
  expect(__LINE__, 4, (int)map_get(map, "bar"));

  map_put(map, "foo", (void *)6);
  expect(__LINE__, 6, (int)map_get(map, "foo"));
}

void runtest()
{
  test_vec();
  test_map();
  printf("OK\n");
}

// エラーを報告するための関数
void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

Vector *new_vector()
{
  Vector *vec = (Vector *)malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elem)
{
  if (vec->capacity == vec->len)
  {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len] = elem;
  vec->len++;
}

Map *new_map()
{
  debug_out("new_map\n");
  Map *map = malloc(sizeof(Map));
  map->keys = new_vector();
  map->vals = new_vector();
  return map;
}

void map_put(Map *map, char *key, void *val)
{
  debug_out("map_put(map, %s, %s)", key, val);
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void *map_get(Map *map, char *key)
{
  for (int i = map->keys->len - 1; i >= 0; i--)
    if (strcmp(map->keys->data[i], key) == 0)
      return map->vals->data[i];
  return NULL;
}

void mycc_out(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  const char *debug;
  debug = getenv("DEBUG");
  //printf("debug: %s\n", debug);
  if (debug == NULL)
  {
    vprintf(fmt, ap);
  }
  va_end(ap);
}

void debug_out(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  const char *debug;
  debug = getenv("DEBUG");
  if (debug)
  {
    vprintf(fmt, ap);
  }
  va_end(ap);
}

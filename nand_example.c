#ifdef NDEBUG
#undef NDEBUG
#endif

#include "nand.h"
#include "memory_tests.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

/** MAKRA SKRACAJĄCE IMPLEMENTACJĘ TESTÓW **/

// To są możliwe wyniki testu.
#define PASS 0
#define FAIL 1
#define WRONG_TEST 2

// Oblicza liczbę elementów tablicy x.
#define SIZE(x) (sizeof x / sizeof x[0])

#define TEST_ECANCELED(f)                \
  do {                                   \
    errno = 0;                           \
    if ((f) != -1 || errno != ECANCELED) \
      return FAIL;                       \
  } while (0)

#define TEST_PASS(f)                     \
  do {                                   \
    if ((f) != 0)                        \
      return FAIL;                       \
  } while (0)

#define ASSERT(f)                        \
  do {                                   \
    if (!(f))                            \
      return FAIL;                       \
  } while (0)

#define V(code, where) (((unsigned long)code) << (3 * where))

/** WŁAŚCIWE TESTY **/

// To jest przykładowy test udostępniony studentom.
static int example(void) {
  nand_t *g[3];
  ssize_t path_len;
  bool s_in[2], s_out[3];

  g[0] = nand_new(2);
  g[1] = nand_new(2);
  g[2] = nand_new(2);
  assert(g[0]);
  assert(g[1]);
  assert(g[2]);

  TEST_PASS(nand_connect_nand(g[2], g[0], 0));
  TEST_PASS(nand_connect_nand(g[2], g[0], 1));
  TEST_PASS(nand_connect_nand(g[2], g[1], 0));

  TEST_PASS(nand_connect_signal(s_in + 0, g[2], 0));
  TEST_PASS(nand_connect_signal(s_in + 1, g[2], 1));
  TEST_PASS(nand_connect_signal(s_in + 1, g[1], 1));

  ASSERT(0 == nand_fan_out(g[0]));
  ASSERT(0 == nand_fan_out(g[1]));
  ASSERT(3 == nand_fan_out(g[2]));

  int c[3] = {0};
  for (ssize_t i = 0; i < 3; ++i) {
    nand_t *t = nand_output(g[2], i);
    for (int j = 0; j < 3; ++j)
      if (g[j] == t)
        c[j]++;
  }
  ASSERT(c[0] == 2 && c[1] == 1 && c[2] == 0);

  s_in[0] = false, s_in[1] = false;
  path_len = nand_evaluate(g, s_out, 3);
  ASSERT(path_len == 2 && s_out[0] == false && s_out[1] == true && s_out[2] == true);

  s_in[0] = true, s_in[1] = false;
  path_len = nand_evaluate(g, s_out, 3);
  ASSERT(path_len == 2 && s_out[0] == false && s_out[1] == true && s_out[2] == true);

  s_in[0] = false, s_in[1] = true;
  path_len = nand_evaluate(g, s_out, 3);
  ASSERT(path_len == 2 && s_out[0] == false && s_out[1] == false && s_out[2] == true);

  s_in[0] = true, s_in[1] = true;
  path_len = nand_evaluate(g, s_out, 3);
  ASSERT(path_len == 2 && s_out[0] == true && s_out[1] == true && s_out[2] == false);

  nand_delete(g[0]);
  nand_delete(g[1]);
  nand_delete(g[2]);
  return PASS;
}

// Testuje proste przypadki.
static int simple(void) {
  bool s[1];

  nand_t *g0 = nand_new(0);
  nand_t *g1 = nand_new(1);
  nand_t *g2 = nand_new(100);
  assert(g0);
  assert(g1);
  assert(g2);

  TEST_PASS(nand_connect_nand(g1, g1, 0));

  ASSERT(0 == nand_evaluate(&g0, s, 1));
  ASSERT(s[0] == false);

  TEST_ECANCELED(nand_evaluate(&g1, s, 1));
  TEST_ECANCELED(nand_evaluate(&g2, s, 1));

  nand_delete(NULL);
  nand_delete(g0);
  nand_delete(g1);
  nand_delete(g2);
  return PASS;
}

// Testuje reakcję implementacji na niepowodzenie alokacji pamięci.
static unsigned long alloc_fail_test(void) {
  unsigned long visited = 0;
  nand_t *nand1, *nand2;
  ssize_t len;
  int result;
  bool s_in[2], s_out[1];

  errno = 0;
  if ((nand1 = nand_new(2)) != NULL)
    visited |= V(1, 0);
  else if (errno == ENOMEM && (nand1 = nand_new(2)) != NULL)
    visited |= V(2, 0);
  else
    return visited |= V(4, 0);

  errno = 0;
  if ((nand2 = nand_new(1)) != NULL)
    visited |= V(1, 1);
  else if (errno == ENOMEM && (nand2 = nand_new(1)) != NULL)
    visited |= V(2, 1);
  else
    return visited |= V(4, 1);

  errno = 0;
  if ((result = nand_connect_nand(nand2, nand1, 0)) == 0)
    visited |= V(1, 2);
  else if (result == -1 && errno == ENOMEM && nand_connect_nand(nand2, nand1, 0) == 0)
    visited |= V(2, 2);
  else
    return visited |= V(4, 2);

  errno = 0;
  if ((result = nand_connect_signal(&s_in[1], nand1, 1)) == 0)
    visited |= V(1, 3);
  else if (result == -1 && errno == ENOMEM && nand_connect_signal(&s_in[1], nand1, 1) == 0)
    visited |= V(2, 3);
  else
    return visited |= V(4, 3);

  errno = 0;
  if ((result = nand_connect_signal(&s_in[0], nand2, 0)) == 0)
    visited |= V(1, 4);
  else if (result == -1 && errno == ENOMEM && nand_connect_signal(&s_in[0], nand2, 0) == 0)
    visited |= V(2, 4);
  else
    return visited |= V(4, 4);

  s_in[0] = false;
  s_in[1] = false;
  errno = 0;
  if ((len = nand_evaluate(&nand1, s_out, 1)) == 2 && s_out[0] == true)
    visited |= V(1, 5);
  else if (len == -1 && (errno == ENOMEM || errno == ECANCELED) && nand_evaluate(&nand1, s_out, 1) == 2 && s_out[0] == true)
    visited |= V(2, 5);
  else
    return visited |= V(4, 5);

  s_in[0] = false;
  s_in[1] = true;
  errno = 0;
  if ((len = nand_evaluate(&nand1, s_out, 1)) == 2 && s_out[0] == false)
    visited |= V(1, 6);
  else if (len == -1 && (errno == ENOMEM || errno == ECANCELED) && nand_evaluate(&nand1, s_out, 1) == 2 && s_out[0] == false)
    visited |= V(2, 6);
  else
    return visited |= V(4, 6);

  nand_delete(nand1);
  nand_delete(nand2);

  return visited;
}

// Testuje reakcję implementacji na niepowodzenie alokacji pamięci.
static int memory_test(unsigned long (* test_function)(void)) {
  memory_test_data_t *mtd = get_memory_test_data();

  unsigned fail = 0, pass = 0;
  mtd->call_total = 0;
  mtd->fail_counter = 1;
  while (fail < 3 && pass < 3) {
    mtd->call_counter = 0;
    mtd->alloc_counter = 0;
    mtd->free_counter = 0;
    mtd->function_name = NULL;
    unsigned long visited_points = test_function();
    if (mtd->alloc_counter != mtd->free_counter ||
        (visited_points & 0444444444444444444444UL) != 0) {
      fprintf(stderr,
              "fail_counter %u, alloc_counter %u, free_counter %u, "
              "function_name %s, visited_point %lo\n",
              mtd->fail_counter, mtd->alloc_counter, mtd->free_counter,
              mtd->function_name, visited_points);
      ++fail;
    }
    if (mtd->function_name == NULL)
      ++pass;
    else
      pass = 0;
    mtd->fail_counter++;
  }

  return mtd->call_total > 0 && fail == 0 ? PASS : FAIL;
}

// Testuje reakcję implementacji na niepowodzenie alokacji pamięci.
static int memory(void) {
  memory_tests_check();
  return memory_test(alloc_fail_test);
}

/** URUCHAMIANIE TESTÓW **/

typedef struct {
  char const *name;
  int (*function)(void);
} test_list_t;

#define TEST(t) {#t, t}

static const test_list_t test_list[] = {
  TEST(example),
  TEST(simple),
  TEST(memory),
};

static int do_test(int (*function)(void)) {
  int result = function();
  puts("quite long magic string");
  return result;
}

int main(int argc, char *argv[]) {
  if (argc == 2)
    for (size_t i = 0; i < SIZE(test_list); ++i)
      if (strcmp(argv[1], test_list[i].name) == 0)
        return do_test(test_list[i].function);

  fprintf(stderr, "Użycie:\n%s nazwa_testu\n", argv[0]);
  return WRONG_TEST;
}

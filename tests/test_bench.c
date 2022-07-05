#include "../re_api.h"

#include <stdio.h>
#include <time.h>

#define LONG_SUBJECT_LEN (1UL << 28L)

mn_uint32 xorshift32(mn_uint32* state)
{
  mn_uint32 x = *state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  return *state = x;
}

void fill_rand(char* buf, mn_size buf_size)
{
  mn_uint32 rng_state = 5;
  mn_size i;
  mn_uint32* buf_4 = (mn_uint32*)buf;
  for (i = 0; i < buf_size / 4; i++) {
    buf_4[i] = xorshift32(&rng_state);
  }
}

void bench_long_match(void)
{
  re reg;
  clock_t start, end;
  clock_t ms;
  unsigned long mb = (LONG_SUBJECT_LEN / 1048576UL);
  char* subject = malloc(LONG_SUBJECT_LEN);
  fill_rand(subject, LONG_SUBJECT_LEN);
  re_init(&reg, "aaaaaaa$");
  start = clock();
  re_is_match(&reg, subject, LONG_SUBJECT_LEN, 'U');
  end = clock();
  ms = (end - start) / (CLOCKS_PER_SEC / 1000);
  printf(
      "%lu MB / %lu ms (%.2f MB/s)\n", mb, ms,
      (((float)mb) / (float)(ms)) * 1000);
  re_destroy(&reg);
  free(subject);
}

void run_bench(const char* label, void (*bench_func)(clock_t*, clock_t*))
{
  clock_t start;
  clock_t end;
  printf("%s: ", label);
  fflush(stdout);
  bench_func(&start, &end);
  printf("%lu ms\n", (end - start) / (CLOCKS_PER_SEC / 1000));
}

int main()
{
  /* Benchmark search speed */
  bench_long_match();
  return 0;
}

#include "../re_api.h"

#include <stdio.h>
#include <time.h>

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

char pointer_chase(char* buf, mn_size buf_size)
{
  mn_uint32 state = 10;
  static void* pointers[256];
  mn_size i;
  char* end = buf + buf_size;
  void** current = pointers + 0;
  for (i = 0; i < 256; i++) {
    pointers[i] = pointers + i;
  }
  for (i = 0; i < 65536; i++) {
    mn_uint32 a = xorshift32(&state) & 0xFF;
    mn_uint32 b = xorshift32(&state) & 0xFF;
    void* temp = pointers[a];
    pointers[a] = pointers[b];
    pointers[b] = temp;
  }
  while (buf < end) {
    current = *current;
    buf++;
  }
  return (char)(current - pointers);
}

void bench_long_match(void)
{
  re reg;
  clock_t start, end;
  clock_t us;
  re_span grp[2];
  mn_size sub_len = 1;
  int i;
  int max_bits = 29;
  char* subject = malloc(1 << max_bits);
  fill_rand(subject, 1 << max_bits);
  for (i = 28; i < max_bits; i++) {
    sub_len = 1 << i;
    start = clock();
    printf("%i\n", pointer_chase(subject, sub_len));
    end = clock();
    us = (end - start) / (CLOCKS_PER_SEC / 1000000);
    printf(
        "Pointer chase speed:  %lu bytes / %lu ms (%.3f MB/s)\n", sub_len,
        us / 1000, (((float)sub_len / 1048576) / (float)(us)) * 1000000);
    re_init(&reg, "abcdefghijklmnopqrstuvwxyz$");
    start = clock();
    re_is_match(&reg, subject, sub_len, 'B');
    end = clock();
    us = (end - start) / (CLOCKS_PER_SEC / 1000000);
    printf(
        "Bool search speed:  %lu bytes / %lu ms (%.3f MB/s)\n", sub_len,
        us / 1000, (((float)sub_len / 1048576) / (float)(us)) * 1000000);
    start = clock();
    re_match_groups(&reg, subject, sub_len, 'B', 1, grp);
    end = clock();
    us = (end - start) / (CLOCKS_PER_SEC / 1000000);
    printf(
        "Bound search speed: %lu bytes / %lu ms (%.3f MB/s)\n", sub_len,
        us / 1000, (((float)sub_len / 1048576) / (float)(us)) * 1000000);
    start = clock();
    re_match_groups(&reg, subject, sub_len, 'B', 2, grp);
    end = clock();
    us = (end - start) / (CLOCKS_PER_SEC / 1000000);
    printf(
        "Group search speed: %lu bytes / %lu ms (%.3f MB/s)\n", sub_len,
        us / 1000, (((float)sub_len / 1048576) / (float)(us)) * 1000000);
    re_destroy(&reg);
  }
  free(subject);
}

int main(void)
{
  /* Benchmark search speed */
  bench_long_match();
  return 0;
}

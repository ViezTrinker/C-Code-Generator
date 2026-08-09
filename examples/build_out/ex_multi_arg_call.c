/*!
 *\file ex_multi_arg_call.c
 *\brief Feature primer for multi-argument Call blocks.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int32_t add3(int32_t a, int32_t b, int32_t c) { return ((a + b) + c); }

int main(void) {
  /* Multi-arg Call — Arg0..Arg2 into add3 */
  int32_t sum = 0;
  sum = add3(10, 20, 12);
  printf("add3(10, 20, 12) = %d\n", sum);
  fflush(stdout);
  return 0;
}

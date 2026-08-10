/*!
 *\file add_two_integers.c
 *\brief Adds two integers and prints the sum.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  /* Variable 1 */
  int32_t a = 7;
  /* Variable 2 */
  int32_t b = 11;
  /* The sum of those two variables */
  int32_t sum = (a + b);
  printf("a=%d\n", a);
  fflush(stdout);
  printf("b=%d\n", b);
  fflush(stdout);
  printf("sum=%d\n", sum);
  fflush(stdout);
  printf("Press Enter to exit...\n");
  fflush(stdout);
  (void)getchar();
  return 0;
}

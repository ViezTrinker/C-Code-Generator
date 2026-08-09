/*!
 *\file ex_malloc_free.c
 *\brief Feature primer for Malloc and Free with typed pointers.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  /* Malloc/Free — uint8_t* buffer */
  uint8_t *pBuf = (uint8_t *)(malloc((size_t)(8)));
  pBuf[0] = (uint8_t)(42);
  printf("pBuf[0] = %d\n", (int32_t)(pBuf[0]));
  fflush(stdout);
  free(pBuf);
  return 0;
}

/*!
 *\file ex_struct_literal.c
 *\brief Feature primer for Struct Literal designated initializers.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Point
{
   int32_t x;
   int32_t y;
} Point;



int main(void)
{
   /* StructLiteral — designated initializer into a typed Decl */
   Point point = (Point){ .x = 3, .y = 7 };
   printf("point = (%d, %d)\n", point.x, point.y);
   fflush(stdout);
   return 0;
}

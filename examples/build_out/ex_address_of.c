/*!
 *\file ex_address_of.c
 *\brief Feature primer for Address Of and Hero* parameters.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Hero {
  int32_t hp;
  int32_t atk;
} Hero;

void bump_hp(Hero *pHero) {
  pHero->hp = (pHero->hp + 1);
  return;
}

int main(void) {
  /* AddressOf — pass &hero into bump_hp(Hero*) */
  Hero hero = (Hero){.hp = 10, .atk = 4};
  bump_hp(&hero);
  printf("hero.hp after bump = %d\n", hero.hp);
  fflush(stdout);
  return 0;
}

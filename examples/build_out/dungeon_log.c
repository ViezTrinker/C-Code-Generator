/*!
 *\file dungeon_log.c
 *\brief Text adventure showcasing structs, multi-arg calls, files, and heap
 * use.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

typedef struct Hero {
  int32_t hp;
  int32_t maxHp;
  int32_t atk;
  int32_t gold;
  int32_t room;
} Hero;

void print_status(int32_t hp, int32_t maxHp, int32_t atk, int32_t gold,
                  int32_t room) {
  printf("--- Status ---\n");
  fflush(stdout);
  printf("HP: %d / %d\n", hp, maxHp);
  fflush(stdout);
  printf("ATK: %d  Gold: %d  Room: %d\n", atk, gold, room);
  fflush(stdout);
  return;
}

void log_event(int32_t code) {
  int32_t year = 0;
  int32_t month = 0;
  int32_t day = 0;
  int32_t hour = 0;
  int32_t minute = 0;
  int32_t second = 0;
  FILE *fp;
  {
    time_t cgenTimeNow = time(NULL);
    struct tm *pCgenTm = localtime(&cgenTimeNow);
    if (pCgenTm != NULL) {
      year = (int32_t)(pCgenTm->tm_year + 1900);
      month = (int32_t)(pCgenTm->tm_mon + 1);
      day = (int32_t)pCgenTm->tm_mday;
      hour = (int32_t)pCgenTm->tm_hour;
      minute = (int32_t)pCgenTm->tm_min;
      second = (int32_t)pCgenTm->tm_sec;
    }
  }
  fp = fopen("dungeon_log.txt", "a");
  if ((fp != 0)) {
    fprintf(fp, "%d %d-%d-%d %d:%d:%d\n", code, year, month, day, hour, minute,
            second);
    fflush(fp);
    fclose(fp);
    return;
  }
  return;
}

int32_t roll_damage(int32_t atk) {
  int32_t dmg = 0;
  float bonus = 0.0;
  dmg = (atk + (rand() % 3));
  dmg += (dmg / 2);
  bonus = (float)(dmg);
  printf("(atk bonus %.1f)\n", bonus);
  fflush(stdout);
  if ((dmg < 0)) {
    dmg = (-dmg);
  }
  return dmg;
}

void do_combat(Hero *pHero) {
  /* Fight a random enemy until one side falls */
  int32_t enemyHp = 20;
  int32_t hit = 0;
  printf("A monster appears!\n");
  fflush(stdout);
  while (((enemyHp > 0) && (pHero->hp > 0))) {
    hit = roll_damage(pHero->atk);
    printf("You hit for %d!\n", hit);
    fflush(stdout);
    enemyHp -= hit;
    if ((hit == 0)) {
      continue;
    } else {
      if ((enemyHp > 0)) {
        hit = roll_damage(8);
        printf("Monster hits for %d!\n", hit);
        fflush(stdout);
        pHero->hp = (pHero->hp - hit);
        assert((pHero->hp > -100));
      }
    }
  }
  if ((pHero->hp > 0)) {
    printf("You won the fight!\n");
    fflush(stdout);
    log_event(1);
    pHero->gold = (pHero->gold + 5);
    pHero->room = (pHero->room + 1);
  } else {
    printf("You were defeated...\n");
    fflush(stdout);
    log_event(2);
    pHero->hp = 1;
  }
  return;
}

void do_loot(Hero *pHero) {
  int32_t loot[5];
  for (int32_t li = 0; li < 5; ++li) {
    loot[li] = (int32_t)(((li + 1) * 5));
  }
  printf("You find a treasure cache!\n");
  fflush(stdout);
  {
    int32_t cgenShuffleLen = (int32_t)(5);
    for (int32_t cgenShuffleIndex = cgenShuffleLen - 1; cgenShuffleIndex > 0;
         --cgenShuffleIndex) {
      int32_t cgenShuffleSwap = rand() % (cgenShuffleIndex + 1);
      char cgenShuffleTemp = loot[cgenShuffleIndex];
      loot[cgenShuffleIndex] = loot[cgenShuffleSwap];
      loot[cgenShuffleSwap] = cgenShuffleTemp;
    }
  }
  int32_t pick = 0;
  pick = (int32_t)(loot[0]);
  printf("Loot gold: %d\n", pick);
  fflush(stdout);
  pHero->gold = (pHero->gold + pick);
  char flavor = 0;
  flavor = (int32_t)('A' + (rand() % 26));
  log_event(3);
  printf("Flavor rune: %c\n", flavor);
  fflush(stdout);
  return;
}

void show_past_runs(void) {
  FILE *fp;
  char line[64];
  int32_t ok = 0;
  printf("--- Past events (dungeon_log.txt) ---\n");
  fflush(stdout);
  fp = fopen("dungeon_log.txt", "r");
  if ((fp == 0)) {
    printf("No log yet.\n");
    fflush(stdout);
    return;
  } else {
    ok = 1;
    while ((ok == 1)) {
      if (fgets(line, 64, fp) == NULL) {
        line[0] = '\0';
        ok = 0;
      } else {
        ok = 1;
      }
      if ((ok == 1)) {
        printf("%s", line);
        fflush(stdout);
      }
    }
    fclose(fp);
    uint8_t *stamp = (uint8_t *)(malloc((size_t)(4)));
    FILE *fp2;
    fp2 = fopen("dungeon_stamp.bin", "wb");
    stamp[0] = (uint8_t)(42);
    (void)fwrite(stamp, 1, 1, fp2);
    fclose(fp2);
    fp2 = fopen("dungeon_stamp.bin", "rb");
    (void)fread(stamp, 1, 1, fp2);
    fclose(fp2);
    free(stamp);
    return;
  }
  return;
}

int main(void) {
  srand((unsigned int)time(NULL));
  /* Dungeon Log — explore, fight, loot, rest */
  Hero hero = (Hero){.hp = 30, .maxHp = 30, .atk = 6, .gold = 0, .room = 0};
  char name[32];
  char title[32];
  int32_t menu = 0;
  int32_t playing = 1;
  printf("Hero name: ");
  fflush(stdout);
  if (fgets(name, 32, stdin) == NULL) {
    name[0] = '\0';
  }
  strcpy(title, "Hero");
  strncpy(title, name, 16);
  title[(16) - 1] = '\0';
  int32_t nameLen = (int32_t)(strlen(name));
  if ((nameLen == 0)) {
    strcpy(name, "Wanderer");
  }
  printf("Welcome, adventurer!\n");
  fflush(stdout);
  printf("Press Enter to begin...\n");
  fflush(stdout);
  (void)getchar();
  show_past_runs();
  int64_t now = 0;
  now = ((int64_t)time(NULL));
  printf("Session time: %lld\n", now);
  fflush(stdout);
  while ((playing == 1)) {
    printf("\n[1] Explore  [2] Rest  [3] Status  [4] Quit\n");
    fflush(stdout);
    printf("Enter value: ");
    fflush(stdout);
    if (scanf("%d", &menu) != 1) {
      menu = 0;
    }
    switch (menu) {
    case 1: {
      printf("You explore deeper...\n");
      fflush(stdout);
      int32_t roll = 0;
      roll = (rand() % 3);
      if ((roll == 0)) {
        do_combat(&hero);
      } else if ((roll == 1)) {
        do_loot(&hero);
      } else {
        printf("The room is empty.\n");
        fflush(stdout);
        log_event(0);
      }
      break;
    }
    case 2: {
      float healBonus = 0.0;
      printf("You rest...\n");
      fflush(stdout);
#ifdef _WIN32
      Sleep((DWORD)(((1) * 1000)));
#else
      sleep((unsigned int)(1));
#endif
      printf("Enter float: ");
      fflush(stdout);
      if (scanf("%f", &healBonus) != 1) {
        healBonus = 0.0f;
      }
      int32_t heal = 0;
      heal = ((int32_t)(healBonus) + 5);
      hero.hp = (hero.hp + heal);
      if ((hero.hp > hero.maxHp)) {
        hero.hp = hero.maxHp;
      }
      log_event(4);
      break;
    }
    case 3: {
      print_status(hero.hp, hero.maxHp, hero.atk, hero.gold, hero.room);
      printf("Name buffer ready.\n");
      fflush(stdout);
      int32_t cmp = strcmp(name, "Wanderer");
      printf("Name vs Wanderer: %d\n", cmp);
      fflush(stdout);
      int32_t tick = 0;
      tick = 0;
      ++tick;
      --tick;
      int32_t flag = 0;
      flag = ((tick <= 0) || (!(tick >= 1)));
      if (flag) {
        for (int32_t bi = 0; bi < 3; ++bi) {
          break;
        }
      }
      break;
    }
    case 4: {
      playing = 0;
      printf("Farewell!\n");
      fflush(stdout);
      char again = 0;
      printf("Enter character: ");
      fflush(stdout);
      if (scanf(" %c", &again) != 1) {
        again = 0;
      }
      break;
    }
    default: {
      printf("Unknown choice.\n");
      fflush(stdout);
      break;
    }
    }
  }
  return 0;
}

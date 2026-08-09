/*!
 *\file live_clock.c
 *\brief Prints the local date and time once per second.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main(void) {
  int32_t year;
  int32_t month;
  int32_t day;
  int32_t hour;
  int32_t minute;
  int32_t second;
  while (1) {
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
    printf("%04d-%02d-%02d %02d:%02d:%02d\n", year, month, day, hour, minute,
           second);
    fflush(stdout);
#ifdef _WIN32
    Sleep((DWORD)(((1) * 1000)));
#else
    sleep((unsigned int)(1));
#endif
  }
  return 0;
}


#include "averager.hpp"
#include <assert.h>
#include <stdio.h>

#define N_ITEMS(staticArray) sizeof(staticArray)/sizeof(staticArray[0])

#define PUSH(avg, staticArray) \
 do { \
   for (unsigned int i=0; i<N_ITEMS(staticArray); i++) { \
    avg.push(staticArray[i]); \
  } \
 } while (0)

#define PRINT(staticArray) \
 do { \
   fprintf(stderr, "int[] %s", "("); \
   for (unsigned int i=0; i<N_ITEMS(staticArray); i++) { \
    fprintf(stderr, "%d, ", staticArray[i]); \
  } \
  fprintf(stderr, "%s\n", ")"); \
 } while (0)

void test() {
  Averager<9> avg;
  int oneToTen[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
  int fiveHundreds[] = { 100, 100, 100, 100, 100 };
  int centeredFifty[] = { 52, 48, 53, 50, 57, 46, 42, 54 };

  PUSH(avg, fiveHundreds);
  PUSH(avg, fiveHundreds);
  fprintf(stderr, "pushing full of hundreds, should be 100:\n\t%d\n",
          avg.getMedian());
  assert(avg.getMedian() == 100);

  PUSH(avg, oneToTen);
  fprintf(stderr, "pushing full of [1-10], should be 6:\n\t%d\n",
          avg.getMedian());
  assert(avg.getMedian() == 6);

  int oneTwoTree[] = {1, 2, 3};
  int twoZeros[] = {0, 0};
  PUSH(avg, oneTwoTree);
  PUSH(avg, twoZeros);
  fprintf(stderr, "pushing in lower numbers, should lower median:\n\t%d\n",
          avg.getMedian());
  assert(avg.getMedian() == 3);

  PUSH(avg, centeredFifty);
  fprintf(stderr, "values centered around 50:\n\t%d\n",
          avg.getMedian());
  assert(avg.getMedian() == 50);
}


int main() {
  test();
}

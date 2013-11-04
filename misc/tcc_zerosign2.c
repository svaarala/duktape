#include <stdio.h>
#include <math.h>
int main(int argc, char *argv[]) {
  double d = 0.0;
  printf("signbit: %d\n", signbit(d));
  d = -d;
  printf("signbit: %d\n", signbit(d));
}


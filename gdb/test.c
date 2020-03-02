#include <stdio.h>

int func(int n) {
  int sum = 0;
  for(int i = 1; i <= n; i ++)
   sum += i;
  return sum;
} 

int main() {
  int result = 0;
  for(int i = 1; i <= 100; i ++)
    result += i;
  printf("result[1-100] = %d\n", result);
  printf("result[1-250] = %d\n", func(250));
  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* my_strcat1(char *s1, char *s2);

int main(int argc, char const *argv[]) {
  char str1[1000], str2[1000];

  scanf("%s %s", str1, str2);
  char* out = my_strcat1(str1, str2);
  printf("%s\n", out);
  free(out);
  return 0;
}

char* my_strcat1(char *s1, char *s2) {
  int len = 0;
  int i = 0;
  while(s2[i++] != '\0') {
    len++;
  }
  i = 0;
  while(s1[i++] != '\0') {
    len++;
  }
  char* out = malloc(sizeof(char) * len - 1);
  strcpy(out, s1);
  strcpy(&(out[i - 1]), s2);
  return out;
}

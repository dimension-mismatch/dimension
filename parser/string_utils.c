#include "string_utils.h"
#include <stdio.h>

int string_to_int(char* str){
  int num = 0;
  while(*str != '\0'){
    if(*str < '0' || *str > '9'){
      return 0;
    }
    num = num * 10 + (*str - '0');
    str++;
  }
  return num;
}
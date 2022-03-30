#include <stdio.h>  

int fillArray() {
  int i;
  int *a=NULL;

  for (i=0; i < 10; i++) {
    a[i] = -5;
  }

  for (i=0; i < 10; i++) {
    printf("%d ",a[i]);
  }
  printf("\n");

  return 1;

}

void second() {

  int i;
  for (i=8; i< 25; i++) {
    i = i + fillArray();
  }
  printf("%d\n",i);

}


int main() {

  int i;
  for (i=5; i<10; i++) {
    second();
  }
  printf("%d\n",i);

  return 1;
}

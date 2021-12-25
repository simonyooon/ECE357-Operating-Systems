// Simon Yoon 
// compOSps03pr1 - p1prA

#include <stdio.h>

int igv = 0;
int ugv;

int main(void){

int local = 0;
printf("Text: address of binary of program = %p\n", (void *)(&main))

printf("Data : address of initialized global var = %p\n", (void *)(&igv));

printf("BSS : address of uninitialized global var = %p\n", (void *)(&ugv));

printf("Stack : address of local var = %p\n", (void *)(&local));
}
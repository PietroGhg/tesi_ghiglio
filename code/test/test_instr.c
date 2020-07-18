#include <stdlib.h>
#include <stdio.h>

struct s{
    int line;
    struct s* prev;
};

struct s* head;

void init(){
    head = (struct s*)malloc(sizeof(struct s));
    head->line = -1; //line del main
    head->prev = NULL;
}

void clear(){
  struct s* temp1 = head;
  struct s* temp2;
  do{
    temp2 = temp1->prev;
    free(temp1);
    temp1 = temp2;
  }while(temp1 != NULL);
}
    

void push(int line){
    struct s* new_el = (struct s*)malloc(sizeof(struct s));
    new_el->line = line;
    new_el->prev = head;
    head = new_el;
}

void pop(){
    struct s* temp = head;
    head = head->prev;
    free(temp);
}

void dump(){
  struct s* temp = head;
  while(temp != NULL){
    printf("%d ", temp->line); //printf o un altro modo 
    temp = temp->prev;
  }
  printf("\n");
}

  

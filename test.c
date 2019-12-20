#include "mem.h"
#include "common.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAILLE_BUFFER 128000

void afficher_zone(void *adresse, size_t taille, int free)
{
  printf("Zone %s, Adresse : %lu, Taille : %lu\n", free?"libre":"occupee",
         adresse - get_memory_adr(), (unsigned long) taille);
}

void afficher_zone_libre(void *adresse, size_t taille, int free)
{
  if (free)
	  afficher_zone(adresse, taille, 1);
}

void afficher_zone_occupee(void *adresse, size_t taille, int free)
{
  if (!free)
	  afficher_zone(adresse, taille, 0);
}


void test1() {
  printf("--------------  Test 1 --------------\n");
  mem_init(get_memory_adr(),get_memory_size());
  int* tab_adresse = malloc(10*sizeof(int));
  for(int i = 0; i != 10; i++) {
    tab_adresse[i] = -1;
  }

  srand(time(NULL));
  for(int i = 0; i != 10; i++) {
    int taille = rand()%500+1;
    void* ptr = mem_alloc(taille);
    if (ptr != NULL) {
      tab_adresse[i] = (int)(ptr-get_memory_adr());
      for(int i = 0; i != taille; i++) *((char*)ptr+i) = i;
      printf("Memoire allouee en %d\n", (int)(ptr-get_memory_adr()));
    } else printf("Echec de l'allocation\n");
  }

  printf("\n");
  mem_show(afficher_zone);
  printf("\n");

  int nb_suppr = rand()%10;
  for(int i = 0; i != nb_suppr; i++) {
    int random = rand()%10;
    int adr_suppr = tab_adresse[random];
    while (adr_suppr == -1) {
      random = rand()%10;
      adr_suppr = tab_adresse[random];
    }
    tab_adresse[random] = -1;

    mem_free(get_memory_adr() + adr_suppr);
    printf("Memoire liberee en %d\n", adr_suppr);
  }
  printf("\n");
  mem_show(afficher_zone);
  printf("\n            TEST REUSSI !            \n");
  printf("-------------------------------------\n\n");
}

void test2() {
  printf("--------------  Test 2 --------------\n");
  mem_init(get_memory_adr(),get_memory_size());
  srand(time(NULL));
  int* tab_adresse = malloc(100*sizeof(int));
  for(int i = 0; i != 100; i++) {
    tab_adresse[i] = -1;
  }

  for(int i = 0; i != 100; i++) {
    int random = rand()%500;
    void* ptr = mem_alloc(750+random);
    if (ptr != NULL) {
      tab_adresse[i] = (int)(ptr-get_memory_adr());
      printf("Memoire allouee en %d\n", (int)(ptr-get_memory_adr()));
    } else printf("Echec de l'allocation\n");
  }

  printf("\n");
  mem_show(afficher_zone);
  printf("\n");

  for(int i = 0; i != 80; i++) {
    int random = rand()%100;
    int adr_suppr = tab_adresse[random];
    while (adr_suppr == -1) {
      random = rand()%100;
      adr_suppr = tab_adresse[random];
    }
    tab_adresse[random] = -1;

    mem_free(get_memory_adr() + adr_suppr);
    printf("Memoire liberee en %d\n", adr_suppr);
  }

  printf("\n");
  mem_show(afficher_zone);
  printf("\n");

  for(int i = 0; i != 80; i++) {
    int random = rand()%500;
    void* ptr = mem_alloc(750+random);
    if (ptr != NULL) {
      printf("Memoire allouee en %d\n", (int)(ptr-get_memory_adr()));
    } else printf("Echec de l'allocation\n");
  }

  printf("\n");
  mem_show(afficher_zone);
  printf("\n");
  printf("\n            TEST REUSSI !            \n");
  printf("-------------------------------------\n\n");
}

void test3() {
  printf("--------------  Test 3 --------------\n");
  mem_init(get_memory_adr(), get_memory_size());
  void *ptr;
  srand(time(NULL));
  int res[100];
  int valptr[100];

  for(int i = 0; i<100; i++){
    res[i] = rand()%10;
    res[i]++;
    ptr = mem_alloc(res[i]);
    valptr[i] = (int)(ptr-get_memory_adr());
    if(ptr != NULL) printf("Mémoire allouée en : %i \n", (int) (ptr-get_memory_adr()));
  }

  printf("\n");
  //on libère ceux de rang pairs
  for(int i = 0; i<100; i+=2){
    mem_free(get_memory_adr()+valptr[i]);
    printf("Memoire liberee en : %i\n", valptr[i]);
  }

  printf("\n");  
  mem_show(afficher_zone);
  printf("\n");

  //on libère ceux de rang impair
  for(int i = 1; i<100; i+=2){
    mem_free(get_memory_adr()+valptr[i]);
    printf("Memoire liberee en : %i\n", valptr[i]);
  }

  //On ralloue de la meme manière mais apres on va libérer ceux de rang impairs puis ceux de rang pair
  for(int i = 0; i<100; i++){
    res[i] = rand()%10;
    res[i]++;
    ptr = mem_alloc(res[i]);
    valptr[i] = (int)(ptr-get_memory_adr());
    if(ptr != NULL) printf("Mémoire allouée en : %i \n", (int) (ptr-get_memory_adr()));
  }

  printf("\n");
  //on libère ceux de rang impairs
  for(int i = 1; i<100; i+=2){
    mem_free(get_memory_adr()+valptr[i]);
    printf("Memoire liberee en : %i\n", valptr[i]);
  }

  mem_show(afficher_zone);

  //on libère ceux de rang pair
  for(int i = 0; i<100; i+=2){
    mem_free(get_memory_adr()+valptr[i]);
    printf("Memoire liberee en : %i\n", valptr[i]);
  }

  printf("\n");  
  mem_show(afficher_zone);
  printf("\n");  
  printf("\n            TEST REUSSI !            \n");
  printf("-------------------------------------\n\n");
}

int main() {
  for(int i=0; i!=100; i++) {
    test1();
    test2();
    test3();
  }
  printf("Tous les tests sont passes!!\n");
  return 0;
}
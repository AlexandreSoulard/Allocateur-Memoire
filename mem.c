#include "mem.h"
#include "common.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

// constante définie dans gcc seulement
#define ALIGNMENT 8

/* La seule variable globale autorisée
 * On trouve à cette adresse la taille de la zone mémoire
 */
static void* memory_addr;

static inline void *get_system_memory_adr() {
	return memory_addr;
}

static inline size_t get_system_memory_size() {
	return *(size_t*)memory_addr;
}

struct header {
	size_t size;
	struct fb* zl;
};

struct fb {
	size_t size;
	struct fb* next;
};

struct ab {
	size_t size;
};

int allign(int val) {
	return (((val)+(ALIGNMENT-1))&~(ALIGNMENT-1));
}


void mem_init(void* mem, size_t taille) {
    memory_addr = mem;
    *(size_t*)memory_addr = taille;
	assert(mem == get_system_memory_adr()); 	//On teste si on peut accéder à la mémoire à l'adresse voulue si ce n'est pas possible on arrête le programme.
	assert(taille == get_system_memory_size());		//On teste si cette fois on peut allouer la taille demandée pour gérer la mémoire.
	struct header* init_block = (struct header*) (mem);		//On initialise notre header à l'endroit du début de la mémoire à laquelle on a accéder grâce aux tests précédents.
	struct fb* free_block = (struct fb*) (mem + sizeof(struct header));
	free_block->size = taille;		//On initialise la mémoire en libérant tous les blocs disponibles.
	init_block->zl = free_block;
	init_block->size = get_system_memory_size();	//On crée un bloc libre de taille : la taille de la mémoire (ici 128000).
	free_block->size = taille - sizeof(struct header);
	free_block->next = NULL;
	mem_fit(&mem_fit_first);
}

void mem_show(void (*print)(void *, size_t, int)) {
	struct header* header = (struct header*)((char*)get_system_memory_adr());	//On récupère notre header pour avoir accès à toute la zone mémoire.
	struct ab* ab = (struct ab*)((char*)header + sizeof(struct header));
	struct fb* end = (struct fb*)((char*)get_system_memory_adr() + header->size);	//On initialise une structure fb pour pointer vers la fin de la zone mémoire afin de savoir quand est-ce qu'on a fini d'afficher la zone mémoire
	struct fb* list = header->zl;	//On initialise un pointeur afin de savoir ou est le premier bloc libre.
	int taille = 0;		//Taille de la mémoire déjà affichée.
	if (list == NULL) {		//Si il n'y a aucun bloc libre.
		while((struct fb*)ab != end) {		//Tant qu'on a pas atteint la fin de la zone mémoire on continue à afficher les différents blos alloués.
			print(ab, ab->size, 0);		//On affiche un par un les blocs alloués.
			ab = (struct ab*)((char*)ab + ab->size);
		}
		return;
	}
	while (taille != get_system_memory_size() - sizeof(struct header)) {	//Tant que la taille de la mémoire déjà affichée n'est pas égale à toute la taille de la zone mémoire on continue.
		while((struct fb*)((char*)ab) != list) {	//Tant que la zone ou on est n'est pas égale au premier bloc libre : on est sur une zone allouée, on affiche les blocs occupés un par un.
			print(ab, ab->size, 0);
			taille += ab->size;
			ab = (struct ab*)((char*)ab + ab->size);	//On passe à la zone suivante.
		}
		if (list->size != 0) print(list, list->size, 1);	//Si la taille de la zone ou on est est différente de 0 on affiche alors que c'est une zone libre.
		taille += list->size;
		ab = (struct ab*)((char*)list + list->size);
		if (list->next == NULL) list = end;
		else list = list->next;
	}
}

static mem_fit_function_t *mem_fit_fn;

void mem_fit(mem_fit_function_t *f) {
	mem_fit_fn = f;
}

void *mem_alloc(size_t taille) {
	struct header* header = (struct header*)((char*)get_system_memory_adr());	//On récupère toujours le header afin d'avoir accès à la zone mémoire qu'on gère.
	if (taille <= 0 || allign(taille) + sizeof(struct ab)>=header->size) return NULL;	//Si la taille demandée est nulle ou inférieure alors on ne peut pas allouer de zone mémoire.
	if (taille%8 != 0) taille = allign(taille);		//ALIGNEMENT
	struct fb *fb=mem_fit_fn((struct fb*)(header->zl), taille + sizeof(struct ab));
	if (fb == NULL) return NULL;	//Si la zone retournée par la fonction précédente est NULL dans ce cas on ne peut pas allouer car on n'a pas trouvé de zone assez grande pour allouer de la taille demandée.

	struct fb* Premier = header->zl;
	struct fb* Prev = Premier;
	struct fb* After = Premier;
	struct ab* ab = (struct ab*)((char*)fb);
	
	while ((char*)After < (char*)ab) {	//On encadre la future zone libre. Prev représente la zone libre avant la zone à allouer et After représente la zone où l'on veut allouer
		Prev = After;
		After = After->next;
	}

	if (fb->size > taille + sizeof(struct ab)+sizeof(struct fb)-1) {	//Si la taille de la zone renvoyée par mem_fit_fn est assez grande pour ne pas laisser à peine de place pour allouer une autre
		//zone mémoire, alors on crée un nouveau bloc alloué et on réorganise les différents pointeurs pour réduire la taille de la zone libre.
		if ((char*)Prev == (char*)After) {	//Ce cas se présente si la zone à allouée se trouve dans le premier bloc libre de la zone mémoire.
			int size = After->size;
			ab->size = taille + sizeof(struct ab);
			struct fb* new_fb = (struct fb*)((char*)ab + ab->size);
			new_fb->size = size - ab->size;
			new_fb->next = Premier->next;
			header->zl = new_fb;
		} else {	//Si la zone à allouer se trouve dans une autre zone que la première zone libre. Pour une description plus précise voir le compte-rendu.
			int size = After->size;
			ab->size = taille + sizeof(struct ab);
			struct fb* tmp = After->next;
			After = (struct fb*)((char*)ab + ab->size);
			After->size = size - ab->size;
			After->next = tmp;
			Prev->next = After;
		}
	} else {
		if ((char*)Prev == (char*)After) {	//Ce cas se présente si la taille, de la zone où on alloue, est pas assez grande pour laisser une assez grande place pour allouer à la suite.
			//Dans ce cas la zone libre devient entièrement une zone allouée. Pour plus de précision voir le compte-rendu.
			int size = Premier->size;
			Premier = Premier->next;
			ab->size = size;
			header->zl = Premier;
		} else {
			Prev->next = After->next;
			ab->size = After->size;
		}
	}
	return ((char*)ab + sizeof(struct ab));
}

void mem_free(void* mem) {
	if (mem == NULL) return;
	struct header* header = (struct header*)((char*)get_system_memory_adr());	// On récupère le header afin d'avoir accès à la zone mémoire
	struct fb* Premier = header->zl;	// On récupère la 1ère zone libre
	struct fb* Prev = Premier;
	struct fb* After = Premier;
	struct fb* new_fb = (struct fb*)((char*)mem - sizeof(struct ab));	// On met la zone à libérer en tant que zone libre

	if (Premier == NULL) {		// Si la zone mémoire ne contient pas de zone libre, alors la nouvelle zone libre devient la zone libre
		header->zl = new_fb;
		header->zl->next = NULL;
		return;
	}

	while (After->next != NULL && (char*)After < (char*)mem) {		// On encadre la zone libre pour avoir accès aux zones libres avant et après la zone à libérer
		Prev = After;
		After = After->next;
	}
	
	if (((char*)new_fb + new_fb->size) == (char*)After) {		// Si l'adresse de la nouvelle zone libre + sa taille équivaut à l'adresse de After, alors on fusionne ces 2 zones
		new_fb->size = new_fb->size + After->size;
		new_fb->next = After->next;
	} else if (((char*)new_fb + new_fb->size) < (char*)After) {		// Si l'adresse de la nouvelle zone libre + sa taille est inférieure à l'adresse de After, alors la zone libre suivant
		new_fb->next = After;										// la nouvelle zone sera After
	} else {		// Sinon il n'y a pas de zone libre après et alors la nouvelle zone pointe sur NULL
		new_fb->next = NULL;
	}

	if (((char*)Prev + Prev->size) == (char*)new_fb) {			// Si l'adresse de la zone Prev + sa taille équivaut à l'adresse de new_fb, alors on fusionne ces 2 zones
		Prev->size = Prev->size + new_fb->size;
		Prev->next = new_fb->next;
	} else if (((char*)Prev + Prev->size) < (char*)new_fb) {	// Si l'adresse de la zone Prev + sa taille est inférieure à la nouvelle zone, alors Prev->next pointe sur la nouvelle zone
		Prev->next = new_fb;
	} else {					// Sinon la 1ère zone libre de la zone mémoire devient la nouvelle zone libre
		header->zl = new_fb;
	}
}

struct fb* mem_fit_first(struct fb *list, size_t size) {		// On parcourt l'ensemble des zones libres jusqu'à trouver celle qui a une taille supérieure ou égale à la taille de la mémoire
	while (list!=NULL) {										// que l'on veut allouer
		if (list->size >= size) return list;
		else list = list->next;
	}
	return NULL;
}

/* Fonction à faire dans un second temps
 * - utilisée par realloc() dans malloc_stub.c
 * - nécessaire pour remplacer l'allocateur de la libc
 * - donc nécessaire pour 'make test_ls'
 * Lire malloc_stub.c pour comprendre son utilisation
 * (ou en discuter avec l'enseignant)
 */
size_t mem_get_size(void *zone) {
	struct ab* ab = (struct ab*)((char*)zone - sizeof(ab));
	return (size_t)(ab->size - sizeof(struct ab));
}

/* Fonctions facultatives
 * autres stratégies d'allocation
 */
struct fb* mem_fit_best(struct fb* list, size_t size) {
	return NULL;
}

struct fb* mem_fit_worst(struct fb* list, size_t size) {
	return NULL;
}



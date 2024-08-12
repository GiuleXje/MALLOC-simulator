//314CA Pal Roberto-Giulio

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

#define MAX_STRING_SIZE 610

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

//debuging fucntion
void debug(void);

// node of a double linked list
struct node {
	void *address, *start_add;
	size_t size;
	char data[MAX_STRING_SIZE];
	struct node *next, *prev;
};

// double linked list
struct dll_t {
	struct node *head;
	size_t size;
};

//the structure that stores information about the free blocks of memory
struct al_t {
	struct dll_t **list;
	void *heap_start;
	size_t bytes_per_list;
	size_t lists_count;
};

// information required for DUMP_MEMORY
struct info_t {
	size_t total_mem;
	size_t mem_aloc;
	size_t free_mem;
	size_t free_blocks;
	size_t free_calls;
	size_t malloc_calls;
	size_t aloc_blocks;
	size_t fragmentations;
};

//creates a double linked list
struct dll_t *create(void);

//adds a node to a double linked list
void add_node(struct dll_t *list, void *address,
			  size_t block_size, void *start_address);

// searches for a block of size bytes or returns the lowest index that has
// a block of memory bigger than bytes (binary search)
unsigned int give_index(struct al_t *allocator, size_t bytes);

// the struct used to store the memory that has been allocated
struct um_t {
	struct dll_t *list;
};

//function to remove the first element of a linked list
void remove_head(struct dll_t *list);

//function that prints a double's linked list content
void print_list(struct dll_t *list);

//remove an empty list
void shift_left(struct al_t *al, size_t index);

//make room for a new list
void shift_right(struct al_t *al, size_t index);

//fragmentation
void frag_grenade(struct al_t *al, struct um_t *mem_used, size_t bytes,
				  struct info_t *info, size_t index);

//reconstruct the memory to how it was before the malloc calls(bonus)
void
reconstruct(struct al_t *al, void *aux_add, size_t bytes, void *start_add);

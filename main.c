//2024 Pal Roberto-Giulio

#include "func.h"

void useless_read(struct al_t *al, struct um_t *mem_used);

void INIT_HEAP(struct al_t *al)
{
	al->list = malloc((al->lists_count) * sizeof(struct dll_t *));
	DIE(!al->list, "malloc() failed line 8");

	void *curr_add = al->heap_start;
	for (size_t i = 0 ; i < al->lists_count ; i++) {
		size_t block_size = 1 << (i + 3);
		struct dll_t *dll_list = create();
		DIE(!dll_list, "malloc failed line 15");
		size_t n = al->bytes_per_list / block_size;
		for (size_t j = 0 ; j < n ; j++) {
			add_node(dll_list, curr_add, block_size, curr_add);
			curr_add += block_size;
		}
		al->list[i] = dll_list;
	}
}

void MALLOC(struct al_t *al, size_t bytes,
			struct um_t *mem_used, struct info_t *info)
{
	if (al->lists_count == 0) {
		printf("Out of memory\n");
		return;
	}
	if (bytes > al->list[al->lists_count - 1]->head->size) {
		printf("Out of memory\n");
		return;
	}
	info->malloc_calls++;
	size_t index = give_index(al, bytes);
	if (!mem_used->list) {
		mem_used->list = create();
		DIE(!mem_used->list, "malloc() failed! line 40");
	}
	// if we have a block of size equal to bytes
	// and it's no need for fragmentation
	if (bytes == al->list[index]->head->size) {
		add_node(mem_used->list, al->list[index]->head->address,
				 bytes, al->list[index]->head->start_add);
		remove_head(al->list[index]);
		if (!al->list[index]->head) {
			free(al->list[index]);//free the empty block list
			shift_left(al, index);//remove it from the array
		}
	} else { // if fragmentation is needed
		frag_grenade(al, mem_used, bytes, info, index);
	}
	info->mem_aloc += bytes;
	info->free_mem -= bytes;
	info->aloc_blocks++;
}

void FREE_ADDRESS(struct al_t *al, struct um_t *mem_used,
				  void *address, size_t c, struct info_t *info)
{
	size_t bytes;
	void *aux_add, *start_add;
	bool ok = false;
	struct node *curr = mem_used->list->head;
	while (curr && !ok) {//search for the address
		if (curr->address == address) {
			aux_add = address;
			start_add = curr->start_add;
			bytes = curr->size;
			if (curr->prev)
				curr->prev->next = curr->next;
			else
				mem_used->list->head = curr->next;
			if (curr->next)
				curr->next->prev = curr->prev;
			ok = true;
			free(curr);//remove it from the list
		} else {
			curr = curr->next;
		}
	}
	if (!ok) {
		printf("Invalid free\n");
		return;
	}
	info->free_calls++;
	if (!c) {//not bonus
		if (!al->lists_count) {
			al->list = realloc(al->list, (al->lists_count + 1) *
							   sizeof(struct dll_t *));
			DIE(!al->list, "realloc() failed line 92\n");
			al->list[0] = create();
			DIE(!al->list[0], "malloc() failed line 95\n");
			add_node(al->list[0], aux_add, bytes, start_add);
			al->lists_count++;
		} else if (al->list[al->lists_count - 1]->head->size < bytes) {
			al->list = realloc(al->list, (al->lists_count + 1) *
							   sizeof(struct dll_t *));
			DIE(!al->list, "malloc() failed line 100\n");
			al->list[al->lists_count] = create();
			DIE(!al->list[al->lists_count], "malloc() failed line 103");
			add_node(al->list[al->lists_count], aux_add, bytes, start_add);
			al->lists_count++;
		} else {
			size_t index = give_index(al, bytes);
			if (bytes == al->list[index]->head->size) {
				add_node(al->list[index], aux_add, bytes, start_add);
			} else {
				al->list = realloc(al->list, (al->lists_count + 1) *
								   sizeof(struct dll_t *));
				for (size_t i = al->lists_count ; i > index ; i--)
					al->list[i] = al->list[i - 1];
				al->list[index] = create();
				DIE(!al->list[index], "malloc()failed line 116\n");
				add_node(al->list[index], aux_add, bytes, start_add);
				al->lists_count++;
			}
		}
		info->mem_aloc -= bytes;
		info->free_mem += bytes;
	} else {//bonus
		info->mem_aloc -= bytes;
		info->free_mem += bytes;
		reconstruct(al, aux_add, bytes, start_add);
	}
	info->aloc_blocks--;
}

void DUMP_MEMORY(struct al_t *al, struct um_t *mem_used,
				 struct info_t *info)
{
	info->free_blocks = 0;
	//calculating the number of free memory blocks
	for (size_t i = 0 ; i < al->lists_count ; i++)
		info->free_blocks += al->list[i]->size;
	printf("+++++DUMP+++++\n");
	printf("Total memory: %ld bytes\n", info->total_mem);
	printf("Total allocated memory: %ld bytes\n", info->mem_aloc);
	printf("Total free memory: %ld bytes\n", info->free_mem);
	printf("Free blocks: %ld\n", info->free_blocks);
	printf("Number of allocated blocks: %ld\n", info->aloc_blocks);
	printf("Number of malloc calls: %ld\n", info->malloc_calls);
	printf("Number of fragmentations: %ld\n", info->fragmentations);
	printf("Number of free calls: %ld\n", info->free_calls);
	for (size_t i = 0 ; i < al->lists_count ; i++) {
		printf("Blocks with %ld bytes - ", al->list[i]->head->size);
		printf("%ld free block(s) : ", al->list[i]->size);
		print_list(al->list[i]);
	}
	printf("Allocated blocks :");
	if (mem_used->list) {
		struct node *curr = mem_used->list->head;
		while (curr) {
			printf(" (%p - %ld)", curr->address, curr->size);
			curr = curr->next;
		}
	}
	printf("\n-----DUMP-----\n");
}

void DESTROY_HEAP(struct al_t *al, struct um_t *mem_used)
{
	if (al->list) {
		//free data regarding the SGF
		for (size_t i = 0 ; i < al->lists_count ; i++) {
			struct node *curr = al->list[i]->head;
			while (curr) {
				struct node *aux = curr->next;
				free(curr);
				curr = aux;
			}
			free(curr);
			free(al->list[i]);
		}
		free(al->list);
	}
	if (mem_used->list) {//check to see if any memory has been allocated
	//then free it
		struct node *curr = mem_used->list->head;
		while (curr) {
			struct node *aux = curr->next;
			free(curr);
			curr = aux;
		}
		free(mem_used->list);
	}
	exit(0);
}

void WRITE(struct um_t *mem_used, void *address, char *data, size_t bytes,
		   struct al_t *al, struct info_t *info)
{
	if (bytes > strlen(data))
		bytes = strlen(data);
	bool ok = false;
	struct node *curr = mem_used->list->head;
	void *aux_add = address;
	bool first_block = false;
	struct node *write_from = NULL, *write_until = NULL;
	while (curr && !ok) {
		if (aux_add == curr->address && !first_block) {
			if (bytes <= curr->size) {
				write_from = curr;
				write_until = curr;
				ok = true;
			} else {
				write_from = curr;
				aux_add = curr->address + curr->size - 1;
				first_block = true;
			}
		} else if (first_block) {
			if (aux_add + 1 == curr->address) {
				aux_add = curr->address + curr->size - 1;
				if (aux_add >= address + bytes - 1) {
					ok = true;
					write_until = curr;
					break;
				}
			} else {
				ok = false;
				break;
			}
		}
		curr = curr->next;
	}
	if (!write_until) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(al, mem_used, info);
		useless_read(al, mem_used);
	}
	if (ok) {
		while (write_from != write_until) {
			size_t to_copy = write_from->size;
			for (size_t i = 0 ; i < to_copy ; i++)
				write_from->data[i] = data[i];
			char aux[MAX_STRING_SIZE];
			strcpy(aux, data + to_copy);
			strcpy(data, aux);
			write_from = write_from->next;
			bytes -= to_copy;
		}
		for (size_t i = 0 ; i < bytes ; i++)
			write_from->data[i] = data[i];
	} else {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(al, mem_used, info);
		useless_read(al, mem_used);
	}
}

void READ(struct al_t *al, struct um_t *mem_used, size_t bytes,
		  void *address, struct info_t *info)
{
	struct node *read_from = NULL, *read_until = NULL, *curr;
	bool ok = false;
	bool first_block = false;
	curr = mem_used->list->head;
	void *aux_add = address;
	while (curr && !ok) {
		if (aux_add >= curr->address &&
			aux_add <= curr->address + curr->size - 1 && !first_block) {
			if (aux_add + bytes - 1 <= curr->address + curr->size - 1) {
				read_from = curr;
				read_until = curr;
				ok = true;
			} else {
				read_from = curr;
				aux_add = curr->address + curr->size - 1;
				first_block = true;
			}
		} else if (first_block) {
			if (aux_add + 1 == curr->address) {
				aux_add = curr->address + curr->size - 1;
				if (aux_add >= address + bytes - 1) {
					ok = true;
					read_until = curr;
					ok = true;
				} else {
					break;
				}
			}
		}
		curr = curr->next;
	}
	if (!read_until) {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(al, mem_used, info);
		useless_read(al, mem_used);
	}
	if (ok) {
		while (read_from != read_until) {
			size_t to_print = read_from->address + read_from->size - address;
			printf("%s", read_from->data);
			address = read_from->address + read_from->size;
			bytes -= to_print;
			read_from = read_from->next;
		}
		for (size_t i = 0 ; i < bytes ; i++)
			printf("%c", read_from->data[i]);
		printf("\n");
	} else {
		printf("Segmentation fault (core dumped)\n");
		DUMP_MEMORY(al, mem_used, info);
		useless_read(al, mem_used);
	}
}

void useless_read(struct al_t *al, struct um_t *mem_used)
{
	//used to read the rest of the data after a segmentation fault
	void *head_add, *address;
	size_t list_count, bytes_per_list, c, bytes;
	char command[15];
	scanf("%s", command);
	while (strcmp(command, "DESTROY_HEAP") != 0) {
		if (strcmp(command, "INIT_HEAP") == 0) {
			scanf("%p%ld%ld%ld", &head_add, &list_count, &bytes_per_list, &c);
		} else if (strcmp(command, "MALLOC") == 0) {
			scanf("%ld", &bytes);
		} else if (strcmp(command, "FREE") == 0) {
			scanf("%p", &address);
		} else if (strcmp(command, "READ") == 0) {
			scanf("%p%ld", &address, &bytes);
		} else if (strcmp(command, "WRITE") == 0) {
			char data[MAX_STRING_SIZE];
			fgets(data, MAX_STRING_SIZE, stdin);
		}
		scanf("%s", command);
	}
	DESTROY_HEAP(al, mem_used);
}

int main(void)
{
	void *head_add, *address;
	char command[15];
	size_t list_count, bytes_per_list;
	size_t c;
	size_t bytes;
	struct al_t al;
	struct um_t mem_used;
	mem_used.list = NULL;
	struct info_t info;

	while (69) {
		scanf("%s", command);
		if (strcmp(command, "INIT_HEAP") == 0) {
			scanf("%p%ld%ld%ld", &head_add, &list_count, &bytes_per_list, &c);
			info.total_mem = list_count * bytes_per_list;
			al.heap_start = head_add;
			al.lists_count = list_count;
			al.bytes_per_list = bytes_per_list;
			INIT_HEAP(&al);
			info.free_mem = info.total_mem;
			info.free_calls = 0;
			info.fragmentations = 0;
			info.aloc_blocks = 0;
			info.malloc_calls = 0;
			info.mem_aloc = 0;
			info.free_blocks = 0;
			for (size_t i = 0 ; i < list_count ; i++)
				info.free_blocks += list_count / (1 << (i + 3));
		} else if (strcmp(command, "MALLOC") == 0) {
			scanf("%ld", &bytes);
			MALLOC(&al, bytes, &mem_used, &info);
		} else if (strcmp(command, "FREE") == 0) {
			scanf("%p", &address);
			FREE_ADDRESS(&al, &mem_used, address, c, &info);
		} else if (strcmp(command, "READ") == 0) {
			scanf("%p%ld", &address, &bytes);
			READ(&al, &mem_used, bytes, address, &info);
		} else if (strcmp(command, "WRITE") == 0) {
			char aux[MAX_STRING_SIZE], data[MAX_STRING_SIZE];
			scanf("%p", &address);
			scanf("%s", aux);
			strcpy(data, aux + 1);
			//read until \n is encountered
			scanf("%[^\n]", data + strlen(data));
			getchar();//read the \n
			size_t k = strlen(data);
			for (int i = k - 1 ; i >= 0 ; i--) {
				if (!isdigit(data[i])) {
					//get the number of bytes
					bytes = atoi(data + i + 1);
					//separating the string from the number of bytes
					//and getting rid of the last '"'
					data[i - 1] = '\0';
					break;
				}
			}
			WRITE(&mem_used, address, data, bytes, &al, &info);
		} else if (strcmp(command, "DUMP_MEMORY") == 0) {
			DUMP_MEMORY(&al, &mem_used, &info);
		} else if (strcmp(command, "DESTROY_HEAP") == 0) {
			DESTROY_HEAP(&al, &mem_used);
		}
	}
	return 0;
}

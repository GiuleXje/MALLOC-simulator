//2024 Pal Roberto-Giulio

#include "func.h"

void debug(void)
{
	printf("\naici\n");
}

struct dll_t *create(void)
{
	struct dll_t *list = (struct dll_t *)malloc(sizeof(struct dll_t));
	if (!list)
		return NULL;
	list->head = NULL;
	list->size = 0;
	return list;
}

void add_node(struct dll_t *list, void *add, size_t block_size,
			  void *start_address)
{
	struct node *new_node = (struct node *)malloc(sizeof(struct node));
	DIE(!new_node, "malloc() failed on a new node");
	new_node->address = add;
	new_node->size = block_size;
	new_node->start_add = start_address;
	new_node->next = NULL;
	new_node->prev = NULL;
	for (int i = 0 ; i < MAX_STRING_SIZE ; i++)
		new_node->data[i] = '\0';
	list->size++;
	if (!list->head) {
		list->head = new_node;
		return;
	}
	if (new_node->address < list->head->address) {
		new_node->next = list->head;
		list->head->prev = new_node;
		new_node->prev = NULL;
		list->head = new_node;
		return;
	}
	struct node *curr, *ant;
	ant = list->head;
	curr = ant->next;
	while (curr) {
		if (new_node->address < curr->address)
			break;
		ant = curr;
		curr = curr->next;
	}
	ant->next = new_node;
	new_node->prev = ant;
	new_node->next = curr;
	if (curr)
		curr->prev = new_node;
}

unsigned int give_index(struct al_t *allocator, size_t bytes)
{
	int left = 0, right = allocator->lists_count - 1;
	if (left >= right)
		return 0;
	int mid;
	while (left <= right) {
		mid = (left + right) / 2;
		if (allocator->list[mid]->head->size == bytes)
			return mid;
		else if (allocator->list[mid]->head->size < bytes)
			left = mid + 1;
		else
			right = mid - 1;
	}
	return ((left < 0) ? 0 : left);
}

void remove_head(struct dll_t *list)
{
	struct node *curr = list->head;
	list->size--;
	struct node *aux = list->head->next;
	if (aux)
		aux->prev = NULL;
	list->head = aux;
	free(curr);
}

void print_list(struct dll_t *list)
{
	struct node *curr = list->head;
	while (curr) {
		printf("%p", curr->address);
		if (curr->next)
			printf(" ");
		curr = curr->next;
	}
	printf("\n");
}

void shift_left(struct al_t *al, size_t index)
{
	for (size_t i = index + 1 ; i < al->lists_count ; i++)
		al->list[i - 1] = al->list[i];
	al->lists_count--;
}

void shift_right(struct al_t *al, size_t index1)
{
	al->lists_count++;
	for (size_t i = al->lists_count - 1 ; i > index1 ; i--)
		al->list[i] = al->list[i - 1];
}

void frag_grenade(struct al_t *al, struct um_t *mem_used, size_t bytes,
				  struct info_t *info, size_t index)
{
	info->fragmentations++;
	add_node(mem_used->list, al->list[index]->head->address, bytes,
			 al->list[index]->head->start_add);
	void *aux_add = al->list[index]->head->address + bytes;
	void *aux_start = al->list[index]->head->start_add;
	size_t frag_block_size = al->list[index]->head->size - bytes;
	remove_head(al->list[index]);
	if (!al->list[index]->head) {
		free(al->list[index]);//free the empty block list
		shift_left(al, index);//remove it from the array
	}
	if (!al->lists_count) {
		al->list = realloc(al->list, (al->lists_count + 1) *
						   sizeof(struct dll_t *));
		DIE(!al->list, "realloc() failed\n");
		al->list[al->lists_count] = create();
		DIE(!al->list[al->lists_count], "malloc failed\n");
		add_node(al->list[al->lists_count], aux_add,
				 frag_block_size, aux_start);
		al->lists_count++;
	} else if (al->list[al->lists_count - 1]->head->size <
			   frag_block_size) {
		al->list = realloc(al->list, (al->lists_count + 1) *
						   sizeof(struct dll_t *));
		DIE(!al->list, "realloc() failed\n");
		al->list[al->lists_count] = create();
		DIE(!al->list[al->lists_count], "malloc failed\n");
		add_node(al->list[al->lists_count], aux_add,
				 frag_block_size, aux_start);
		al->lists_count++;
	} else {
		size_t index1 = give_index(al, frag_block_size);
		al->list = realloc(al->list, (al->lists_count + 1) *
						   sizeof(struct dll_t *));
		DIE(!al->list, "realloc() failed\n");
		if (!al->lists_count) {
			al->list[0] = create();
			DIE(!al->list[0], "malloc() failed\n");
			add_node(al->list[0], aux_add, bytes, aux_start);
			al->lists_count++;
		} else if (frag_block_size == al->list[index1]->head->size) {
			add_node(al->list[index1], aux_add, frag_block_size,
					 aux_start);
		} else {
			shift_right(al, index1);//make room for the new list
			al->list[index1] = create();
			DIE(!al->list[index], "malloc() failed line 94\n");
			add_node(al->list[index1], aux_add, frag_block_size,
					 aux_start);
		}
	}
}

void
reconstruct(struct al_t *al, void *aux_add, size_t bytes, void *start_add)
{
	if (!al->lists_count) {
		al->list = realloc(al->list, (al->lists_count + 1) *
						   sizeof(struct dll_t *));
		DIE(!al->list, "realloc() failed\n");
		al->list[0] = create();
		DIE(!al->list[0], "malloc() failed\n");
		add_node(al->list[0], aux_add, bytes, start_add);
		al->lists_count++;
		return;
	}
	bool ok = false;
	for (size_t i = 0 ; i < al->lists_count && !ok ; i++) {
		struct node *curr = al->list[i]->head;
		while (curr && !ok) {
			if (curr->start_add == start_add) {
				ok = true;
				bytes += curr->size;
				if (curr->next)
					curr->next->prev = curr->prev;
				if (curr->prev)
					curr->prev->next = curr->next;
				else
					al->list[i]->head = curr->next;
				free(curr);
			} else {
				curr = curr->next;
			}
		}
		if (!al->list[i]->head) {
			free(al->list[i]);
			shift_left(al, i);
		}
	}
	if (!al->lists_count) {
		al->list = realloc(al->list, (al->lists_count + 1) *
						   sizeof(struct dll_t *));
		DIE(!al->list, "realloc() failed\n");
		al->list[0] = create();
		DIE(!al->list[0], "malloc() failed\n");
		add_node(al->list[0], aux_add, bytes, start_add);
		al->lists_count++;
	} else if (bytes > al->list[al->lists_count - 1]->head->size) {
		al->list = realloc(al->list, (al->lists_count + 1) *
						   sizeof(struct dll_t *));
		DIE(!al->list, "realloc failed\n");
		al->list[al->lists_count] = create();
		DIE(!al->list[al->lists_count], "malloc failed\n");
		al->lists_count++;
	} else {
		size_t index = give_index(al, bytes);
		if (al->list[index]->head->size == bytes) {
			add_node(al->list[index], aux_add, bytes, start_add);
		} else {
			al->list = realloc(al->list, (al->lists_count + 1) *
							   sizeof(struct dll_t *));
			DIE(!al->list, "realloc() failed\n");
			shift_right(al, index);
			al->list[index] = create();
			DIE(!al->list[index], "malloc() failed\n");
			add_node(al->list[index], aux_add, bytes, start_add);
		}
	}
}

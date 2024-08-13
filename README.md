# MALLOC-simulator

  This code will handle memory allocations for a system in C.
In this program we will handle our available memory using segregated free lists.
Also, I chose a similar container to store the used memory.(for simplicity)
  The main commands that can be applied to this program
are as follows:
1) INIT_HEAP <starting_adress> <number_of_lists> <bytes_per_list> <reconstruction_type>

 It creates the segregated free lists. I handled cases where bytes_per_list is a power of 2,
greater or equal to 8. The starting adress is the location of the first block of memory to be
allocated, for example if the starting adress is 0x1, the second block, for a list of 8 bytes
will be at 0x1 + 8 bytes and so on for the rest untill we fill our bytes required.
  Number_of_lists indicates the number of different memory blocks that will be saved in our segregated free lists.
Let's say that number_of_lists = 3, we will have 3 lists, the first with blocks of memory of size 8, the second
with size 16, and the last with size 32.
  Bytes_per_list indicates the size available for each list. Let's say that our size is 128, it means that in the
list that has blocks of size 8 we will have 16 different blocks, in the list with blocks of size 16 we will have 8
different blocks and so on.
  Reconstruction_type will be relevant if we choose to free the memory later, it can be either 0 or 1, we will discuss
it when describing FREE.

 A relevant ilustration for want this function desires to achieve:
![image](https://github.com/user-attachments/assets/fd1df3a2-9605-4a4f-a78b-ded24565baa6)


2) MALLOC <number_of_bytes>

  It searches for the first memory block available, of size at least equal to number_of_bytes.
If there are more memory blocks that respect this, we choose the one with the lowest address.
If there are only blocks that are are bigger than the one we want to allocate, we fragment the block.
For example we want to allocate a block of size 6, and the first available block is of size 16, we will
save the block of size 6 in a separate container that stores allocated memory, and we add to our initial
segregated free lists, a new list of blocks of size 10.
  If there are no blocks available, the program will print the message: "Out of memory".

3) FREE <address>
  Frees the block at the <address>. If starting with address, there was no block of memory already
allocated, we print "Segmentation fault(core dumpled)" and we dump the memory using DUMP_MEMORY(will be discussed later).
If found, the block is deleted from the container that stores allocated memory and it is restored back into the segregated
free lists, in the list containing blocks equal to it's size, and in the order of their addresses.
  Now we have to talk about the <reconstruction_type> from INIT_HEAP. If reconstruction type is set to 0,
we will just restore the block back into the segregated free list. If reconstruction type is set to 1, the block that is beeing
freed will be stored back into it's original place from segregated free list. This case is not completely finished yet, so i would
recommend using only reconstruction_type = 0.
  Free 0x0 is the same to Free(NULL) form C, and it will have no effect.

4) READ <address> <number_of_bytes>
  If starting with address there were no blocks allocated, "Segmentation fault(core dumped)" and we dump the memory. If at least a byte form [address,address + number_of_bytes)
was not allocated, "Segmentation fault(core dumped)" and dump the memory. Otherwise, we read what was written at the current address.

5) Write <address> <data> <number_of_bytes>
  Writes the first number_of_bytes from data to the given address. If at least a byte from [address, address + number_of_bytes) was not allocated,
"Segementation fault(core dumped)" and dump the memory. Else, we copy the first number_of_bytes bytes from data to the current address.

6) DUMP_MEMORY
   Prints all the free memory zones along with the allocated memory zones:
   ![image](https://github.com/user-attachments/assets/5ce8bcda-1c9c-411e-a4bb-edd948424d9e)

7) DESTROY_HEAP
   The endpoit of our program, we free all the used memory and end the execution.


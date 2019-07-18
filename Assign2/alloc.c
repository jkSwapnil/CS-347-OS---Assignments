#include "alloc.h"

/* Code to allocate page of 4KB size with mmap() call and
* initialization of other necessary data structures.
* return 0 on success and 1 on failure (e.g if mmap() fails)
*/
struct mstatus *start=NULL;
struct mstatus *end=NULL;
char * page_pointer;

int init()
{
	page_pointer = mmap(0, PAGESIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
	if(page_pointer==MAP_FAILED){
		return -1;
	}
	//printf("%s", page_pointer);
	start = (struct mstatus*)malloc(sizeof(struct mstatus));
	start-> size = PAGESIZE;
	start-> free = 1;
	start-> start_addr=page_pointer;
	end = start;
	return 0;
}

/* optional cleanup with munmap() call
* return 0 on success and 1 on failure (if munmap() fails)
*/
int cleanup()
{	
	//deleting the structure information about the memory
	while(start!=end){
		struct mstatus* buffer= start;
		start=buffer->next;
		free(buffer);
	}
	free(start);
	int status = munmap(page_pointer, PAGESIZE);
	if(status==-1)
		return -1;
	else 
		return 0;
}

/* Function to allocate memory of given size
* argument: bufSize - size of the buffer
* return value: on success - returns pointer to starting address of allocated memory
*               on failure (not able to allocate) - returns NULL
*/
char *alloc(int bufSize)
{
	// write your code below
	if(bufSize%MINALLOC!=0)
		return NULL;

	int satisfied=0;
	struct mstatus * buffer = start;
	while(satisfied!=1){
		if(buffer->size >= bufSize && buffer->free==1){
			satisfied=1;
			if(buffer->size > bufSize){
			struct mstatus * temp = (struct mstatus*)malloc(sizeof(struct mstatus));
			temp->size = buffer-> size - bufSize;
			temp-> free=1;
			temp-> start_addr= buffer->start_addr + bufSize;
			buffer-> size= bufSize;
			buffer-> free=0;
			temp-> next = buffer->next;
			buffer-> next= temp;
			return buffer->start_addr;
			}
			else{
				buffer-> free=0;
				return buffer->start_addr;
			}
		}
		else{
			buffer=buffer->next;
		}
		if(buffer==end)
			return NULL;
	} 

}


/* Function to free the memory
* argument: takes the starting address of an allocated buffer
*/
void dealloc(char *memAddr)
{
	struct mstatus * buffer = start;
	while(1){
		if (memAddr== buffer->start_addr){
			buffer->free=1;
			return;
		}
		else{
			buffer= buffer->next;
		}
		if(buffer==end){
			printf("%s\n", "Not possible!!!");
			return;
		}
	}

}
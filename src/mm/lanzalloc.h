#ifndef __LANZALLOC_H__
#define __LANZALLOC_H__

struct lanzalloc;

// Initialize lanzalloc
/*
	parameters:
	void* memory: The memory head address you want to use for lanzalloc.
	unsigned int size: The size of the memory.
	unsigned int maxunit: The maximum number of memory units that lanzalloc can maintain.
	return:
	Return a lanzalloc pointer if initialize successfully, or return NULL.
*/
struct lanzalloc* lanzalloc_initialize(void* memory, unsigned int size, unsigned int maxunit);

// Alloc
/*
	parameters:
	struct lanzalloc* lanzalloc: Memory will be allocated from it.
	unsigned int size: The size you want to allocate.
	return:
	Return a pointer if alloc successfully, or return NULL.
*/
void* lanzalloc_alloc(struct lanzalloc* lanzalloc, unsigned int size);


// Realloc
/*
	parameters:
	struct lanzalloc* lanzalloc: Memory will be reallocated from it.
	void* address: Memory to be operated on.
	unsigned int newSize: The newsize you want to reallocate.
	return:
	Return a pointer if alloc successfully, or return NULL.
*/
void* lanzalloc_realloc(struct lanzalloc* lanzalloc, void* address, unsigned int newSize);

// Free
/*
	parameters:
	struct lanzalloc* lanzalloc: Memory will be free from it.
	void* address: Memory to be operated on.
*/
void lanzalloc_free(struct lanzalloc* lanzalloc, void* address);

// Free memory
/*
	return:
	Return the size of free memory.
*/
unsigned int lanzalloc_freemem(struct lanzalloc* lanzalloc);

// Used memory
/*
	return:
	Return the size of used memory.
*/
unsigned int lanzalloc_usedmem(struct lanzalloc* lanzalloc);

#endif

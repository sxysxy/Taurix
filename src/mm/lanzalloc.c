#include "lanzalloc.h"

struct unit {
	unsigned int size; // If size == 0, it is a unused unit.
	void* address; // If address == 0, it is a free unit.
};

struct lanzalloc {
	unsigned int unitMax;
	void* pool;
	unsigned int poolSize;
	struct unit* units;
};

// Defragment
static int lanzalloc_defragment(struct lanzalloc* lanzalloc) {
	int i, j;
	for (i = 0; i < lanzalloc->unitMax; i++) {
		struct unit* unit = lanzalloc->units + i;
		if (unit->size) {
			void* tail = (void*)((char*)unit->address + unit->size);
			for (j = 0; j < lanzalloc->unitMax; j++) {
				struct unit* unit2 = lanzalloc->units + j;
				if (unit != unit2 && unit2->size && tail == unit2->address) {
					unit->size += unit2->size;
					unit2->size = 0;
					unit2->address = 0;
					return 1;
				}
			}
		}
	}
	return 0;
}

static void lanzalloc_defragment_full_cycle(struct lanzalloc* lanzalloc) {
	while(lanzalloc_defragment(lanzalloc));
}

// Initialize lanzalloc
struct lanzalloc* lanzalloc_initialize(void* memory, unsigned int size, unsigned int maxunit) {
#ifdef __cplusplus
	struct lanzalloc* alloc = reinterpret_cast<struct lanzalloc*>(memory);
#else 
	struct lanzalloc* alloc = memory;
#endif
	do {
		if (size < (sizeof(struct lanzalloc) + sizeof(struct unit) * maxunit)) break;
		alloc->unitMax = maxunit;
#ifdef __cplusplus
		alloc->units = (void*)((char*)memory + sizeof(struct lanzalloc));
		alloc->pool = (void*)((char*)memory + sizeof(struct lanzalloc) + sizeof(struct unit) * maxunit);
#else 
		alloc->units = (void*)((char*)memory + sizeof(struct lanzalloc));
		alloc->pool = (void*)((char*)memory + sizeof(struct lanzalloc) + sizeof(struct unit) * maxunit);
#endif
		alloc->poolSize = size - (sizeof(struct lanzalloc) + sizeof(struct unit) * maxunit);
		int i = 1;
		for (; i < maxunit; i++) {
			alloc->units[i].size = 0;
			alloc->units[i].address = 0;
		}
		alloc->units[0].size = alloc->poolSize;
		alloc->units[0].address = alloc->pool;
		return alloc;
	} while(0);
	return 0;
}

// Alloc
void* lanzalloc_alloc(struct lanzalloc* alloc, unsigned int size) {
	int i;
	do {
		for (i = 0; i < alloc->unitMax; i++) {
			struct unit* unit = alloc->units + i;
			// Find a free unit.
			if (unit->size) {
				if (unit->size >= size + sizeof(unsigned int)) {
					// Write size data.
					*((unsigned int*)unit->address) = size;
					// Return a address after size data.
					char* result = (char*)unit->address + sizeof(unsigned int);
					// Recover memory.
					if (unit->size > size + sizeof(unsigned int)) {
						unit->size -= size + sizeof(unsigned int);
						unit->address = result + size;
					} else {
						unit->size = 0;
						unit->address = 0;
					}
					// Return pointer.
					return (void*)result;
				}
			}
		}
	} while(lanzalloc_defragment(alloc));
	return 0;
}

// Realloc
void* lanzalloc_realloc(struct lanzalloc* lanzalloc, void* address, unsigned int newSize) {
	if (address == 0) {
		return lanzalloc_alloc(lanzalloc, newSize);
	}
	if (newSize == 0) {
		lanzalloc_free(lanzalloc, address);
		return 0;
	}
	int i;
	unsigned int oldSize = *(unsigned int*)((char*)address - sizeof(unsigned int));
	int delta = newSize - oldSize;
	if (delta > 0) {
		void* tail = (void*)((char*)address + oldSize);
		for (i = 0; i < lanzalloc->unitMax; i++) {
			struct unit* unit = lanzalloc->units + i;
			// Find a free unit.
			if (unit->size >= delta && tail == unit->address) {
				unit->size -= delta;
				unit->address = (void*)((char*)unit->address + delta);
				*(unsigned int*)((char*)address - sizeof(unsigned int)) = newSize;
				return address;
			}
		}
		return 0;
	} else if (delta < 0) {
		do {
			for (i = 0; i < lanzalloc->unitMax; i++) {
				struct unit* unit = lanzalloc->units + i;
				// Find a empty unit.
				if (unit->size) continue;
				unit->size = -delta;
				unit->address = (void*)((char*)address + newSize);
				*(unsigned int*)((char*)address - sizeof(unsigned int)) = newSize;
				return address;
			}
		} while(lanzalloc_defragment(lanzalloc));
		return 0;
	} else {
		return address;
	}
	return 0;
}

// Free
void lanzalloc_free(struct lanzalloc* lanzalloc, void* address) {
	if (!address) return;
	int i;
	do {
		for (i = 0; i < lanzalloc->unitMax; i++) {
			struct unit* unit = lanzalloc->units + i;
			// Find a empty unit.
			if (unit->size) continue;
			unit->size = *(unsigned int*)((char*)address - sizeof(unsigned int)) + sizeof(unsigned int);
			unit->address = (char*)address - sizeof(unsigned int);
			return;
		}
	} while(lanzalloc_defragment(lanzalloc));
}

// Free memory
unsigned int lanzalloc_freemem(struct lanzalloc* lanzalloc) {
	unsigned int result = 0;
	int i;
	for (i = 0; i < lanzalloc->unitMax; i++) {
		result += lanzalloc->units[i].size;
	}
	return result;
}

// Used memory
unsigned int lanzalloc_usedmem(struct lanzalloc* lanzalloc) {
	return lanzalloc->poolSize - lanzalloc_freemem(lanzalloc);
}

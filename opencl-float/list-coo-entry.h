
#ifndef __ARRAY_LIST_COO_ENTRY
#define __ARRAY_LIST_COO_ENTRY



// Matrix entry in COO format
// at position (row, col) is 'value'
typedef struct __CooEntry {
	int row;
	int col;
	float value;
} CooEntry;

typedef struct __ListCooEntry {
	CooEntry *arr;
	unsigned int elements;
	unsigned int size;
} ListCooEntry;



int cooEntryCompare(const void *a, const void *b);

ListCooEntry* initListCooEntry(unsigned int elementCount);

void freeListCooEntry(ListCooEntry *list);

void appendToListCooEntry(ListCooEntry *list, int i, int j, float value);

void sortListCooEntry(ListCooEntry *list);

int isSortedListCooEntry(ListCooEntry *list);

#endif

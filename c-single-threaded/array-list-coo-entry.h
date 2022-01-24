
#ifndef __ARRAY_LIST_COO_ENTRY
#define __ARRAY_LIST_COO_ENTRY



// Matrix entry in COO format
// at position (row, col) is 'value'
typedef struct __CooEntry {
	int row;
	int col;
	double value;
} CooEntry;

typedef struct __ArrayListCooEntry {
	CooEntry *arr;
	unsigned int elements;
	unsigned int size;
} ArrayListCooEntry;



int cooEntryCompare(const void *a, const void *b);

ArrayListCooEntry* initArrayListCooEntry(int initialSize);

void freeArrayListCooEntry(ArrayListCooEntry *list);

void appendToArrayListCooEntry(ArrayListCooEntry *list, int i, int j, double value);

void sortArrayListCooEntry(ArrayListCooEntry *list);

int isSortedArrayListCooEntry(ArrayListCooEntry *list);

#endif

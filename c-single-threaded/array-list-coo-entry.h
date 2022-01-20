
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
	long elements;
	long size;
} ArrayListCooEntry;



extern int cooEntryCompare(const void *a, const void *b);

extern ArrayListCooEntry* initArrayListCooEntry(int initialSize);

extern void freeArrayListCooEntry(ArrayListCooEntry *list);

extern void appendToArrayListCooEntry(ArrayListCooEntry *list, int i, int j, double value);

extern void sortArrayListCooEntry(ArrayListCooEntry *list);

#endif

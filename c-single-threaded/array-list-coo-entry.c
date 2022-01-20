
#include <stdlib.h>
#include "array-list-coo-entry.h"


// Comparison function for qsort
int cooEntryCompare(const void *a, const void *b) {
	CooEntry *coo1 = (CooEntry*) a;
	CooEntry *coo2 = (CooEntry*) b;
	int row_diff = coo1->row - coo2->row;
	if (row_diff == 0) {
		// Same row => return column diff
		return (coo1->col - coo2->col);
	}
	return row_diff;
}

ArrayListCooEntry* initArrayListCooEntry(int initialSize) {
	ArrayListCooEntry *list = (ArrayListCooEntry*) malloc(sizeof(ArrayListCooEntry));
	list->size = (long) initialSize;
	list->elements = 0L;
	list->arr = (CooEntry*) malloc(sizeof(CooEntry) * initialSize);
	return list;
}

void freeArrayListCooEntry(ArrayListCooEntry *list) {
	free(list->arr);
	free(list);
}

void appendToArrayListCooEntry(ArrayListCooEntry *list, int i, int j, double value) {
	if (list->elements >= list->size) {
		long newSize = 2L * list->size;
		list->arr = (CooEntry*) reallocarray(list->arr, newSize, sizeof(CooEntry));
		list->size = newSize;
	}
	CooEntry *entry = &(list->arr[list->elements]);
	entry->row = i;
	entry->col = j;
	entry->value = value;

	list->elements++;
}

void sortArrayListCooEntry(ArrayListCooEntry *list) {
	qsort(list->arr, list->elements, sizeof(CooEntry), cooEntryCompare);
}


/*
#include <stdio.h>
int main(int argc, char **argv) {

	ArrayListCooEntry *list = initArrayListCooEntry(4);

	/ * Sorted
	appendToArrayListCooEntry(list, 0, 1, 5);
	appendToArrayListCooEntry(list, 0, 3, 1);
	appendToArrayListCooEntry(list, 1, 0, 2);
	appendToArrayListCooEntry(list, 1, 1, 3);
	appendToArrayListCooEntry(list, 1, 2, 6);
	appendToArrayListCooEntry(list, 2, 2, 7);
	appendToArrayListCooEntry(list, 3, 0, 1);
	* /

	appendToArrayListCooEntry(list, 2, 2, 7.0);
	appendToArrayListCooEntry(list, 3, 0, 1.0);
	appendToArrayListCooEntry(list, 0, 1, 5.0);
	appendToArrayListCooEntry(list, 1, 0, 2.0);
	appendToArrayListCooEntry(list, 1, 2, 6.0);
	appendToArrayListCooEntry(list, 0, 3, 1.0);
	appendToArrayListCooEntry(list, 1, 1, 3.0);


	printf("Size: %ld; Elements: %ld\n", list->size, list->elements);
	for (long j = 0; j < list->elements; j++) {
		CooEntry e = list->arr[j];
		printf("[%d] Row: %d, Col: %d, Value: %f\n", j, e.row, e.col, e.value);
	}

	sortArrayListCooEntry(list);


	printf("Size: %ld; Elements: %ld\n", list->size, list->elements);
	for (long j = 0; j < list->elements; j++) {
		CooEntry e = list->arr[j];
		printf("[%d] Row: %d, Col: %d, Value: %f\n", j, e.row, e.col, e.value);
	}

	freeArrayListCooEntry(list);
	return 0;
}*/


#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "list-coo-entry.h"



/*
 * Vmesna struktura saj ne pricakujemo, da so vhodni podatki ze sortirani.
 * Tako lahko sortiramo z vgrajeno implementacijo quicksorta.
 */


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


ListCooEntry* initListCooEntry(unsigned int elementCount) {
	ListCooEntry *list = (ListCooEntry*) malloc(sizeof(ListCooEntry));
	list->size = elementCount;
	list->elements = 0u;
	list->arr = (CooEntry*) malloc(sizeof(CooEntry) * elementCount);
	return list;
}

void freeListCooEntry(ListCooEntry *list) {
	free(list->arr);
	free(list);
}

void appendToListCooEntry(ListCooEntry *list, int i, int j, double value) {
	CooEntry *entry = &(list->arr[list->elements]);
	entry->row = i;
	entry->col = j;
	entry->value = value;

	list->elements++;
}

void sortListCooEntry(ListCooEntry *list) {
	qsort(list->arr, list->elements, sizeof(CooEntry), cooEntryCompare);
}

int isSortedListCooEntry(ListCooEntry *list) {
	CooEntry *entry = &(list->arr[0]);
	int prevRow = entry->row;
	int prevCol = entry->col;
	int currRow, currCol;

	for (int i = 1; i < list->elements ; i++) {
		entry = &(list->arr[i]);
		currRow = entry->row;
		currCol = entry->col;
		if ((prevRow >= currRow) && (prevCol > currCol)) {
			return 0;
		}
		prevRow = currRow;
		prevCol = currCol;
	}
	return 1;
}

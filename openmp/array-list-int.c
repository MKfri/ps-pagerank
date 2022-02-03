
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "array-list-int.h"


ArrayListInt* initArrayListInt(int initialSize) {
	ArrayListInt *list = (ArrayListInt*) malloc(sizeof(ArrayListInt));
	list->size = (unsigned int) initialSize;
	list->elements = 0u;
	list->arr = (int*) malloc(sizeof(int) * initialSize);
	return list;
}

void freeArrayListInt(ArrayListInt *list) {
	free(list->arr);
	free(list);
}

void appendToArrayListInt(ArrayListInt *list, int newElement) {
	if (list->elements >= list->size) {
		// Limit size to ~16 GB
		if (list->size == UINT_MAX) {
			printf("[0x4567] Error, cannot extend array list of integers, unsigned int overflow");
			return;
		}

		unsigned int newSize = 2u * list->size;
		if (newSize > UINT_MAX) {
			newSize = UINT_MAX;
		}

		list->arr = (int*) realloc(list->arr, newSize * sizeof(int));
		list->size = newSize;
	}
	list->arr[list->elements] = newElement;
	list->elements++;
}


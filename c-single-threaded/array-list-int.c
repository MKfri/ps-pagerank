
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

		list->arr = (int*) reallocarray(list->arr, newSize, sizeof(int));
		list->size = newSize;
	}
	list->arr[list->elements] = newElement;
	list->elements++;
}


/*

int main(int argc, char **argv) {

	ArrayListInt *list = initArrayListInt(4);

	for (int i = 419; i < 1037337337; i++) {
		appendToArrayListInt(list, i);
	}

	printf("Size: %ld; Elements: %ld\n", list->size, list->elements);
	for (long j = 0; j < list->elements; j++) {
		if (j%1337L == 0L) {
			//printf("[%d] => %d\n", j, list->arr[j]);
		}
	}

	freeArrayListInt(list);
	return 0;
}*/

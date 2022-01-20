
#include <stdlib.h>
#include "array-list-int.h"


typedef struct __ArrayListInt {
	int *arr;
	long elements;
	long size;
} ArrayListInt;

ArrayListInt* initArrayListInt(int initialSize) {
	ArrayListInt *list = (ArrayListInt*) malloc(sizeof(ArrayListInt));
	list->size = (long) initialSize;
	list->elements = 0L;
	list->arr = (int*) malloc(sizeof(int) * initialSize);
	return list;
}

void freeArrayListInt(ArrayListInt *list) {
	free(list->arr);
	free(list);
}

void appendToArrayListInt(ArrayListInt *list, int newElement) {
	if (list->elements >= list->size) {
		long newSize = 2L * list->size;
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

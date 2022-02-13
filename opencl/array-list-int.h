
#ifndef __ARRAY_LIST_INT
#define __ARRAY_LIST_INT

typedef struct __ArrayListInt {
	int *arr;
	unsigned int elements;
	unsigned int size;
} ArrayListInt;


ArrayListInt* initArrayListInt(int initialSize);

void freeArrayListInt(ArrayListInt *list);

void appendToArrayListInt(ArrayListInt *list, int newElement);

#endif

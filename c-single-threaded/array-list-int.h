
#ifndef __ARRAY_LIST_INT
#define __ARRAY_LIST_INT

typedef struct __ArrayListInt {
	int *arr;
	unsigned int elements;
	unsigned int size;
} ArrayListInt;


extern ArrayListInt* initArrayListInt(int initialSize);

extern void freeArrayListInt(ArrayListInt *list);

extern void appendToArrayListInt(ArrayListInt *list, int newElement);

#endif

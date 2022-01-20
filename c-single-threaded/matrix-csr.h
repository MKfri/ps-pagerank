
#include "array-list-coo-entry.h"

#ifndef __MATRIX_CSR
#define __MATRIX_CSR


typedef struct __MatrixCsr {
	double *values;
	unsigned int *columnIdx;
	unsigned int *rowPtr;

	int rowPtrLen;
	unsigned int valuesLen;
} MatrixCsr;

extern MatrixCsr* cooToCsr(ArrayListCooEntry *listCoo, int rowCount);

extern void freeMatrixCsr(MatrixCsr *csr);


#endif

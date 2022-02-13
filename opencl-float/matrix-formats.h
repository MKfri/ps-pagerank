
#include "list-coo-entry.h"

#ifndef __MATRIX_FORMATS
#define __MATRIX_FORMATS


typedef struct __MatrixCoo {
	float *values;
	unsigned int *columnIdx;
	unsigned int *rowIdx;

	unsigned int valuesLen;
} MatrixCoo;


typedef struct __MatrixCsr {
	float *values;
	unsigned int *columnIdx;
	unsigned int *rowPtr;

	int rowPtrLen;
	unsigned int valuesLen;
} MatrixCsr;


typedef struct __MatrixEll {
	float *values;
	unsigned int *columnIdx;

	unsigned int rows;
	unsigned int columnsPerRow;
} MatrixEll;


// Predpostavimo da je listCoo, ki ga dobimo kot prameter ze urejen
MatrixCsr* compactCooToCsr(ListCooEntry *listCoo, int rowCount);
MatrixCoo* compactCooToCoo(ListCooEntry *listCoo, int rowCount);

void compactCooToHybridEllCsr(ListCooEntry *listCoo, MatrixEll *ellMatrix, MatrixCsr *csrMatrix, int rowCount);


void freeMatrixCsr(MatrixCsr *csr);
void freeMatrixCoo(MatrixCoo *coo);
void freeMatrixEll(MatrixEll *ell);


#endif
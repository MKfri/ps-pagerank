

#include "list-coo-entry.h"

#ifndef __MATRIX_FORMATS
#define __MATRIX_FORMATS



typedef struct __MatrixCoo {
	double *values;
	unsigned int *columnIdx;
	unsigned int *rowIdx;

	unsigned int valuesLen;
} MatrixCoo;


typedef struct __MatrixCsr {
	double *values;
	unsigned int *columnIdx;
	unsigned int *rowPtr;

	int rowPtrLen;
	unsigned int valuesLen;
} MatrixCsr;


typedef struct __MatrixEll {
	double *values;
	unsigned int *columnIdx;

	unsigned int rows;
	unsigned int columnsPerRow;
} MatrixEll;


//void getRowDistribution(ListCooEntry *listCoo, int rowCount, int SIZE);


// Predpostavimo da je listCoo, ki ga dobimo kot prameter ze urejen
MatrixCsr* compactCooToCsr(ListCooEntry *listCoo, int rowCount);
MatrixCoo* compactCooToCoo(ListCooEntry *listCoo, int rowCount);
//MatrixEll* compactCooToEll(ListCooEntry *listCoo, int rowCount);

void compactCooToHybridEllCsr(ListCooEntry *listCoo, MatrixEll *ellMatrix, MatrixCsr *csrMatrix, int rowCount);

//void compactCooToHybrid(ListCooEntry *listCoo, MatrixEll *ellMatrix, MatrixCoo *cooMatrix, int rowCount);


void freeMatrixCsr(MatrixCsr *csr);
void freeMatrixCoo(MatrixCoo *coo);
void freeMatrixEll(MatrixEll *ell);


#endif
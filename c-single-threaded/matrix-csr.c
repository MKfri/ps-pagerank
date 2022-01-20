
#include <stdio.h>
#include <stdlib.h>
#include "matrix-csr.h"


// Predpostavimo da je listCoo, ki ga tu dobimo ze urejen
MatrixCsr* cooToCsr(ArrayListCooEntry *listCoo, int rowCount) {
	long elementCount = listCoo->elements;
	CooEntry *listArr = listCoo->arr;

	MatrixCsr *csr = (MatrixCsr*) malloc(sizeof(MatrixCsr));
	csr->values = (double*) malloc(sizeof(double) * elementCount);

	csr->columnIdx = (unsigned int*) malloc(sizeof(unsigned int) * elementCount);

	// Dolzina `rowCount + 1`
	// Da lahko potem gremo lepo po parih brez skrbi
	csr->rowPtr = (unsigned int*) malloc(sizeof(unsigned int) * (rowCount + 1));

	int currRow = 0;
	csr->rowPtr[currRow] = 0u;

	for (long k = 0; k < elementCount; k++) {
		CooEntry e = listArr[k];
		csr->values[k] = e.value;
		csr->columnIdx[k] = e.col;

		if (e.row > currRow) {
			for (int j = currRow; j < e.row; j++) {
				csr->rowPtr[j+1] = (unsigned int) k;
			}
			currRow = e.row;
		}
	}

	for (int j = currRow; j < rowCount; j++) {
		csr->rowPtr[j+1] = (unsigned int) elementCount;
	}

	csr->rowPtrLen = rowCount; // +1 ???
	csr->valuesLen = elementCount;

	return csr;
}


void freeMatrixCsr(MatrixCsr *csr) {
	free(csr->values);
	free(csr->columnIdx);
	free(csr->rowPtr);
	free(csr);
}


/*
int main() {
	printf("Values = [");
	for (unsigned int i = 0u; i < csrMatrix->valuesLen; i++) {
		if (i != 0u) {
			printf(", ");
		}
		printf("%.3f", csrMatrix->values[i]);
	}
	printf("]\n");

	printf("Columns = [");
	for (unsigned int i = 0u; i < csrMatrix->valuesLen; i++) {
		if (i != 0u) {
			printf(", ");
		}
		printf("%d", csrMatrix->columnIdx[i]);
	}
	printf("]\n");

	printf("Rows = [");
	for (int i = 0; i < csrMatrix->rowPtrLen+1; i++) {
		if (i != 0) {
			printf(", ");
		}
		printf("%d", csrMatrix->rowPtr[i]);
	}
	printf("]\n");
}*/


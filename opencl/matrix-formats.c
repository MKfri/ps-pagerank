
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matrix-formats.h"



MatrixCsr* compactCooToCsr(ListCooEntry *listCoo, int rowCount) {
	unsigned int elementCount = listCoo->elements;
	CooEntry *listArr = listCoo->arr;

	MatrixCsr *csr = (MatrixCsr*) malloc(sizeof(MatrixCsr));
	csr->values = (double*) malloc(sizeof(double) * elementCount);

	csr->columnIdx = (unsigned int*) malloc(sizeof(unsigned int) * elementCount);

	// Dolzina `rowCount + 1`
	// Da lahko potem gremo lepo po parih brez skrbi
	csr->rowPtr = (unsigned int*) malloc(sizeof(unsigned int) * (rowCount + 1));

	int currRow = 0;
	csr->rowPtr[currRow] = 0u;


	for (unsigned int k = 0u; k < elementCount; k++) {
		CooEntry e = listArr[k];
		csr->values[k] = e.value;
		csr->columnIdx[k] = e.col;

		if (e.row > currRow) {
			for (int j = currRow; j < e.row; j++) {
				csr->rowPtr[j+1] = k;
			}
			currRow = e.row;
		}
	}

	for (int j = currRow; j < rowCount; j++) {
		csr->rowPtr[j+1] = elementCount;
	}

	csr->rowPtrLen = rowCount; // +1 ???
	csr->valuesLen = elementCount;

	return csr;
}

// CooEntry array v 3 arraye (values, columns, rows)
MatrixCoo* compactCooToCoo(ListCooEntry *listCoo, int rowCount) {
    unsigned int elementCount = listCoo->elements;
	CooEntry *listArr = listCoo->arr;

    MatrixCoo *coo = (MatrixCoo*) malloc(sizeof(MatrixCoo));
    coo->valuesLen = elementCount;
	coo->values = (double*) malloc(sizeof(double) * elementCount);

	coo->columnIdx = (unsigned int*) malloc(sizeof(unsigned int) * elementCount);
    coo->rowIdx = (unsigned int*) malloc(sizeof(unsigned int) * elementCount);

    for (unsigned int k = 0u; k < elementCount; k++) {
		CooEntry e = listArr[k];
		coo->values[k] = e.value;
		coo->columnIdx[k] = e.col;
		coo->rowIdx[k] = e.row;
	}
    return coo;
}

/*
// Useless za matrike kot jih imamo, se posebej za CPU
// 2D matrika je shranjena kot 1D vektor
// 2 razlicna pristopa / optimizaciji
// CPU -> Matrika je shranjena po vrsticah
// GPU -> po stolpcih
MatrixEll* compactCooToEll(ListCooEntry *listCoo, int rowCount) {
	unsigned int elementCount = listCoo->elements;
	CooEntry *listArr = listCoo->arr;

	MatrixEll *ell = (MatrixEll*) malloc(sizeof(MatrixEll));

	// Najprej poracunamo max stevilo elementov v vrstici
	int maxElements = 0;

	unsigned int currRow = ((CooEntry) listArr[0]).row;
	int currElements = 1;

	unsigned int thisRow;

	for (unsigned int k = 1u; k < elementCount; k++) {
		thisRow = ((CooEntry) listArr[k]).row;

		if (thisRow == currRow) {
			currElements++;
		} else {
			if (currElements > maxElements) {
				maxElements = currElements;
			}
			currRow = thisRow;
			currElements = 1;
		}
	}
	if (currElements > maxElements) {
		maxElements = currElements;
	}

	printf("[ELL] Max elements: %d; rows: %d\n", maxElements, rowCount);
	

	ell->columnsPerRow = maxElements;
	ell->rows = rowCount;

	long allocationSize = (long) maxElements * (long) rowCount;
	double *ellValues = (double*) calloc(allocationSize, sizeof(double));
	unsigned int *ellColIdx = (unsigned int*) calloc(allocationSize, sizeof(unsigned int));


	if (ellValues == NULL || ellColIdx == NULL) {
		printf("Error allocating memory for ELL format\n");
		printf("ellValues == NULL? %d\n", (ellValues == NULL));
		printf("ellColIdx == NULL? %d\n", (ellColIdx == NULL));
		exit(1);
	}

	long rowIndex = 0L;
	long colIndex = 0L;
	long row;
	for (unsigned int k = 0u; k < elementCount; k++) {
		CooEntry e = listArr[k];
		row = (long) e.row;

		if (row > rowIndex) {
			rowIndex = row;
			colIndex = 0L;
		}
		// OPOMBA:
		// Stolpcu pristejemo 1, pred uporabo treba odsteti
		// 0   -> invalid value
		// k>0 -> stolpec == k-1

#ifdef ELL_CPU_OPTIMIZE
		ellValues[maxElements*row + colIndex] = e.value;
		ellColIdx[maxElements*row + colIndex] = e.col + 1u;
#else	// za GPU
		ellValues[rowCount*colIndex + row] = e.value;
		ellColIdx[rowCount*colIndex + row] = e.col + 1u;
#endif
		
		colIndex++;
	}

	ell->values = ellValues;
	ell->columnIdx = ellColIdx;

	return ell;
}*/


/** HYBRID between CSR and ELL **/
void compactCooToHybridEllCsr(ListCooEntry *listCoo, MatrixEll *ellMatrix, MatrixCsr *csrMatrix, int rowCount) {
	unsigned int elementCount = listCoo->elements;
	CooEntry *listArr = listCoo->arr;

	double avgElementsPerRow = ((double) elementCount) / ((double) rowCount);
	int elementsPerRowEll = ceil(avgElementsPerRow);

	unsigned int elementsInCsr = 0;

	// Ugotoviti je potrebno, koliko elementov preostane za v CSR reprezentacijo
	unsigned int currentRow = ((CooEntry) listArr[0]).row;
	int currentElements = 1;

	for (unsigned int k = 1u; k < elementCount; k++) {
		unsigned int thisRow = ((CooEntry) listArr[k]).row;
		if (thisRow == currentRow) {
			currentElements++;
		} else {
			if (currentElements > elementsPerRowEll) {
				elementsInCsr += (currentElements - elementsPerRowEll);
			}
			currentRow = thisRow;
			currentElements = 1;
		}
	}
	if (currentElements > elementsPerRowEll) {
		elementsInCsr += (currentElements - elementsPerRowEll);
	}

	// Inicializacija ELL
	ellMatrix->columnsPerRow = elementsPerRowEll;
	ellMatrix->rows = rowCount;
	long allocationSize = (long) elementsPerRowEll * (long) rowCount;
	double *ellValues = (double*) calloc(allocationSize, sizeof(double));
	unsigned int *ellColIdx = (unsigned int*) calloc(allocationSize, sizeof(unsigned int));

	// Inicializacija CSR
	csrMatrix->valuesLen = elementsInCsr;
	double *csrValues = (double*) malloc(sizeof(double) * elementsInCsr);
	unsigned int *csrColIdx = (unsigned int*) malloc(sizeof(unsigned int) * elementsInCsr);
	// Dolzina `rowCount + 1`
	// Da lahko potem gremo lepo po parih brez skrbi
    unsigned int *csrRowPtr = (unsigned int*) malloc(sizeof(unsigned int) * (rowCount + 1));
	
	// Did we get the memory??
	if (ellValues == NULL || ellColIdx == NULL || csrValues == NULL || csrColIdx == NULL || csrRowPtr == NULL) {
		printf("Error allocating memory for HYBRID format\n");
		printf("ellValues == NULL? %d\n", (ellValues == NULL));
		printf("ellColIdx == NULL? %d\n", (ellColIdx == NULL));
		printf("csrValues == NULL? %d\n", (csrValues == NULL));
		printf("csrColIdx == NULL? %d\n", (csrColIdx == NULL));
		printf("csrRowPtr == NULL? %d\n", (csrRowPtr == NULL));
		exit(1);
	}


	long csrIndex = 0L;
	long csrRow = 0L;
	csrRowPtr[csrRow] = 0u;

	long rowIndex = 0L;
	long colIndex = 0L;
	long row;

	for (unsigned int k = 0u; k < elementCount; k++) {
		CooEntry e = listArr[k];
		row = (long) e.row;

		if (row > rowIndex) {
			rowIndex = row;
			colIndex = 0L;
		}

		if (colIndex < elementsPerRowEll) {
			// ELL
			ellValues[rowCount*colIndex + row] = e.value;
			ellColIdx[rowCount*colIndex + row] = e.col + 1u;
		} else {
			// CSR
			csrValues[csrIndex] = e.value;
			csrColIdx[csrIndex] = e.col;

			if (row > csrRow) {
				for (int j = csrRow; j < row; j++) {
					csrRowPtr[j+1] = csrIndex;
				}
				csrRow = row;
			}
			csrIndex++;
		}
		colIndex++;
	}

	for (long j = csrRow; j < rowCount; j++) {
		csrRowPtr[j+1] = elementsInCsr;
	}

	// ELL
	ellMatrix->values = ellValues;
	ellMatrix->columnIdx = ellColIdx;
	// CSR
	csrMatrix->values = csrValues;
	csrMatrix->columnIdx = csrColIdx;
    csrMatrix->rowPtr = csrRowPtr;
    csrMatrix->rowPtrLen = rowCount;
}





/** HYBRID between COO and ELL **/
/*
void compactCooToHybrid(ListCooEntry *listCoo, MatrixEll *ellMatrix, MatrixCoo *cooMatrix, int rowCount) {
	unsigned int elementCount = listCoo->elements;
	CooEntry *listArr = listCoo->arr;

	double avgElementsPerRow = ((double) elementCount) / ((double) rowCount);
	int elementsPerRowEll = ceil(avgElementsPerRow);

	unsigned int elementsInCoo = 0;

	// Ugotoviti je potrebno, koliko elementov preostane za v COO reprezentacijo
	unsigned int currentRow = ((CooEntry) listArr[0]).row;
	int currentElements = 1;

	for (unsigned int k = 1u; k < elementCount; k++) {
		unsigned int thisRow = ((CooEntry) listArr[k]).row;
		if (thisRow == currentRow) {
			currentElements++;
		} else {
			if (currentElements > elementsPerRowEll) {
				elementsInCoo += (currentElements - elementsPerRowEll);
			}
			currentRow = thisRow;
			currentElements = 1;
		}
	}
	if (currentElements > elementsPerRowEll) {
		elementsInCoo += (currentElements - elementsPerRowEll);
	}


	// Inicializacija ELL
	ellMatrix->columnsPerRow = elementsPerRowEll;
	ellMatrix->rows = rowCount;
	long allocationSize = (long) elementsPerRowEll * (long) rowCount;
	double *ellValues = (double*) calloc(allocationSize, sizeof(double));
	unsigned int *ellColIdx = (unsigned int*) calloc(allocationSize, sizeof(unsigned int));

	// Inicializacija COO
	cooMatrix->valuesLen = elementsInCoo;
	double *cooValues = (double*) malloc(sizeof(double) * elementsInCoo);
	unsigned int *cooColIdx = (unsigned int*) malloc(sizeof(unsigned int) * elementsInCoo);
    unsigned int *cooRowIdx = (unsigned int*) malloc(sizeof(unsigned int) * elementsInCoo);

	if (ellValues == NULL || ellColIdx == NULL || cooValues == NULL || cooColIdx == NULL || cooRowIdx == NULL) {
		printf("Error allocating memory for HYBRID format\n");
		printf("ellValues == NULL? %d\n", (ellValues == NULL));
		printf("ellColIdx == NULL? %d\n", (ellColIdx == NULL));
		printf("cooValues == NULL? %d\n", (cooValues == NULL));
		printf("cooColIdx == NULL? %d\n", (cooColIdx == NULL));
		printf("cooRowIdx == NULL? %d\n", (cooRowIdx == NULL));
		exit(1);
	}

	long cooIndex = 0L;

	long rowIndex = 0L;
	long colIndex = 0L;
	long row;
	for (unsigned int k = 0u; k < elementCount; k++) {
		CooEntry e = listArr[k];
		row = (long) e.row;

		if (row > rowIndex) {
			rowIndex = row;
			colIndex = 0L;
		}

		if (colIndex < elementsPerRowEll) {
			// ELL
#ifdef ELL_CPU_OPTIMIZE
			ellValues[elementsPerRowEll*row + colIndex] = e.value;
			ellColIdx[elementsPerRowEll*row + colIndex] = e.col + 1u;
#else	// za GPU
			ellValues[rowCount*colIndex + row] = e.value;
			ellColIdx[rowCount*colIndex + row] = e.col + 1u;
#endif
		} else {
			// COO
			cooValues[cooIndex] = e.value;
			cooColIdx[cooIndex] = e.col;
			cooRowIdx[cooIndex] = e.row;

			cooIndex++;
		}
		
		colIndex++;
	}

	// ELL
	ellMatrix->values = ellValues;
	ellMatrix->columnIdx = ellColIdx;
	// COO
	cooMatrix->values = cooValues;
	cooMatrix->columnIdx = cooColIdx;
    cooMatrix->rowIdx = cooRowIdx;
}



void getRowDistribution(ListCooEntry *listCoo, int rowCount, int SIZE) {
	unsigned int elementCount = listCoo->elements;
	CooEntry *listArr = listCoo->arr;

	//int SIZE = 1000000;
	unsigned int *distribution = (unsigned int*) calloc(SIZE, sizeof(unsigned int));


	// Najprej poracunamo max stevilo elementov v vrstici
	int maxElements = 0;

	unsigned int currRow = ((CooEntry) listArr[0]).row;
	int currElements = 1;

	unsigned int thisRow;

	for (unsigned int k = 1u; k < elementCount; k++) {
		thisRow = ((CooEntry) listArr[k]).row;

		if (thisRow == currRow) {
			currElements++;
		} else {
			distribution[currElements]++;

			if (currElements > maxElements) {
				maxElements = currElements;
			}
			currRow = thisRow;
			currElements = 1;
		}
	}
	distribution[currElements]++;
	if (currElements > maxElements) {
		maxElements = currElements;
	}

	int nonZeroRows = 0;

	for (int k = 1; k <= maxElements; k++) {
		printf("%d, %d\n", k, distribution[k]);
		nonZeroRows += distribution[k];
	}
	printf("0, %d\n", rowCount - nonZeroRows);
}
*/


void freeMatrixCsr(MatrixCsr *csr) {
	free(csr->values);
	free(csr->columnIdx);
	free(csr->rowPtr);
	free(csr);
}

void freeMatrixCoo(MatrixCoo *coo) {
	free(coo->values);
	free(coo->columnIdx);
	free(coo->rowIdx);
	free(coo);
}

void freeMatrixEll(MatrixEll *ell) {
	free(ell->values);
	free(ell->columnIdx);
	free(ell);
}



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include "matrix-formats.h"



MatrixCsr* compactCooToCsr(ListCooEntry *listCoo, int rowCount) {
	/**/unsigned int elementCount = listCoo->elements;
	CooEntry *listArr = listCoo->arr;

	MatrixCsr *csr = (MatrixCsr*) malloc(sizeof(MatrixCsr));
	csr->values = (double*) malloc(sizeof(double) * elementCount);

	csr->columnIdx = (unsigned int*) malloc(sizeof(unsigned int) * elementCount);

	// Dolzina `rowCount + 1`
	// Da lahko potem gremo lepo po parih brez skrbi
	csr->rowPtr = (unsigned int*) malloc(sizeof(unsigned int) * (rowCount + 1));

	CooEntry lastEntry = listArr[elementCount-1];
	int lastRow = lastEntry.row;

	// Sem zapisemo kje bo katera od niti zacela
	// Torej kos, ki ga mora obdelati nit J je:
	// listArr[i] | taskBondaries[J] <= i < taskBoundaries[J+1]
	int maxThreads = omp_get_max_threads();
	unsigned int *taskBoundaries = (unsigned int*) calloc(maxThreads + 1, sizeof(unsigned int));
	taskBoundaries[maxThreads] = elementCount;


	#pragma omp parallel shared(listArr, taskBoundaries, csr, listCoo, elementCount, lastRow)
	{
		double countPerThread = ((double) elementCount) / omp_get_num_threads();
		int threadIndex = omp_get_thread_num();
		unsigned int start = ceil(countPerThread * threadIndex);

		CooEntry myFirstEntry = listArr[start];
		int currRow = myFirstEntry.row;

		unsigned int realStart = start;
		if (start > 0u) {
			CooEntry e;
			do {
				realStart -= 1u;
				e = listArr[realStart];
			} while (e.row == currRow);
			realStart += 1u;
		}
		taskBoundaries[threadIndex] = realStart;

		#pragma omp barrier


		csr->rowPtr[currRow] = realStart;
		unsigned int realEnd = taskBoundaries[threadIndex+1];


		for (unsigned int k = realStart; k < realEnd; k++) {
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

		if (realEnd < elementCount) {
			unsigned int firstRowOfNextThread = ((CooEntry) listArr[realEnd+1]).row;
			for (int j = currRow; j < firstRowOfNextThread; j++) {
				csr->rowPtr[j+1] = realEnd;
			}
		}

		#pragma omp for
		for (int j = lastRow; j < rowCount; j++) {
			csr->rowPtr[j+1] = elementCount;
		}
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

	#pragma omp parallel for
    for (unsigned int k = 0u; k < elementCount; k++) {
		CooEntry e = listArr[k];
		coo->values[k] = e.value;
		coo->columnIdx[k] = e.col;
		coo->rowIdx[k] = e.row;
	}
    return coo;
}


// Useless za matrike kot jih imamo, se posebej za CPU
// 2D matrika je shranjena kot 1D vektor
// 2 razlicna pristopa / optimizaciji
// CPU -> Matrika je shranjena po vrsticah
// GPU -> po stolpcih
// Tej funkciji je prihranjena paralelizacija,
// se ne skalira preko najmanjsega primerka podatkov
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
}


/** HYBRID between COO and ELL **/
void compactCooToHybrid(ListCooEntry *listCoo, MatrixEll *ellMatrix, MatrixCoo *cooMatrix, int rowCount) {
	unsigned int elementCount = listCoo->elements;
	CooEntry *listArr = listCoo->arr;

	double avgElementsPerRow = ((double) elementCount) / ((double) rowCount);
	int elementsPerRowEll = ceil(avgElementsPerRow);

	unsigned int elementsInCoo = 0;


	// Delitev dela na niti
	int maxThreads = omp_get_max_threads();
	unsigned int *taskBoundaries = (unsigned int*) calloc(maxThreads + 1, sizeof(unsigned int));
	taskBoundaries[maxThreads] = elementCount;

	// Array za sledenje katera nit bo zapisala koliko COO elementov
	// Kasneje se v ta array iz dolzin poracunajo indeksi
	unsigned int *cooElementsPerThread = (unsigned int*) calloc(maxThreads + 1, sizeof(unsigned int));


	double *ellValues;
	unsigned int *ellColIdx;
	double *cooValues;
	unsigned int *cooColIdx;
	unsigned int *cooRowIdx;


	#pragma omp parallel
	{
		double countPerThread = ((double) elementCount) / omp_get_num_threads();
		int threadIndex = omp_get_thread_num();
		unsigned int start = ceil(countPerThread * threadIndex);

		CooEntry myFirstEntry = listArr[start];
		int currRow = myFirstEntry.row;

		unsigned int realStart = start;
		if (start > 0u) {
			CooEntry e;
			do {
				realStart--;
				e = listArr[realStart];
			} while (e.row == currRow);
			realStart++;
		}
		taskBoundaries[threadIndex] = realStart;

		// Sinhroniziramo array taskBoundaries
		#pragma omp barrier

		// Tukaj znani vrednosti spremenljivk realEnd in realStart
		unsigned int realEnd = taskBoundaries[threadIndex+1];


		// Ugotoviti je potrebno, koliko elementov preostane za v COO reprezentacijo
		int cooElementsCountedByThisThread = 0;
		int elementsInCurrentRow = 1;
		unsigned int currentRow = ((CooEntry) listArr[realStart]).row;

		for (unsigned int k = realStart + 1u; k < realEnd; k++) {
			unsigned int thisRow = ((CooEntry) listArr[k]).row;
			if (thisRow == currentRow) {
				elementsInCurrentRow++;
			} else {
				if (elementsInCurrentRow > elementsPerRowEll) {
					cooElementsCountedByThisThread += (elementsInCurrentRow - elementsPerRowEll);
				}
				currentRow = thisRow;
				elementsInCurrentRow = 1;
			}
		}

		if (elementsInCurrentRow > elementsPerRowEll) {
			cooElementsCountedByThisThread += (elementsInCurrentRow - elementsPerRowEll);
		}
		cooElementsPerThread[threadIndex+1] = cooElementsCountedByThisThread;

		#pragma omp barrier


		#pragma omp single
		{
			for (int t = 1; t <= maxThreads; t++) {
				cooElementsPerThread[t] += cooElementsPerThread[t-1];
			}
			elementsInCoo = cooElementsPerThread[maxThreads];

			// Inicializacija ELL
			ellMatrix->columnsPerRow = elementsPerRowEll;
			ellMatrix->rows = rowCount;
			long allocationSize = (long) elementsPerRowEll * (long) rowCount;
			ellValues = (double*)       calloc(allocationSize, sizeof(double));
			ellColIdx = (unsigned int*) calloc(allocationSize, sizeof(unsigned int));

			// Inicializacija COO
			cooMatrix->valuesLen = elementsInCoo;
			cooValues = (double*)       malloc(sizeof(double) * elementsInCoo);
			cooColIdx = (unsigned int*) malloc(sizeof(unsigned int) * elementsInCoo);
			cooRowIdx = (unsigned int*) malloc(sizeof(unsigned int) * elementsInCoo);

			if (ellValues == NULL || ellColIdx == NULL || cooValues == NULL || cooColIdx == NULL || cooRowIdx == NULL) {
				printf("Error allocating memory for HYBRID format\n");
				printf("ellValues == NULL? %d\n", (ellValues == NULL));
				printf("ellColIdx == NULL? %d\n", (ellColIdx == NULL));
				printf("cooValues == NULL? %d\n", (cooValues == NULL));
				printf("cooColIdx == NULL? %d\n", (cooColIdx == NULL));
				printf("cooRowIdx == NULL? %d\n", (cooRowIdx == NULL));
				exit(1);
			}

			// ELL
			ellMatrix->values = ellValues;
			ellMatrix->columnIdx = ellColIdx;
			// COO
			cooMatrix->values = cooValues;
			cooMatrix->columnIdx = cooColIdx;
			cooMatrix->rowIdx = cooRowIdx;
		}

		long cooIndex = cooElementsPerThread[threadIndex];
		long rowIndex = ((CooEntry) listArr[realStart]).row;
		long colIndex = 0L;
		long row;

		for (unsigned int k = realStart; k < realEnd; k++) {
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
	}
	free(taskBoundaries);
	free(cooElementsPerThread);
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


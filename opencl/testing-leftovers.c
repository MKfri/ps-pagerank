
/** Various main methods used for testing copied here */
#include "matrix-formats.h"
#include <stdio.h>
#include <stdlib.h>


// HYBRID
int main() {

	/*
	DELA:)
	int N = 5;
	double *mtx = (double*) calloc(N*N, sizeof(double));
	for (int i = 0; i < N; i++) {
		// Diagonalci
		mtx[i*N + i] =  (double) i;
		// Prvi stolpec
		mtx[i*N] = (double) i;
		// Zadnja vrstica
		mtx[(N-1)*N + i] = (double) N - 1.0;
	}
	mtx[N*N-2] = 0.0;*/

	// Tudi ta dela:)
	int N = 9;
	double *mtx = (double*) calloc(N*N, sizeof(double));
	for (int i = 0; i < N; i++) {
		// Prvi stolpec
		mtx[i*N] = (double) i + 1.0;
		// Drugi stolpec
		mtx[i*N + 1] = (double) i + 1.0;
		// Zadnja vrstica
		mtx[(N-1)*N + i] = (double) N - 1.0;
	}
	mtx[2] = 33.0;
	mtx[3] = 1337.0;

	int nonZeros = 0;
	for (int i = 0; i < N*N; i++) {
		if (mtx[i] != 0.0)
			nonZeros++;
		if ((i % N) == 0) {
			printf("\n");
		}
		printf("%f ", mtx[i]);
	}
	printf("\n");
	
	ListCooEntry* list = initListCooEntry(nonZeros);

	for (int i = 0; i < N*N; i++) {
		if (mtx[i] != 0.0) {
			appendToListCooEntry(list, i / N, i % N, mtx[i]);
		}
	}


	MatrixEll *ell = (MatrixEll*) malloc(sizeof(MatrixEll));
	MatrixCsr *csrMatrix = (MatrixCsr*) malloc(sizeof(MatrixCsr));
	compactCooToHybridEllCsr(list, ell, csrMatrix, N);

	// ELL
	int rows = ell->rows;
	int columnsPerRow = ell->columnsPerRow;
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columnsPerRow; j++) {
			printf("%f ", ell->values[j*rows + i]);
		}
		printf("\n");
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columnsPerRow; j++) {
			printf("%d ", ell->columnIdx[j*rows + i]);
		}
		printf("\n");
	}

	// CSR
	int elementCount = csrMatrix->valuesLen;
	printf("CSR: \n");
	for (int k = 0; k < (csrMatrix->rowPtrLen+1); k++) {
		printf("%d; ", csrMatrix->rowPtr[k]);
	}
	printf("\n");
	for (int i = 0; i < N; i++) {
		printf("Row = %d; ", i);
		int s = csrMatrix->rowPtr[i];
		int e = csrMatrix->rowPtr[i+1];

		for (int k = s; k < e; k++) {
			printf("(c=%d, val=%f); ", csrMatrix->columnIdx[k], csrMatrix->values[k]);
		}
		printf("\n");
	}




	return 0;
}

/*
// ELL
int main() {
	int N = 6;
	double *mtx = (double*) calloc(N*N, sizeof(double));

	for (int i = 0; i < N; i++) {
		// Diagonalci
		mtx[i*N + i] =  (double) (N - i) + 1.0; 
	}
	
	for (int i = 0; i < N; i++) {
		// Prvi stolpec
		mtx[i*N] = (double) i + 1.0; 
	}

	mtx[N*N-2] = 10.0;

	int nonZeros = 0;
	for (int i = 0; i < N*N; i++) {
		if (mtx[i] != 0.0)
			nonZeros++;
		if ((i % N) == 0) {
			printf("\n");
		}
		printf("%f ", mtx[i]);

	}
	ListCooEntry* list = initListCooEntry(nonZeros);

	for (int i = 0; i < N*N; i++) {
		if (mtx[i] != 0.0) {
			appendToListCooEntry(list, i / N, i % N, mtx[i]);
		}
	}


	MatrixEll* ell = compactCooToEll(list, N);
	int rows = ell->rows;
	int columnsPerRow = ell->columnsPerRow;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columnsPerRow; j++) {
			printf("%f ", ell->values[j*rows + i]);
		}
		printf("\n");
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columnsPerRow; j++) {
			printf("%d ", ell->columnIdx[j*rows + i]);
		}
		printf("\n");
	}

	return 0;
}*/


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



/*
#include <stdio.h>
int main(int argc, char **argv) {

	ListCooEntry *list = initListCooEntry(4);

	/ * Sorted
	appendToListCooEntry(list, 0, 1, 5);
	appendToListCooEntry(list, 0, 3, 1);
	appendToListCooEntry(list, 1, 0, 2);
	appendToListCooEntry(list, 1, 1, 3);
	appendToListCooEntry(list, 1, 2, 6);
	appendToListCooEntry(list, 2, 2, 7);
	appendToListCooEntry(list, 3, 0, 1);
	* /

	appendToListCooEntry(list, 2, 2, 7.0);
	appendToListCooEntry(list, 3, 0, 1.0);
	appendToListCooEntry(list, 0, 1, 5.0);
	appendToListCooEntry(list, 1, 0, 2.0);
	appendToListCooEntry(list, 1, 2, 6.0);
	appendToListCooEntry(list, 0, 3, 1.0);
	appendToListCooEntry(list, 1, 1, 3.0);


	printf("Size: %ld; Elements: %ld\n", list->size, list->elements);
	for (long j = 0; j < list->elements; j++) {
		CooEntry e = list->arr[j];
		printf("[%d] Row: %d, Col: %d, Value: %f\n", j, e.row, e.col, e.value);
	}

	sortListCooEntry(list);


	printf("Size: %ld; Elements: %ld\n", list->size, list->elements);
	for (long j = 0; j < list->elements; j++) {
		CooEntry e = list->arr[j];
		printf("[%d] Row: %d, Col: %d, Value: %f\n", j, e.row, e.col, e.value);
	}

	freeListCooEntry(list);
	return 0;
}*/
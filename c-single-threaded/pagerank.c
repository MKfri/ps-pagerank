

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#include "array-list-int.h"
#include "array-list-coo-entry.h"
#include "matrix-csr.h"


//#define _OUTPUT_PRINT
//#define _OUTPUT_CHECK


#define D 0.85
#define EPSILON 1e-8



double normOfVectorDifference(double *v1, double *v2, int steviloVozlisc) {
	double sum = 0.0;
	double tmp;
	for (int i = 0; i < steviloVozlisc; i++) {
		tmp = (v1[i] - v2[i]);
		sum += tmp*tmp;
	}
	return sqrt(sum);
}


int main(int argc, char **argv) {

	// Prvi CLI argument je datoteka, oz. pot do datoteke
	if (argc != 2) {
		printf("Error: exactly one argument must be passed\n");
		exit(1);
	}
	printf("Input file: `%s`\n", argv[1]);

	ArrayListCooEntry *listCoo;
	ArrayListInt      *listInt;

	listInt = initArrayListInt(16);
	listCoo = initArrayListCooEntry(16);

	double start = omp_get_wtime();

	FILE *data_file;
	data_file = fopen(argv[1], "r");


	if (data_file == NULL) {
		printf("Error: Cannot open file\n");
		exit(1);
	}

	/*
	 * Iz datoteke bomo brali pare "from to",
	 * ki predstavljajo usmerjene povezave iz vozlisca from v vozlisce to.
	 * 
	 * Pare bomo shranili v en raztegljiv seznam (ArrayList), tako da najprej
	 * dodamo "from", nato pa se "to".
	 *
	 * Shranili bomo maksimum, torej maksimalni indeks, ki ga nosi neko vozlisce.
	 * Privzamemo da se vozlisca zacnejo indeksirati z 0, zato velja:
	 * stevilo vozlisc = max + 1
	 */

	int from, to;
	int readCount = 1;
	int maxVal = 0;

	while (!feof(data_file) && readCount > 0) {
		readCount = fscanf(data_file, "%d %d", &from, &to);

		if (readCount == 2) {
			// From
			appendToArrayListInt(listInt, from);
			// To
			appendToArrayListInt(listInt, to);

			if (from > maxVal) {
				maxVal = from;
			}
			if (to > maxVal) {
				maxVal = to;
			}
		} else if (readCount == 1) {
			// Datoteka je v napacnem formatu 
			printf("Unexpected count of read arguments; readCount == 1");
			exit(1);
		}
	}
	int steviloVozlisc = maxVal + 1;

	double readEnd = omp_get_wtime();


	// L(k) => stevilo izhodnih povezav vozlisca k
	int *L = (int*) calloc(steviloVozlisc, sizeof(int));
	
	long elementCount = listInt->elements;
	int *listArr = listInt->arr;
	for (long k = 0L; k < elementCount; k += 2L) {
		L[listArr[k]]++;
	}

	double lEnd = omp_get_wtime();
	


	// Sestavljamo redko matriko v obliki COO
	// 16 B na element v matriki
	for (long k = 0L; k < elementCount; k += 2L) {
		int from = listArr[k];
		int to = listArr[k+1];
		double val = 1.0 / L[from];

		appendToArrayListCooEntry(listCoo, to, from, val);
	}

	double cooEnd = omp_get_wtime();

	// Free(L); razen ce ga bi rabli na koncu za verifikacijo
	free(L);

	// Free(listInt)
	listArr = NULL;
	freeArrayListInt(listInt);


	// Najprej preverimo ce je sortiranje potrebno
	// Quicksort ima casovno zahtevnost O(n^2) za ze sortirane sezname
	if (!isSortedArrayListCooEntry(listCoo)) {
		// Sort da lahko pole enostavno pretvorimo v CSR format
		// Problem: precej pocasno
		// -> Mogoce paraleliziramo sort?
		//printf("Sorting\n");
		sortArrayListCooEntry(listCoo);
	}
	//printf("Is sorted??? %d\n", isSortedArrayListCooEntry(listCoo));


	double sortEnd = omp_get_wtime();

	MatrixCsr *csrMatrix = cooToCsr(listCoo, steviloVozlisc);

	double csrEnd = omp_get_wtime();

	// Free(listCoo)
	freeArrayListCooEntry(listCoo);

	

	// Podatke smo nalozili in spravili v pravi format
	// -> zacnemo izvajanje algoritma pagerank

	double *prevR = (double*) malloc(sizeof(double) * steviloVozlisc);
	double *currR = (double*) malloc(sizeof(double) * steviloVozlisc);

	double oneOverN = 1.0 / steviloVozlisc;
	for (int i = 0; i < steviloVozlisc; i++) {
		currR[i] = oneOverN;
	}

	double oneMinusDOverN = (1.0 - D) / steviloVozlisc;

	int iterations = 0;

	double *tmp;
	double *csrValues = csrMatrix->values;
	unsigned int *csrColumns = csrMatrix->columnIdx;

	while (normOfVectorDifference(prevR, currR, steviloVozlisc) > EPSILON) {
		iterations++;

		tmp = prevR;
		prevR = currR;
		currR = tmp;

		// Zmnozimo matriko csrMatrix z vektorjem prevR in shranimo v currR
		for (int i = 0; i < steviloVozlisc; i++) {
			currR[i] = 0;

			unsigned int endRowPtrIndex = csrMatrix->rowPtr[i+1];
			for (unsigned int j = csrMatrix->rowPtr[i]; j < endRowPtrIndex; j++) {
				currR[i] = currR[i] + csrValues[j] * prevR[csrColumns[j]];
			}

			currR[i] *= D;
			currR[i] += oneMinusDOverN;
		}
	}
	tmp = NULL;
	csrValues = NULL;
	csrColumns = NULL;

	// Free
	free(prevR);
	freeMatrixCsr(csrMatrix);


	double calculationEnd = omp_get_wtime();

	printf("Read: %.4f\n", readEnd - start);
	printf("Sort: %.4f\n", sortEnd - cooEnd);
	printf("Prep: %.4f\n", csrEnd - readEnd);
	printf("Calc: %.4f\n", calculationEnd - csrEnd);
	printf("Full: %.4f (Read + Prep + Calc)\n", calculationEnd - start);

	printf("Iterations: %d\n", iterations);


#ifdef _OUTPUT_PRINT
	printf("currR = [");
	for (int i = 0; i < steviloVozlisc; i++) {
		if (i != 0) {
			printf(", ");
		}
		printf("%f", currR[i]);
	}
	printf("]\n");
#endif

#ifdef _OUTPUT_CHECK
	double endSum = 0.0;
	double sqSum = 0.0;
	for (int i = 0; i < steviloVozlisc; i++) {
		endSum += currR[i];
		sqSum += currR[i]*currR[i];
	}
	printf("Sum  %.8f\n", endSum);
	printf("Norm %.8f\n", sqrt(sqSum));
#endif


	free(currR);

	return 0;
}


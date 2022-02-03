

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#include "array-list-int.h"
#include "list-coo-entry.h"
#include "matrix-formats.h"


//#define OUTPUT_PRINT
//#define OUTPUT_CHECK
#define CONSIDER_SORT


#define D 0.85
#define EPSILON 1e-8 // Precision
#define EPSILON_SQUARED EPSILON*EPSILON


/** Set at compile time (i.e. -DFORMAT_CSR) **/
//#define FORMAT_CSR
//#define FORMAT_COO
//#define FORMAT_ELL
//#define FORMAT_HYBRID
//#define GET_NNZ_DISTRIBUTION



int main(int argc, char **argv) {

	// Prvi CLI argument je datoteka, oz. pot do datoteke
	if (argc != 2) {
		printf("Error: exactly one argument must be passed\n");
		exit(1);
	}
	printf("Input file: `%s`\n", argv[1]);


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

	ArrayListInt *listInt = initArrayListInt(4095);
	int from, to;
	int readCount = 1;
	int maxVal = 0;

	while (!feof(data_file) && readCount > 0) {
		readCount = fscanf(data_file, "%d %d", &from, &to);

		if (readCount == 2) {
			appendToArrayListInt(listInt, from);
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
	
	unsigned int elementCount = listInt->elements;
	int *listArr = listInt->arr;
	for (unsigned int k = 0u; k < elementCount; k += 2u) {
		L[listArr[k]]++;
	}

	double lEnd = omp_get_wtime();


	/* 
	 * Sestavljamo redko matriko v obliki COO (16 B na element v matriki)
	 *
	 * Zakaj COO?
	 * Ni garantirano, da je vhodna datoteka sortirana, 
	 * format COO pa lahko imamo tudi v nesortirani obliki.
	 * Za pretvorbo v CSR pa potrebujemo sortiran COO, 
	 * Bolj tocno, sortiran kot: primarno -> narascajoce vrstice, sekundarno -> narascajoci stolpci
	 * 
	 * Zakaj tak format (in ne 3 arrayi)?
	 * Za sortiranje z built-in QSORT potrebujemo nek kompakten format
	 */
	ListCooEntry *listCoo = initListCooEntry((listInt->elements) / 2);

	for (unsigned int k = 0u; k < elementCount; k += 2u) {
		int from = listArr[k];
		int to = listArr[k+1];
		double val = 1.0 / L[from];

		appendToListCooEntry(listCoo, to, from, val);
	}

	double cooEnd = omp_get_wtime();

	// Free(L); razen ce ga bi rabli na koncu za verifikacijo
	free(L);

	// Free(listInt)
	listArr = NULL;
	freeArrayListInt(listInt);


	int sortNeeded = 0;
	int sortConsidered = 0;
#ifdef CONSIDER_SORT
	sortConsidered = 1;
	// Sort da lahko potem enostavno pretvorimo v CSR format (oz. kak drug)
	// Najprej preverimo ce je sortiranje potrebno
	// Quicksort ima casovno zahtevnost O(n^2) za ze sortirane sezname
	if (!isSortedListCooEntry(listCoo)) {
		sortListCooEntry(listCoo);
		sortNeeded = 1;
	}
#endif

	double sortEnd = omp_get_wtime();


#ifdef FORMAT_CSR
	MatrixCsr *csrMatrix = compactCooToCsr(listCoo, steviloVozlisc);

#elif defined FORMAT_COO
	MatrixCoo *cooMatrix = compactCooToCoo(listCoo, steviloVozlisc);

#elif defined FORMAT_ELL
	MatrixEll *ellMatrix = compactCooToEll(listCoo, steviloVozlisc);

#elif defined FORMAT_HYBRID
	MatrixEll *ellMatrix = (MatrixEll*) malloc(sizeof(MatrixEll));
	MatrixCoo *cooMatrix = (MatrixCoo*) malloc(sizeof(MatrixCoo));
	compactCooToHybrid(listCoo, ellMatrix, cooMatrix, steviloVozlisc);

#elif defined GET_NNZ_DISTRIBUTION
	// Distribution of nonzero (NNZ) elements in rows
	#warning Maybe fix allocation
	int allocation = 1000000;
	getRowDistribution(listCoo, steviloVozlisc, allocation);
	exit(0);

#else
	#warning Format ni definiran, koda ne bo delovala

#endif

	double convEnd = omp_get_wtime();

	// Free(listCoo)
	freeListCooEntry(listCoo);

	

	// Podatke smo nalozili in spravili v pravi format
	// -> zacnemo izvajanje algoritma pagerank

	double *prevR = (double*) malloc(sizeof(double) * steviloVozlisc);
	double *currR = (double*) malloc(sizeof(double) * steviloVozlisc);

	double *tmp;

	double oneOverN = 1.0 / steviloVozlisc;
	double oneMinusDOverN = (1.0 - D) / steviloVozlisc;

	int iterations = 0;


	// Do & while -> Lazje dodati OpenMP pragme
	double norm;
	double squaredSum;

/* TODO, neka numericna napaka se prikrade, ne vem zakaj
#if defined (FORMAT_COO) || defined(FORMAT_HYBRID)
	// Na tak nacin ne bo potrebe po 2x: #pragma omp atomic
	int maxThreads = omp_get_max_threads();
	unsigned int *threadBoundaries = (unsigned int*) calloc(maxThreads + 1, sizeof(unsigned int));
	threadBoundaries[maxThreads] = cooMatrix->valuesLen;
#endif*/


	#pragma omp parallel
	{

/* TODO, neka numericna napaka se prikrade, ne vem zakaj
#if defined (FORMAT_COO) || defined(FORMAT_HYBRID)
		double *cooValues = cooMatrix->values;
		unsigned int *cooColumns = cooMatrix->columnIdx;
		unsigned int *cooRows = cooMatrix->rowIdx;

		double countPerThread = ((double) cooMatrix->valuesLen) / omp_get_num_threads();
		int threadIndex = omp_get_thread_num();
		unsigned int start = ceil(countPerThread * threadIndex);

		int currRow = cooMatrix->rowIdx[start];

		unsigned int realStart = start;
		if (start > 0u) {
			while (cooMatrix->rowIdx[realStart] == currRow) {
				realStart--;
			}
			realStart++;
		}
		threadBoundaries[threadIndex] = realStart;

		#pragma omp barrier // Dodano za debug
		printf("I am %d; start = %d, end = %d\n", threadIndex, threadBoundaries[threadIndex], threadBoundaries[threadIndex+1]);
		if (threadIndex > 0) {
			printf("I am %d; row-1 = %d, row0 = %d\n", threadIndex, 
					cooMatrix->rowIdx[threadBoundaries[threadIndex]-1], 
					cooMatrix->rowIdx[threadBoundaries[threadIndex]]);
					//threadBoundaries[threadIndex+1]);
		}

#endif*/

		#pragma omp for
		for (int i = 0; i < steviloVozlisc; i++) {
			currR[i] = oneOverN;
		}
		// Implicitni barrier sinhronizira tudi vsebino seznama threadBoundaries


#if defined (FORMAT_COO) || defined (FORMAT_HYBRID)
		double nOverP = ((double) cooMatrix->valuesLen) / omp_get_num_threads();
		int threadIndex = omp_get_thread_num();
		unsigned int start = ceil(nOverP * threadIndex);
		unsigned int end = ceil(nOverP * (threadIndex + 1));

		double *cooValues = cooMatrix->values;
		unsigned int *cooColumns = cooMatrix->columnIdx;
		unsigned int *cooRows = cooMatrix->rowIdx;
		
		//printf("N = %d, P = %d\n", cooMatrix->valuesLen, omp_get_num_threads());
		//printf("I am %d; start = %d, end = %d\n", threadIndex, start, end);
#endif


		do {
			#pragma omp single
			{
				iterations++;

				tmp = prevR;
				prevR = currR;
				currR = tmp;
			}

			squaredSum = 0.0;
			double tmp2;

			// Zmnozimo matriko in vektor prevR ter shranimo v currR
#ifdef FORMAT_CSR
			double *csrValues = csrMatrix->values;
			unsigned int *csrColumns = csrMatrix->columnIdx;
			
			#pragma omp for schedule(guided,1024)
			for (int i = 0; i < steviloVozlisc; i++) {
				currR[i] = 0.0;

				unsigned int endRowPtrIndex = csrMatrix->rowPtr[i+1];
				for (unsigned int j = csrMatrix->rowPtr[i]; j < endRowPtrIndex; j++) {
					currR[i] = currR[i] + csrValues[j] * prevR[csrColumns[j]];
				}

				currR[i] *= D;
				currR[i] += oneMinusDOverN;
			}
#endif

#if defined (FORMAT_ELL) || (FORMAT_HYBRID)
			// ELL part, for pure ELL or for HYBRID
			int rows = ellMatrix->rows;
			long columnsPerRow = (long) ellMatrix->columnsPerRow;
			long effectiveIndex;
			double current;
			int column;

			#pragma omp for
			for (long i = 0L; i < steviloVozlisc; i++) {
				currR[i] = 0.0;

				for (long j = 0L; j < columnsPerRow; j++) {
					
	#ifdef ELL_CPU_OPTIMIZE
					effectiveIndex = i*columnsPerRow + j;
	#else	// za GPU
					effectiveIndex = j*rows + i;
	#endif

					column = ellMatrix->columnIdx[effectiveIndex];
					if (column == 0u) {
						break;
					}
					currR[i] += ellMatrix->values[effectiveIndex] * prevR[column-1u];
				}

				currR[i] *= D;
				currR[i] += oneMinusDOverN;
			}
#endif

#if defined (FORMAT_COO)
			#pragma omp for
			for (int i = 0; i < steviloVozlisc; i++) {
				currR[i] = oneMinusDOverN;
			}
#endif
#if defined (FORMAT_COO) || (FORMAT_HYBRID)
/* TODO, neka numericna napaka se prikrade, ne vem zakaj
			unsigned int myStart = threadBoundaries[threadIndex];
			unsigned int myEnd = threadBoundaries[threadIndex+1];

			unsigned int currentRow = cooRows[myStart];
			double accumulator = D * cooValues[myStart] * prevR[cooColumns[myStart]];

			for (unsigned int i = myStart+1u; i < myEnd; i++) {
				unsigned int thisRow = cooRows[i];
				if (thisRow > currentRow) {
					//printf("I=(%d), R=(%d) Accumulator [%d] = %.8f\n", iterations, currentRow, threadIndex, accumulator);
					currR[currentRow] += accumulator;
					accumulator = 0.0;
					currentRow = thisRow;
				}
				accumulator += D * cooValues[i] * prevR[cooColumns[i]];
			}
			#pragma omp barrier
*/

			// This offers a less impressive speedup
			/*#pragma omp for
			for (unsigned int i = 0u; i < cooMatrix->valuesLen; i++) {
				#pragma atomic
				currR[cooRows[i]] += D * cooValues[i] * prevR[cooColumns[i]];
			}*/
			
			unsigned int currentRow = cooRows[start];
			double accumulator = D * cooValues[start] * prevR[cooColumns[start]];

			// To izvede vsaka nit s svojimi indeksi
			// Prvo in zadnjo vrstico moramo prišteti z atomic
			int isFirstRowForThisThread = 1;
			for (unsigned int i = start+1u; i < end; i++) {
				unsigned int thisRow = cooRows[i];
				if (thisRow > currentRow) {
					if (isFirstRowForThisThread) {
						isFirstRowForThisThread = 0;
						#pragma omp atomic
						currR[currentRow] += accumulator;
					} else {
						currR[currentRow] += accumulator;
					}
					accumulator = 0.0;
					currentRow = thisRow;
				}
				accumulator += D * cooValues[i] * prevR[cooColumns[i]];
			}
			#pragma omp atomic
			currR[currentRow] += accumulator;

			#pragma omp barrier
#endif

			// Izracun norme
			#pragma omp for reduction(+:squaredSum)
			for (int i = 0; i < steviloVozlisc; i++) {
				tmp2 = (prevR[i] - currR[i]);
				squaredSum += tmp2*tmp2;
			}
		} while (squaredSum > EPSILON_SQUARED);
		// Konec iteracije
	}


	// Free
	free(prevR);
	tmp = NULL;

#ifdef FORMAT_CSR
	freeMatrixCsr(csrMatrix);
#elif defined FORMAT_COO
	freeMatrixCoo(cooMatrix);
#elif defined FORMAT_ELL
	freeMatrixEll(ellMatrix);
#elif defined FORMAT_HYBRID
	freeMatrixCoo(cooMatrix);
	freeMatrixEll(ellMatrix);
#endif


	double calculationEnd = omp_get_wtime();

	double read = readEnd - start;
	double prep = sortEnd - readEnd;
	double conv = convEnd - sortEnd;
	double calc = calculationEnd - convEnd;

	printf("\nRead: %.4f\n", read);
	printf("Prep: %.4f\n", prep);
	printf("Conv: %.4f\n", conv);
	printf("Calc: %.4f\n\n", calc);

	printf("Sequential:     %.4f (Read + Prep)\n", (read + prep));
	printf("Parallelizable: %.4f (Conv + Calc)\n", (conv + calc));
	printf("Total:          %.4f (Read + Prep + Conv + Calc)\n\n", (read + prep + conv + calc));

	printf("Sorting considered? %d\n", sortConsidered);
	printf("Sorting needed? %d\n", sortNeeded);
	printf("Iterations: %d\n\n", iterations);

	printf("%d, %d, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n",
				iterations,
				(sortConsidered + sortNeeded),
				read,
				prep,
				conv,
				calc,
				read + prep,
				conv + calc,
				read + prep + conv + calc);


#ifdef OUTPUT_PRINT
	printf("currR = [");
	for (int i = 0; i < steviloVozlisc; i++) {
		if (i != 0) {
			printf(", ");
		}
		printf("%f", currR[i]);
	}
	printf("]\n");
#endif

#ifdef OUTPUT_CHECK
	double endSum = 0.0;
	double sqSum = 0.0;
	for (int i = 0; i < steviloVozlisc; i++) {
		endSum += currR[i];
		sqSum += currR[i]*currR[i];
	}
	printf("Sum   %.15f\n", endSum);
	printf("SqSum %.15f\n", sqSum);
	printf("Norm  %.15f\n", sqrt(sqSum));
#endif


	free(currR);

	return 0;
}

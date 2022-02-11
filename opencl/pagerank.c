

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <CL/cl.h>

#include "array-list-int.h"
#include "list-coo-entry.h"
#include "matrix-formats.h"


//#define OUTPUT_PRINT
#define OUTPUT_CHECK
#define CONSIDER_SORT


#define D 0.85
#define EPSILON 1e-8


/** Set at compile time (i.e. -DFORMAT_CSR) **/
//#define FORMAT_CSR
//#define FORMAT_COO
//#define FORMAT_ELL
//#define FORMAT_HYBRID
//#define GET_NNZ_DISTRIBUTION

#define SIZE			(1024)
#define WORKGROUP_SIZE	(256)
#define MAX_SOURCE_SIZE (16384)


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

	char *source_str;
	size_t source_size;

	// branje datoteke
    fp = fopen("kernel.cl", "r");
    if(!fp)
    {
        fprintf(stderr, ":-(\n");
        return 1;
    }
    source_str = (char*)malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    source_str[source_size] = '\0';
    fclose(fp);

    // Podatki o platformi
	cl_platform_id	platform_id[10];
	cl_uint			ret_num_platforms;
	char			*buf;
	size_t			buf_len;
	ret = clGetPlatformIDs(10, platform_id, &ret_num_platforms);

    // Podatki o napravi
	cl_device_id	device_id[10];
	cl_uint			ret_num_devices;
	ret = clGetDeviceIDs(platform_id[0], CL_DEVICE_TYPE_GPU, 10, device_id, &ret_num_devices);

    // Kontekst
	cl_context context = clCreateContext(NULL, 1, &device_id[0], NULL, NULL, &ret);
	
    // Ukazna vrsta
	cl_command_queue command_queue = clCreateCommandQueue(context, device_id[0], 0, &ret);

    // TO_DO: Delitev dela 
	int vectorSize = SIZE;
	size_t local_item_size = WORKGROUP_SIZE;
	size_t num_groups = ((vectorSize - 1) / local_item_size + 1);
	size_t global_item_size = num_groups * local_item_size;


	//rezervacija pomnilnika
	double *csrValues = csrMatrix->values; //matrika
	unsigned int *csrColumns = csrMatrix->columnIdx; //vektor
	unsigned int endRowPtrIndex = csrMatrix->rowPtr; 
	int rPtrLen = csrMatrix->rowPtrLen;
	unsigned int *valLen = csrMatrix->valuesLen;

	double *values = malloc(csrValues * sizeof(double)); //what is the size
	unsigned int *columnIdx = malloc(csrColumns * sizeof(unsigned int));
	unsigned int *rowPtr = malloc(sizeof(unsigned int));
	double *prejsna = malloc(steviloVozlisc * sizeof(double));
	double *trenutna = malloc(steviloVozlisc * sizeof(double));


	int *rowPtrLen = malloc(sizeof(int)); //needed?
	unsigned int valuesLen = malloc(sizeof(unsigned int)); //needed?

	//alokacija pomnilnika na napravi
	cl_mem val_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, csrValues * sizeof(double), csrValues, &ret);
	cl_mem col_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, csrColumns * sizeof(unsigned int), csrColumns, &ret);
	cl_mem rowPtr_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, endRowPtrIndex * sizeof(unsigned int), endRowPtrIndex, &ret);
	cl_mem prej_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(double) * steviloVozlisc, prejsna, &ret);
	cl_mem tren_mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(double) * steviloVozlisc, trenutna, &ret);
	
	//cl_mem rowPtrLen_mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned int), rowPtrLen, &ret);

	// Priprava programa
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source_str, NULL, &ret);

    // Prevajanje
	ret = clBuildProgram(program, 1, &device_id[0], NULL, NULL, NULL);

	// "s"cepec: priprava objekta
	cl_kernel kernel = clCreateKernel(program, "pagerank", &ret);

	// "s"cepec: argumenti
	//podamo en double, 2 pointerja na uint in en uint
	ret = clSetKernelArg(kernel, 0, sizeof(cl_double), &csrValues);
	ret |= clSetKernelArg(kernel, 1, sizeof(cl_uint), &csrColumns);
	ret |= clSetKernelArg(kernel, 2, sizeof(cl_uint), &endRowPtrIndex);
	ret |= clSetKernelArg(kernel, 3, sizeof(cl_int), &rPtrLen);
	ret |= clSetKernelArg(kernel, 4, sizeof(cl_uint), &valLen);
	ret |= clSetKernelArg(kernel, 5, sizeof(cl_double), (void *)&prej_mem_obj);
	ret |= clSetKernelArg(kernel, 6, sizeof(cl_double), (void *)&tren_mem_obj);

	// "s"cepec: zagon
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_item_size, local_item_size, 0, NULL, NULL);

	// Kopiranje rezultatov
	//ret = clEnqueueReadBuffer(command_queue, c_mem_obj, CL_TRUE, 0, h * w * sizeof(float), C, 0, NULL, NULL);

	// "ci"s"cenje
	ret = clFlush(command_queue);
	ret = clFinish(command_queue);
	ret = clReleaseKernel(kernel);
	ret = clReleaseProgram(program);
	ret = clReleaseMemObject(val_mem_obj);
	ret = clReleaseMemObject(col_mem_obj);
	ret = clReleaseMemObject(rowPtr_mem_obj);
	ret = clReleaseMemObject(prej_mem_obj);
	ret = clReleaseMemObject(tren_mem_obj);
	ret = clReleaseCommandQueue(command_queue);
	ret = clReleaseContext(context);



	// Do & while -> Lazje dodati OpenMP pragme
	double norm;
	double squaredSum;

	for (int i = 0; i < steviloVozlisc; i++) {
		currR[i] = oneOverN;
	}

	do {
		iterations++;

		tmp = prevR;
		prevR = currR;
		currR = tmp;

		squaredSum = 0.0;
		double tmp2;

		// Zmnozimo matriko in vektor prevR ter shranimo v currR
#if defined (FORMAT_CSR)
		double *csrValues = csrMatrix->values;
		unsigned int *csrColumns = csrMatrix->columnIdx;

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
		double *cooValues = cooMatrix->values;
		unsigned int *cooColumns = cooMatrix->columnIdx;
		unsigned int *cooRows = cooMatrix->rowIdx;

		for (int i = 0; i < steviloVozlisc; i++) {
			currR[i] = oneMinusDOverN;
		}
		for (unsigned int i = 0u; i < cooMatrix->valuesLen; i++) {
			currR[cooRows[i]] += D * cooValues[i] * prevR[cooColumns[i]];
		}

#elif defined (FORMAT_HYBRID)
		// COO part of HYBRID
		double *cooValues = cooMatrix->values;
		unsigned int *cooColumns = cooMatrix->columnIdx;
		unsigned int *cooRows = cooMatrix->rowIdx;

		for (unsigned int i = 0u; i < cooMatrix->valuesLen; i++) {
			currR[cooRows[i]] += D * cooValues[i] * prevR[cooColumns[i]];
		}
#endif

		// Izracun norme
		for (int i = 0; i < steviloVozlisc; i++) {
			tmp2 = (prevR[i] - currR[i]);
			squaredSum += tmp2*tmp2;
		}
		norm = sqrt(squaredSum);

	} while (norm > EPSILON);
	// Konec iteracije


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
	printf("Sum  %.8f\n", endSum);
	printf("Norm %.8f\n", sqrt(sqSum));
#endif


	free(currR);

	return 0;
}

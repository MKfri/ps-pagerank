
.PHONY: all
all: pagerank-csr pagerank-hybrid

.PHONY: clean
clean:
	rm pagerank-csr pagerank-hybrid

pagerank-csr: pagerank.c list-coo-entry.c array-list-int.c matrix-formats.c
	gcc pagerank.c \
		list-coo-entry.c \
		array-list-int.c \
		matrix-formats.c \
		-DFORMAT_CSR \
		-lOpenCL -O2 \
		-lm -fopenmp \
		-o pagerank-csr


pagerank-coo: pagerank.c list-coo-entry.c array-list-int.c matrix-formats.c
	gcc pagerank.c \
		list-coo-entry.c \
		array-list-int.c \
		matrix-formats.c \
		-DFORMAT_COO \
		-lOpenCL -O2 \
		-lm -fopenmp \
		-o pagerank-coo


pagerank-hybrid: pagerank.c list-coo-entry.c array-list-int.c matrix-formats.c
	gcc pagerank.c \
		list-coo-entry.c \
		array-list-int.c \
		matrix-formats.c \
		-DFORMAT_HYBRID \
		-lOpenCL -O2 \
		-lm -fopenmp \
		-o pagerank-hybrid



.PHONY: all
all: pagerank-csr pagerank-coo pagerank-ell pagerank-hybrid

pagerank-csr: pagerank.c list-coo-entry.c array-list-int.c matrix-formats.c
	gcc pagerank.c \
		list-coo-entry.c \
		array-list-int.c \
		matrix-formats.c \
		-DFORMAT_CSR \
		-lm -fopenmp -O2 \
		-o pagerank-csr

pagerank-coo: pagerank.c list-coo-entry.c array-list-int.c matrix-formats.c
	gcc pagerank.c \
		list-coo-entry.c \
		array-list-int.c \
		matrix-formats.c \
		-DFORMAT_COO \
		-lm -fopenmp -O2 \
		-o pagerank-coo

pagerank-ell: pagerank.c list-coo-entry.c array-list-int.c matrix-formats.c
	gcc pagerank.c \
		list-coo-entry.c \
		array-list-int.c \
		matrix-formats.c \
		-DFORMAT_ELL \
		-lm -fopenmp -O2 \
		-o pagerank-ell

pagerank-hybrid: pagerank.c list-coo-entry.c array-list-int.c matrix-formats.c
	gcc pagerank.c \
		list-coo-entry.c \
		array-list-int.c \
		matrix-formats.c \
		-DFORMAT_HYBRID \
		-lm -fopenmp -O2 \
		-o pagerank-hybrid


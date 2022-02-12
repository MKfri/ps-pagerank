#gcc pagerank.c -O2 -lm -lOpenCL -o pagerank-csr
#    -DFORMAT_CSR \


gcc pagerank.c \
    list-coo-entry.c \
    array-list-int.c \
    matrix-formats.c \
    -lm -fopenmp \
    -lOpenCL -O2 \
    -o pagerank-coo

    #-o pagerank-csr


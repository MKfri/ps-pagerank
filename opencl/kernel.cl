__kernel void pagerank(__global double *values, 
                        __global unsigned int *columnIdx, 
                        __global unsigned int *rowPtr, 
                        __global int rowPtrLen,
                        __global unsigned int *valuesLen,
                        __global unsigned double *prevR,
                        __global unsigned double *currR) {


    int i = get_local_id(0);
    int j = get_local_id(1);
    int gid = get_global_id(0);
    double *csrValues = values;
    unsigned int *csrColumns = columnIdx;

    while(gid < rowPtrLen) {
        
        currR[i] = 0.0;

        unsigned int endRowPtrIndex = rowPtr[i+1];

        for (unsigned int j = rowPtr[i]; j < endRowPtrIndex; j++) {
            currR[i] = currR[i] + csrValues[j] * prevR[csrColumns[j]];
        }

        currR[i] *= D;
        currR[i] += oneMinusDOverN;
    }
}

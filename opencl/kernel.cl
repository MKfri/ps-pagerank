__kernel void pagerank(__global double *values, __global unsigned int *columnIdx, __global unsigned int*rowIdx, __global unsigned int valuesLen) {
    double *csrValues = csrMatrix->values;
    unsigned int *csrColumns = csrMatrix->columnIdx;
    int i = get_global_id(0);

    currR[i] = 0.0;

    unsigned int endRowPtrIndex = csrMatrix->rowPtr[i+1];

    for (unsigned int j = csrMatrix->rowPtr[i]; j < endRowPtrIndex; j++) {
        currR[i] = currR[i] + csrValues[j] * prevR[csrColumns[j]];
    }

    currR[i] *= D;
    currR[i] += oneMinusDOverN;
}

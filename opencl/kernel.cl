#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void pagerank(__global double *values, 
                        __global unsigned int *columnIdx, 
                        __global unsigned int *rowPtr, 
                        int matrixSize,
                        double D,
                        double oneMinusDOverN,
                        __global double *prevR,
                        __global double *currR) {

    int i = get_local_id(0);
    int gid = get_global_id(0);

    if (gid < matrixSize) {
        double newCurrR = 0.0;

        unsigned int startRowPtrIndex = rowPtr[gid];
        unsigned int endRowPtrIndex = rowPtr[gid+1];

        for (unsigned int j = startRowPtrIndex; j < endRowPtrIndex; j++) {
            newCurrR += values[j] * prevR[columnIdx[j]];
        }

        newCurrR *= D;
        newCurrR += oneMinusDOverN;
        currR[gid] = newCurrR;
    }
}

#pragma OPENCL EXTENSION cl_khr_fp64 : enable


__kernel void pagerank(__global float *csrValues,
                        __global unsigned int *csrColumnIdx,
                        __global unsigned int *csrRowPtr,

                        __global float *ellValues,
                        __global unsigned int *ellColumnIdx,

                        int matrixSize,
                        unsigned int columnsPerRow, 
                        float D,
                        float oneMinusDOverN,
                        
                        __global float *prevR,
                        __global float *currR,
                        __global float *workGroupSums,
                        __local  float *localSquaredSums) {


    int lid = get_local_id(0);
    int gid = get_global_id(0);

    float diff = 0.0;

    if (gid < matrixSize) {
        // ELL
        float newCurrR = 0.0;
        long effectiveIndex;
        unsigned int column;

        for (long j = 0L; j < columnsPerRow; j++) {
            effectiveIndex = j * matrixSize + gid;
            column = ellColumnIdx[effectiveIndex];
            if(column == 0u) break;

            newCurrR += ellValues[effectiveIndex] * prevR[column-1u];
        }
        
        // CSR
        unsigned int startRowPtrIndex = csrRowPtr[gid];
        unsigned int endRowPtrIndex = csrRowPtr[gid+1];

        for (unsigned int j = startRowPtrIndex; j < endRowPtrIndex; j++) {
            newCurrR += csrValues[j] * prevR[csrColumnIdx[j]];
        }

        // Set global mem and init norm computation
        newCurrR *= D;
        newCurrR += oneMinusDOverN;
        currR[gid] = newCurrR;
        diff = newCurrR - prevR[gid];
    }


    localSquaredSums[lid] = diff*diff;


    barrier(CLK_LOCAL_MEM_FENCE);

    int floorPow2 = get_local_size(0);
	while (floorPow2 & (floorPow2-1))
		floorPow2 &= floorPow2-1;

	if (get_local_size(0) != floorPow2) {
		if (lid >= floorPow2)
            localSquaredSums[lid - floorPow2] += localSquaredSums[lid];
		barrier(CLK_LOCAL_MEM_FENCE);
    }

    for (int idxStep = floorPow2>>1; idxStep > 0; idxStep >>= 1) {
		if (lid < idxStep)
			localSquaredSums[lid] += localSquaredSums[lid+idxStep];
		barrier(CLK_LOCAL_MEM_FENCE);
	}

	if (lid == 0) {		
		workGroupSums[get_group_id(0)] = localSquaredSums[0];
	}
}
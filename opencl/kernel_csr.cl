#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void pagerank(__global double *values, 
                        __global unsigned int *columnIdx, 
                        __global unsigned int *rowPtr, 
                        int matrixSize,
                        double D,
                        double oneMinusDOverN,
                        __global double *prevR,
                        __global double *currR,
                        __global double *workGroupSums,
                        __local  double *localSquaredSums) {

    int lid = get_local_id(0);
    int gid = get_global_id(0);
    double diff = 0.0;

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
            
        diff = (currR[gid] - prevR[gid]);
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

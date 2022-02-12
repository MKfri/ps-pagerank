__kernel void pagerank(__global double *values, 
                        __global unsigned int *columnIdx, 
                        __global unsigned int *rowIdx, 
                        int matrixSize,
                        double D,
                        double oneMinusDOverN,
                        unsigned int valuesLen,
                        __global double *prevR,
                        __global double *currR,
                        __global double *workGroupSums,
                        __local  double *localSquaredSums) {

    
    int lid = get_local_id(0);
    int gid = get_global_id(0);
    double diff = 0.0;

    if(gid < matrixSize) {
        double newCurrR = 0.0;

        for (unsigned int i = 0u; i < valuesLen; i++) {
			    currR[rowIdx[i]] += D * values[i] * prevR[columnIdx[i]];
        }
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
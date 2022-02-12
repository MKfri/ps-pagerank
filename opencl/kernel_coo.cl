#pragma OPENCL EXTENSION cl_khr_fp64 : enable
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable

/*
void my_atomic_add(__global double *location, const double value) {
    double old = *location;
    double updated = old + value;

    while (atom_cmpxchg(
        (__global long*) location,
        *((long*)&old),
        *((long*)&updated)
    )) {
        old = *location;
        updated = old + value;
    }
}*/
void AtomicAdd(__global double *val, double delta) {
    union {
        double f;
        ulong  i;
    } old;

    union {
        double f;
        ulong  i;
    } new;

    do {
        old.f = *val;
        new.f = old.f + delta;
    } while (atom_cmpxchg ( (volatile __global ulong *)val, old.i, new.i) != old.i);
}



__kernel void pagerank(__global double *values, 
                        __global unsigned int *columnIdx, 
                        __global unsigned int *rowIdx, 
                        int numOfNonzeros, // TODO cast v int na drugi strani, al pa ne
                        double D,
                        double oneMinusDOverN,
                        //unsigned int valuesLen,
                        __global double *prevR,
                        __global double *currR) {
                        //__global double *workGroupSums,
                        //__local  double *localSquaredSums) {

    int lid = get_local_id(0);
    int gid = get_global_id(0);
    double diff = 0.0;

    if (gid < numOfNonzeros) {
        unsigned int thisRow = rowIdx[gid];
        unsigned int thisCol = columnIdx[gid];

        double thisVal = values[gid];

        AtomicAdd(&currR[thisRow], D * thisVal * prevR[thisCol]);
    }

    /*if (gid < steviloVozlisc) {

    }

    / *if (gid < numOfNonzeros) {
        unsigned int thisRow = rowIdx[gid];
        / *unsigned int thisCol = columnIdx[gid];

        double thisVal = values[gid];
        * /
        AtomicAdd(&currR[thisRow], 0.0); //thisVal * prevR[thisCol]);
    }*/



 
/*
    if (gid < numOfNonzeros) {
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
	}*/
}


__kernel void normAndInit(int vectorSize,
                        __global double *prevR,
                        __global double *currR,
                        __global double *workGroupSums,
                        __local  double *localSquaredSums,
                        double oneMinusDOverN) {
    
    int lid = get_local_id(0);
    int gid = get_global_id(0);

    double diff = 0.0;
    if (gid < vectorSize) {
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

    if (gid < vectorSize) {
        prevR[gid] = oneMinusDOverN;
    }
}
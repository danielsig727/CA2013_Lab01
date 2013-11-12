__kernel void reduction_worker(
    __global int *data,
    ulong idxMax,
    uint level)
{
    // calculate the index to store sum
    size_t idx = get_global_id(0);
    idx = idx << level;

    // calculate the index to add to the sum
    unsigned int idxShift = idx + (1 << (level-1));
    if(idxShift >= (size_t) idxMax)
    {
        return;
    }

    // summing
    data[idx] += data[idxShift];
}

__kernel void reduction_worker_scheduler(
    __global int *data,
    ulong idxMax,
    uint level,
    ulong idxStep
)
{
    // calculate the index to store sum
    size_t idx = get_global_id(0);
    idx = idx << level;
    idxStep = idxStep << level;
    for(; idx < idxMax; idx += idxStep)
    {

        // calculate the index to add to the sum
        unsigned int idxShift = idx + (1 << (level-1));
        if(idxShift >= (size_t) idxMax)
        {
            return;
        }

        // summing
        data[idx] += data[idxShift];
    }
}

__kernel void reduction_v2(
    __global int4 *data,
	__global int *gsum,
	__local int *lsum,
    uint nitems)
{
// Calculate local sum
	uint idxMax = nitems/4;
	uint vec_per_worker = idxMax / get_global_size(0);
	uint idx = get_global_id(0) * vec_per_worker;
	int4 psumv = 0;
	int psum;
	for( ; idx < idxMax ; ++idx ){
		 psumv += data[idx];
	}
	psum = psumv.x + psumv.y + psumv.z + psumv.w;

// Group reduction
	if( get_local_id(0) == 0 )
		lsum[0] = 0;

	barrier( CLK_LOCAL_MEM_FENCE ); 
	(void) atomic_add( lsum, psum ); 
	barrier( CLK_LOCAL_MEM_FENCE );

// Write to global mem
	
	if( get_local_id(0) == 0) 
		gsum[ get_group_id(0) ] = lsum[0];

}

__kernel void reduction_v2_finalize(
	__global int *gsum
	)
{
	(void) atomic_add( gsum, gsum[get_global_id(0)]);
}

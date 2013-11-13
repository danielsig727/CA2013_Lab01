__kernel void reduction_v2(
    __global int4 *data,
    __global int *gsum,
    __local int *lsum,
    ulong nitems)
{
// Calculate local sum
    uint idxMax = nitems/4;
    if( idxMax % get_global_size(0) != 0)
        idxMax += get_global_size(0) - (idxMax % get_global_size(0));
    uint vec_per_worker = idxMax / get_global_size(0);
    uint idx = get_global_id(0) * vec_per_worker;
    idxMax = min((uint) nitems/4, idx + vec_per_worker);
    int psum = 0;
    for( ; idx < idxMax ; ++idx ) {
        psum += data[idx].x;
        psum += data[idx].y;
        psum += data[idx].z;
        psum += data[idx].w;
    }

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
    if(get_global_id(0) != 0)
        (void) atomic_add( gsum, gsum[get_global_id(0)]);
}

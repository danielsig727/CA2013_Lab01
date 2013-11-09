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
    if(idxShift >= (size_t) idxMax){
        return;
    }

    // summing
    data[idx] += data[idxShift];
//    data[idx] = 0;
}


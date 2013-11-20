#ifndef __REDUCTION_HH_UQLYSFW6__
#define __REDUCTION_HH_UQLYSFW6__

#include "CL/cl.h"
#include <pthread.h>
#define MIN(x, y) (((x) < (y))?(x):(y))
#define MAX(x, y) (((x) > (y))?(x):(y))

class OclAddReduce
{
public:
    OclAddReduce( int* host_data )
        : mHostData( host_data )
    {
        pthread_mutex_init( &resultLock, NULL );
    };
    ~OclAddReduce()
    {
        clear();
    };
    void run();
    void run_cpu(size_t dsize, int *data);
    void run_gpu(size_t dsize, int *data);
    void getResult_gpu();
    int getResult();
private:
    OclAddReduce( const OclAddReduce & );
    const OclAddReduce& operator=( const OclAddReduce & );
    void initHost();
    void endHost();
    void initPlatform();
    void initDevice();
    void showInfo();
    void initContext();
    void initCommandQ();
    void initDeviceMem(size_t size);
    void initKernel();
    void runKernel_knl(size_t num_src_items);
    void runKernel(size_t num_src_items, size_t dsize, size_t step, int *data);
    void clear();

    /* Data */
    int* mHostData;
    cl_platform_id *mPlatform;
    cl_device_id *mDevice;
    cl_context mContext;
    cl_command_queue mCommandQ;
    cl_program mProgram;
    cl_kernel mKernel;
    cl_kernel mKernel2;

    /*Custom Data*/
    cl_mem mData;
    cl_mem mGrpResult;
    cl_uint mComputeUnits;
    //size_t num_src_items;
    int result;
    pthread_mutex_t resultLock;
    size_t global_work_size;
    size_t local_work_size;
    size_t num_groups;

    struct t_params {
        OclAddReduce *obj;
        size_t size;
        int *data;
        char is_cpu;
    };
    static void* run_wrapper(void *p );

};

#endif /* end of include guard: REDUCTION_HH_UQLYSFW6 */


#ifndef __REDUCTION_HH_UQLYSFW6__
#define __REDUCTION_HH_UQLYSFW6__

#include "CL/cl.h"
#define MIN(x, y) (((x) < (y))?(x):(y))
#define MAX(x, y) (((x) > (y))?(x):(y))

class OclAddReduce
{
public:
    OclAddReduce( int* host_data )
        : mHostData( host_data )
    {};
    ~OclAddReduce()
    {
        clear();
    };
    void run();
    void run_cpu();
    void run_gpu();
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
    void runKernel(size_t num_src_items, size_t step);
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
    size_t num_src_items;
    int result;
    size_t global_work_size;
    size_t local_work_size;
    size_t num_groups;

};

#endif /* end of include guard: REDUCTION_HH_UQLYSFW6 */


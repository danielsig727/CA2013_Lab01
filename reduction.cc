#include <iostream>
#include <cassert>
#include <cstring>
#include "utils.hh"
#include "defines.hh"
#include "reduction.hh"
#include <omp.h>


/*
 * The following function is only used to check whether certain command is correctly executed
 * For more information, please refers to
 *     http://www.khronos.org/registry/cl/sdk/1.2/docs/man/xhtml/
 */
void
errVerify( cl_int status, std::string msg = "" )
{
    using std::cerr;
    using std::endl;
    if( status != CL_SUCCESS )
    {
        cerr<<"There is something error("<<status<<", "<<msg<<")"<<endl;
        cerr.flush();
        exit(-1);
    }
}

void
OclAddReduce::run()
{
    struct t_params p_cpu, p_gpu;
    p_cpu.obj = p_gpu.obj = this;
    p_cpu.is_cpu = 1;
    p_gpu.is_cpu = 0;
    p_cpu.size = p_cpu.size = 0;

    if( DATA_SIZE >= 400000000 ) {
        p_gpu.size = ((DATA_SIZE * 2/9) >> 2) << 2; // /4 and mod 4 = 0
        p_cpu.size = DATA_SIZE - p_gpu.size;
        p_gpu.data = mHostData;
        p_cpu.data = mHostData + p_gpu.size;

        pthread_t t_gpu;
        pthread_create(&t_gpu, NULL, (void* (*)(void*)) &run_wrapper, (void*) &p_gpu);

        std::cout<<"(cpu)"<<std::flush;
        run_cpu(p_cpu.size, p_cpu.data);
        std::cout<<"(cpu_finished)"<<std::flush;

        pthread_join(t_gpu, NULL);
    } else {
        run_cpu(DATA_SIZE, mHostData);
    }
}

void*
OclAddReduce::run_wrapper( void *pv )
{
    struct t_params *p = (struct t_params*) pv;
    if( p->size <= 0)
        pthread_exit(NULL);

    if( p->is_cpu ) {
        std::cout<<"(cpu)";
        p->obj->run_cpu(p->size, p->data);
        std::cout<<"(cpu_finished)"<<std::flush;
    } else {
        std::cout<<"(gpu)";
        p->obj->run_gpu(p->size, p->data);
        std::cout<<"(gpu_finished)"<<std::flush;
    }
    pthread_exit(NULL);
}


void
OclAddReduce::run_cpu(size_t dsize, int *data)
{
    int rst = 0;
    omp_set_num_threads(8);

    #pragma omp parallel for reduction(+:rst)
    for(int i=0; i<dsize; i++) {
        rst = rst +  data[i];
    }

    pthread_mutex_lock(&resultLock);
    result += rst;
    pthread_mutex_unlock(&resultLock);
}

void
OclAddReduce::run_gpu(size_t dsize, int *data)
{

    result = 0;

    /*Step 1: dectect & initialize platform*/
    initPlatform();

    /*Step 2: detect & initialize device*/
    initDevice();

    /*TA's Information Show Function*/
    showInfo();

    /*Step 3: create a context*/
    initContext();

    /*Step 4: create a command queue*/
    initCommandQ();

    int num_src_items = dsize + ((dsize%4)?(4-dsize%4):0);
    size_t step = 125000000;

    /*Step 5: create device buffers*/
    initDeviceMem( MIN(num_src_items, step) );

    /*Step 6: build program, and then create & set kernel*/
    initKernel();

    /*Step 7: run kernel*/
    runKernel(num_src_items, dsize, step, data);

}

void
OclAddReduce::getResult_gpu()
{
    int rst;
    clEnqueueReadBuffer( mCommandQ, mGrpResult, CL_TRUE,
                         0, sizeof(int), &rst, 0, NULL, NULL );
    pthread_mutex_lock(&resultLock);
    result += rst;
    pthread_mutex_unlock(&resultLock);
}

int
OclAddReduce::getResult()
{
    return result;
}

void
OclAddReduce::initPlatform()
{
    cl_uint numPlatforms = 0;

    clGetPlatformIDs( 0, NULL, &numPlatforms );
    mPlatform = new cl_platform_id[ numPlatforms ];
    clGetPlatformIDs( numPlatforms, mPlatform, NULL );
}

void
OclAddReduce::initDevice()
{
    cl_uint numDevices = 0;

    clGetDeviceIDs( mPlatform[0], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices );
    mDevice = new cl_device_id[ numDevices ];
    clGetDeviceIDs( mPlatform[0], CL_DEVICE_TYPE_GPU, numDevices, mDevice, NULL );

    clGetDeviceInfo( *mDevice, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &mComputeUnits, NULL);
}

void
OclAddReduce::showInfo()
{
    using std::cout;
    using std::endl;

    size_t msg_size;
    char* msg;

    cout<<"*********************OpenCL Information*********************"<<endl;
    clGetPlatformInfo( *mPlatform, CL_PLATFORM_VENDOR, 0, NULL, &msg_size );
    msg = new char[ msg_size ];
    clGetPlatformInfo( *mPlatform, CL_PLATFORM_VENDOR, msg_size, (void*) msg, NULL );
    cout<<"Vendor: "<<msg<<endl;
    delete [] msg;

    clGetPlatformInfo( *mPlatform, CL_PLATFORM_VERSION, 0, NULL, &msg_size );
    msg = new char[ msg_size ];
    clGetPlatformInfo( *mPlatform, CL_PLATFORM_VERSION, msg_size, (void*) msg, NULL );
    cout<<"Version info: "<<msg<<endl;
    delete [] msg;

    clGetDeviceInfo( *mDevice, CL_DEVICE_NAME, 0, NULL, &msg_size );
    msg = new char[ msg_size ];
    clGetDeviceInfo( *mDevice, CL_DEVICE_NAME, msg_size, (void*) msg, NULL );
    cout<<"Device: "<<msg<<endl;

    cout<<"Number of compute units: "<<mComputeUnits<<endl;

    cl_bool ava;
    clGetDeviceInfo( *mDevice, CL_DEVICE_AVAILABLE, sizeof( cl_bool), (void*) &ava, NULL);
    cout<<"Available: ";
    if( ava == CL_TRUE )
    {
        cout<<"YES"<<endl;
    }
    else
    {
        cout<<"NO"<<endl;
    }

    cout<<"************************************************************"<<endl;
    cout<<endl;
}

void
OclAddReduce::initContext()
{
    cl_int status;
    mContext = clCreateContext( NULL, 1, mDevice, NULL, NULL, &status );
    errVerify( status, "initContext" );
}

void
OclAddReduce::initCommandQ()
{
    mCommandQ = clCreateCommandQueue( mContext, mDevice[0], 0, NULL );
}

void
OclAddReduce::initDeviceMem(size_t size)
{
    cl_int status;
    mData = clCreateBuffer( mContext, CL_MEM_READ_ONLY,
                            size * sizeof(int), NULL, &status );

    errVerify( status, "initDeviceMem" );
}



void
OclAddReduce::initKernel()
{
    cl_int status;

    // build program with binary
    // please use program "m2c" and do not rename addReduce.cl
#ifndef NV
    buildWithBinary( mProgram, mContext, mDevice );
#else
    buildWithSource( mProgram, mContext, mDevice );
#endif

    // create kernel
    mKernel = clCreateKernel( mProgram, "reduction_v2", &status );
    errVerify( status, "initKernel_rv2" );
    mKernel2 = clCreateKernel( mProgram, "reduction_v2_finalize", &status );
    errVerify( status, "initKernel_rv2f" );

    // setting kernel arguments
    status = clSetKernelArg( mKernel, 0, sizeof(cl_mem), &mData );
    errVerify( status, "initKernel_arg0" );
}

void
OclAddReduce::runKernel(size_t num_src_items, size_t dsize, size_t step, int *data) {
    cl_int status;

    cl_uint ws = 32;
    global_work_size = mComputeUnits * 7 * ws; // 7 wavefronts per SIMD
//		while( (num_src_items / 4) % global_work_size != 0 )
//			global_work_size += ws;
    local_work_size = 128;
    num_groups = global_work_size / local_work_size;

    /*    using std::cout;
        using std::endl;
        cout<<"nsi="<<num_src_items<<" dsize="<<dsize
            <<"\ngws="<<global_work_size<<" lws="<<local_work_size<<" ng="<<num_groups<<endl;*/

    mGrpResult = clCreateBuffer( mContext, CL_MEM_READ_WRITE,
                                 num_groups * sizeof(int), NULL, NULL);

    status = clSetKernelArg( mKernel, 1, sizeof(cl_mem*), &mGrpResult );
    errVerify( status, "runKernel_arg1" );
    status = clSetKernelArg( mKernel, 2, sizeof(cl_int), NULL );
    errVerify( status, "runKernel_arg2" );
    status = clSetKernelArg( mKernel2, 0, sizeof(cl_mem*), &mGrpResult );
    errVerify( status, "runKernel_arg0" );

    int zero = 0;
    for(size_t i=0; i<num_src_items; i += step) {
        size_t size = MIN(step, num_src_items-i);
        if(size == 0) break;

//        std::cout<<"iter="<<i<<" size="<<size<<"\n";

        // size must >= 0 and multiple of 4 (since sum_src_itmes and step are)
        // there's no need to check for size>=4
        status = clEnqueueFillBuffer( mCommandQ, mData, &zero, sizeof(int),
                                      (size-4)*sizeof(int),
                                      4*sizeof(int), 0, NULL, NULL );

        status = clEnqueueWriteBuffer( mCommandQ, mData, CL_FALSE, 0,
                                       MIN(step, dsize-i)  * sizeof(int), data + i, 0, NULL, NULL );
        errVerify( status, "initDeviceMem" );

        runKernel_knl(size);

        getResult_gpu();

        //std::cout<<result<<"\n";
    }

}

void
OclAddReduce::runKernel_knl(size_t num_src_items)
{
    cl_int status;

    status = clSetKernelArg( mKernel, 3, sizeof(num_src_items), &num_src_items );
    errVerify( status, "runKernel_arg3" );

    status = clEnqueueNDRangeKernel( mCommandQ, mKernel, 1,
                                     0, &global_work_size, &local_work_size, 0, NULL, NULL );
    errVerify(status);

    status = clEnqueueNDRangeKernel( mCommandQ, mKernel2, 1,
                                     0, &num_groups, 0, 0, NULL, NULL );
    errVerify(status);
    clFinish( mCommandQ );
}

void
OclAddReduce::clear()
{
    return;
    /* Release the memory*/
    clReleaseKernel( mKernel );
    clReleaseProgram( mProgram );
    clReleaseCommandQueue( mCommandQ );
    clReleaseMemObject( mData );
    clReleaseMemObject( mGrpResult );
    clReleaseContext( mContext );
    if( mDevice != NULL ) delete [] mDevice;
    if( mPlatform != NULL ) delete [] mPlatform;
}

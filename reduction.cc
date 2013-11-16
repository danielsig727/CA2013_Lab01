#include <iostream>
#include <cassert>
#include <cstring>
#include "utils.hh"
#include "defines.hh"
#include "reduction.hh"


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
    run_cpu();
}

void
OclAddReduce::run_cpu()
{
    int rst = 0;
#pragma omp parallel for reduction(+:rst)
    for(int i=0; i<DATA_SIZE; i++){
        rst = rst +  mHostData[i];
    }
    result = rst;
}

void
OclAddReduce::run_gpu()
{
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

    /*Step 5: create device buffers*/
    initDeviceMem();

    /*Step 6: build program, and then create & set kernel*/
    initKernel();

    /*Step 7: run kernel*/
    runKernel();
}

void
OclAddReduce::getResult_gpu()
{
    clEnqueueReadBuffer( mCommandQ, mGrpResult, CL_TRUE,
                         0, sizeof(int), &result, 0, NULL, NULL );
    /*    int x=10;
        int *d = new int[x];
        clEnqueueReadBuffer( mCommandQ, mGrpResult, CL_TRUE,
                0, sizeof(int)*x, d, 0, NULL, NULL );
        for(int i=0; i<x; i++)
            std::cout<<d[i]<<' ';
        std::cout<<std::endl;
        result = d[0];
        delete [] d; */
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
OclAddReduce::initDeviceMem()
{
    num_src_items = DATA_SIZE + ((DATA_SIZE%4)?(4-DATA_SIZE%4):0);

    cl_int status;
    mData = clCreateBuffer( mContext, CL_MEM_READ_ONLY,
                            num_src_items * sizeof(int), NULL, &status );

    int zero = 0;
    status = clEnqueueFillBuffer( mCommandQ, mData, &zero, sizeof(int),
                                  (num_src_items-4)*sizeof(int),
                                  4*sizeof(int), 0, NULL, NULL );

    status = clEnqueueWriteBuffer( mCommandQ, mData, CL_FALSE, 0,
                                   DATA_SIZE  * sizeof(int), mHostData, 0, NULL, NULL );
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
OclAddReduce::runKernel()
{
    cl_int status;
    size_t global_work_size;
    size_t local_work_size;
    size_t num_groups;
    cl_uint ws = 64;
    global_work_size = mComputeUnits * 7 * ws; // 7 wavefronts per SIMD
//		while( (num_src_items / 4) % global_work_size != 0 )
//			global_work_size += ws;
    local_work_size = ws;
    num_groups = global_work_size / local_work_size;

//    using std::cout;
//    using std::endl;
//    cout<<"DATA_SIZE="<<DATA_SIZE<<" nsi="<<num_src_items
//        <<"\ngws="<<global_work_size<<" lws="<<local_work_size<<" ng="<<num_groups<<endl;

    mGrpResult = clCreateBuffer( mContext, CL_MEM_READ_WRITE,
                                 num_groups * sizeof(int), NULL, NULL);

    status = clSetKernelArg( mKernel, 1, sizeof(cl_mem*), &mGrpResult );
    errVerify( status, "runKernel_arg1" );
    status = clSetKernelArg( mKernel, 2, sizeof(cl_int), NULL );
    errVerify( status, "runKernel_arg2" );
    status = clSetKernelArg( mKernel, 3, sizeof(num_src_items), &num_src_items );
    errVerify( status, "runKernel_arg3" );
    status = clSetKernelArg( mKernel2, 0, sizeof(cl_mem*), &mGrpResult );
    errVerify( status, "runKernel_arg0" );


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

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

#ifndef SEPARATED_RUN
void
OclAddReduce::run()
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
#else
void
OclAddReduce::runPrepare()
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
}
void
OclAddReduce::run()
{
    /*Step 7: run kernel*/
    runKernel();
}

#endif

int
OclAddReduce::getResult()
{
    int result = 0;

//    clEnqueueReadBuffer( mCommandQ, mData, CL_TRUE,
//                         0, sizeof(int), &result, 0, NULL, NULL );
    clEnqueueReadBuffer( mCommandQ, mGrpResult, CL_TRUE,
                         0, sizeof(int), &result, 0, NULL, NULL );

    /*    int *d = new int[DATA_SIZE];
        clEnqueueReadBuffer( mCommandQ, mData, CL_TRUE,
                0, sizeof(int)*DATA_SIZE, d, 0, NULL, NULL );
        for(int i=0; i<DATA_SIZE; i++)
            std::cout<<d[i]<<' ';
        std::cout<<std::endl;
        result = d[0];
        delete [] d; */
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
    num_src_items = DATA_SIZE + (DATA_SIZE % 4);
    cl_int status;
    mData = clCreateBuffer( mContext, CL_MEM_READ_WRITE,
                            num_src_items * sizeof(int), NULL, &status );
    status = clEnqueueWriteBuffer( mCommandQ, mData, CL_FALSE, 0,
                                   DATA_SIZE * sizeof(int), mHostData, 0, NULL, NULL );
    errVerify( status, "initDeviceMem" );
    if( num_src_items != DATA_SIZE) {
        size_t delta = num_src_items - DATA_SIZE;
        int *zeros = new int[delta];
        memset( zeros, 0, delta * sizeof(int) );
        status = clEnqueueWriteBuffer( mCommandQ, mData, CL_FALSE, DATA_SIZE * sizeof(int),
                                       delta * sizeof(int), mHostData, 0, NULL, NULL );
        errVerify( status, "initDeviceMem" );
        delete [] zeros;
    }

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
    /*#ifndef MSCHED
        mKernel = clCreateKernel( mProgram, "reduction_worker", &status );
    #else
        mKernel = clCreateKernel( mProgram, "reduction_worker_scheduler", &status );
    #endif*/
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
    /*
    	// Smaller case use conventional kernel
    	status = clSetKernelArg( mKernel, 1, sizeof(size_t), &DATA_SIZE);
        errVerify(status);
        size_t wsize;
        size_t odd;
        size_t dsize;
        unsigned int level;
    #ifdef MSCHED
        size_t numKernel;
    #endif
        for(odd = DATA_SIZE & 1, dsize = DATA_SIZE>>1, level = 1;
                dsize + odd ;
                odd = dsize & 1, dsize = dsize>>1, ++level)
        {
       	    status = clSetKernelArg( mKernel, 2, sizeof(unsigned int), &level );
           	errVerify(status);
            wsize = dsize + odd;
    #ifndef MSCHED
            status = clEnqueueNDRangeKernel( mCommandQ, mKernel, 1,
       	                                     0, &wsize, 0, 0, NULL, NULL );
    #else
            numKernel = (wsize > GPU_KERNLIM) ? GPU_KERNLIM : wsize;
            status = clSetKernelArg( mKernel, 3, sizeof(size_t), &numKernel );
            status = clEnqueueNDRangeKernel( mCommandQ, mKernel, 1,
                                             0, &numKernel, 0, 0, NULL, NULL );
    #endif
            errVerify(status);
    */
    size_t global_work_size;
    size_t local_work_size;
    size_t num_groups;
    cl_uint ws = 64;
    global_work_size = mComputeUnits * 7 * ws; // 7 wavefronts per SIMD
//		while( (num_src_items / 4) % global_work_size != 0 )
//			global_work_size += ws;
    local_work_size = ws;
    num_groups = global_work_size / local_work_size;

    mGrpResult = clCreateBuffer( mContext, CL_MEM_READ_WRITE,
                                 num_groups * sizeof(int), NULL, NULL);

    status = clSetKernelArg( mKernel, 1, sizeof(cl_mem*), &mData );
    errVerify( status, "runKernel_arg1" );
    status = clSetKernelArg( mKernel, 2, sizeof(cl_mem*), &mGrpResult );
    errVerify( status, "runKernel_arg2" );
    status = clSetKernelArg( mKernel, 3, sizeof(cl_int), NULL );
    errVerify( status, "runKernel_arg3" );
    status = clSetKernelArg( mKernel, 4, sizeof(num_src_items), &num_src_items );
    errVerify( status, "runKernel_arg4" );
    status = clSetKernelArg( mKernel2, 0, sizeof(cl_mem*), &mGrpResult );
    errVerify( status, "runKernel_arg0" );


    status = clEnqueueNDRangeKernel( mCommandQ, mKernel, 1,
                                     0, &global_work_size, &local_work_size, 0, NULL, NULL );
    errVerify(status);

    status = clEnqueueNDRangeKernel( mCommandQ, mKernel, 1,
                                     0, &num_groups, 0, 0, NULL, NULL );
    errVerify(status);
    clFinish( mCommandQ );
}

void
OclAddReduce::clear()
{
    /* Release the memory*/
    clReleaseKernel( mKernel );
    clReleaseProgram( mProgram );
    clReleaseCommandQueue( mCommandQ );
    clReleaseMemObject( mData );
    clReleaseContext( mContext );
    if( mDevice != NULL ) delete [] mDevice;
    if( mPlatform != NULL ) delete [] mPlatform;
}

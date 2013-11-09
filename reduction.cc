#include <iostream>
#include <cassert>
#include "utils.hh"
#include "defines.hh"
#include "reduction.hh"


/*
 * The following function is only used to check whether certain command is correctly executed
 * For more information, please refers to
 *     http://www.khronos.org/registry/cl/sdk/1.2/docs/man/xhtml/
 */
void
errVerify( cl_int status, std::string msg = "" ){
    using std::cerr;
    using std::endl;
    if( status != CL_SUCCESS ){
        cerr<<"There is something error("<<status<<", "<<msg<<")"<<endl;
    }
}

void
OclAddReduce::run(){
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

int
OclAddReduce::getResult(){
    int result = 0;

    clEnqueueReadBuffer( mCommandQ, mData, CL_TRUE, 
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

// testing
    for(int i=0; i<DATA_SIZE; i++){
        result += *(mHostData + i);
    }

    return result;
}

void
OclAddReduce::initPlatform(){
    cl_uint numPlatforms = 0;

    clGetPlatformIDs( 0, NULL, &numPlatforms );
    mPlatform = new cl_platform_id[ numPlatforms ];
    clGetPlatformIDs( numPlatforms, mPlatform, NULL );
}

void
OclAddReduce::initDevice(){
    cl_uint numDevices = 0;

    clGetDeviceIDs( mPlatform[0], CL_DEVICE_TYPE_GPU, 0, NULL, &numDevices );
    mDevice = new cl_device_id[ numDevices ];
    clGetDeviceIDs( mPlatform[0], CL_DEVICE_TYPE_GPU, numDevices, mDevice, NULL );
}

void
OclAddReduce::showInfo(){
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

    cl_bool ava;
    clGetDeviceInfo( *mDevice, CL_DEVICE_AVAILABLE, sizeof( cl_bool), (void*) &ava, NULL);
    cout<<"Available: ";
    if( ava == CL_TRUE ){
        cout<<"YES"<<endl;
    } else {
        cout<<"NO"<<endl;
    }

    cout<<"************************************************************"<<endl;
    cout<<endl;
}

void
OclAddReduce::initContext(){
    cl_int status;
    mContext = clCreateContext( NULL, 1, mDevice, NULL, NULL, &status );
    errVerify( status, "initContext" );
}

void
OclAddReduce::initCommandQ(){
    mCommandQ = clCreateCommandQueue( mContext, mDevice[0], 0, NULL );
}

void
OclAddReduce::initDeviceMem(){
    cl_int status;
    mData = clCreateBuffer( mContext, CL_MEM_READ_WRITE, 
            DATA_SIZE * sizeof(int), NULL, &status );
    status = clEnqueueWriteBuffer( mCommandQ, mData, CL_FALSE, 0, 
            DATA_SIZE * sizeof(int), mHostData, 0, NULL, NULL );
    errVerify( status, "initDeviceMem" );
}

void
OclAddReduce::initKernel(){
    cl_int status;

    // build program with binary
    // please use program "m2c" and do not rename addReduce.cl
#ifndef NV
    buildWithBinary( mProgram, mContext, mDevice );
#else
    buildWithSource( mProgram, mContext, mDevice );
#endif

    // create kernel
    mKernel = clCreateKernel( mProgram, "reduction_worker", &status );
    // setting kernel arguments
    status = clSetKernelArg( mKernel, 0, sizeof(cl_mem), &mData );
    errVerify( status, "initKernel" );
}

void
OclAddReduce::runKernel(){
    cl_int status;
    cl_event event;
    status = clSetKernelArg( mKernel, 1, sizeof(size_t), &DATA_SIZE);
    errVerify(status);
    size_t wsize;
    size_t odd; size_t dsize; unsigned int level;
    for(odd = DATA_SIZE & 1, dsize = DATA_SIZE>>1, level = 1; 
            dsize + odd ; 
            odd = dsize & 1, dsize = dsize>>1, ++level){
        status = clSetKernelArg( mKernel, 2, sizeof(unsigned int), &level );
        errVerify(status);
        wsize = dsize + odd;
        status = clEnqueueNDRangeKernel( mCommandQ, mKernel, 1,
            0, &wsize, 0, 0, NULL, NULL );
        errVerify(status);
//        clWaitForEvents(1, &event);
//        std::cout<<"l("<<level<<","<<wsize<<")";
    }

}

void
OclAddReduce::clear(){
//return;
    /* Release the memory*/
    clReleaseKernel( mKernel );
    clReleaseProgram( mProgram );
    clReleaseCommandQueue( mCommandQ );
    clReleaseMemObject( mData );
    clReleaseContext( mContext );
    if( mDevice != NULL ) delete [] mDevice;
    if( mPlatform != NULL ) delete [] mPlatform;
}

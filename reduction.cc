#include <iostream>
#include "utils.hh"
#include "defines.hh"
#include "reduction.hh"


/*
 * The following function is only used to check whether certain command is correctly executed
 * For more information, please refers to
 *     http://www.khronos.org/registry/cl/sdk/1.2/docs/man/xhtml/
 */
void
errVerify( cl_int status ){
    using std::cerr;
    using std::endl;
    if( status != CL_SUCCESS ){
        cerr<<"There is something error"<<endl;
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
    errVerify( status );
}

void
OclAddReduce::initCommandQ(){
    mCommandQ = clCreateCommandQueue( mContext, mDevice[0], 0, NULL );
}

void
OclAddReduce::initDeviceMem(){
}

void
OclAddReduce::initKernel(){

    // build program with binary
    // please use program "m2c" and do not rename addReduce.cl
    buildWithBinary( mProgram, mContext, mDevice );

    // create kernel

    // setting kernel arguments
}

void
OclAddReduce::runKernel(){
}

void
OclAddReduce::clear(){

    /* Release the memory*/
    clReleaseKernel( mKernel );
    clReleaseProgram( mProgram );
    clReleaseCommandQueue( mCommandQ );
    clReleaseContext( mContext );
    if( mDevice != NULL ) delete [] mDevice;
    if( mPlatform != NULL ) delete [] mPlatform;
}

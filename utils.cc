#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include "defines.hh"
#include "utils.hh"

void
initGenerator( unsigned seed )
{
    srand( seed );
}

void
genData( int* &data )
{
    data = new int[ DATA_SIZE ];
    for( int i = 0; i != DATA_SIZE; ++i )
    {
        data[i] = (int) rand();
    }
}

void
deleteData( int* &data )
{
    delete [] data;
}

void
buildWithBinary( cl_program &mProgram, cl_context &mContext, const cl_device_id* const mDevice )
{

    using std::cout;
    using std::endl;

    int err_code;
    size_t file_size;
    unsigned char* bin_content;
    std::fstream file;

    file.open("addReduce.bin", std::ios::in );
    file.seekg(0, file.end);
    file_size = file.tellg();
    file.seekg(0, file.beg);

    bin_content = new unsigned char[ file_size ];
    file.read( (char*) bin_content, file_size );
    file.close();

    mProgram = clCreateProgramWithBinary( mContext, 1, mDevice, &file_size, (const unsigned char**) &bin_content, NULL, &err_code );
    if( err_code != CL_SUCCESS )
    {
        std::cout<<"Error building program"<<std::endl;
    }
    if( err_code == CL_INVALID_CONTEXT )
    {
        cout<<file_size<<endl;
    }

    delete [] bin_content;
    int i = clBuildProgram( mProgram, 1, &mDevice[0], NULL, NULL, NULL );
	assert(i == CL_SUCCESS);
}

void
buildWithSource( cl_program &mProgram, cl_context &mContext, const cl_device_id* const mDevice )
{

    using std::cout;
    using std::endl;

    int err_code;
    size_t file_size;
    unsigned char* src_content;
    std::fstream file;

    file.open("addReduce.cl", std::ios::in );
    file.seekg(0, file.end);
    file_size = file.tellg();
    file.seekg(0, file.beg);

    src_content = new unsigned char[ file_size ];
    file.read( (char*) src_content, file_size );
    file.close();

    mProgram = clCreateProgramWithSource( mContext, 1, (const char**) &src_content, &file_size, &err_code );
    if( err_code != CL_SUCCESS )
    {
        std::cout<<"Error building source"<<std::endl;
    }
    if( err_code == CL_INVALID_CONTEXT )
    {
        cout<<file_size<<endl;
    }

    delete [] src_content;
    int i = clBuildProgram( mProgram, 1, &mDevice[0], NULL, NULL, NULL );
    assert(i == CL_SUCCESS);
}

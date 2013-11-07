#include <iostream>
#include <sys/time.h>
#include <cstdlib>
#include "reduction.hh"
#include "defines.hh"
#include "utils.hh"

void printElapsedTime( struct timeval &start, struct timeval &end ){
    using namespace std;
    unsigned long long start_time = start.tv_sec * 1000000 + start.tv_usec;
    unsigned long long end_time = end.tv_sec * 1000000 + end.tv_usec;
    cout<<endl
        <<"Kernel Time: "<<end_time-start_time<<" us"<<endl;
}

size_t DATA_SIZE;

int main(int argc, char *argv[])
{
    using namespace std;
    int* random_data;

    if( argc != 3 ){
        cerr<<"Usage: \n"
            <<"./reduction [seed] [data_number]"
            <<endl;
        return 0;
    }

    // Initialize with the values
    unsigned seed = atoi( argv[1] );
    if( seed != 0 ){
        initGenerator( seed );
    } else {
        cerr<<"Warning: fallback to default seed"<<endl;
        initGenerator();
    }
    unsigned size = atoi( argv[2] );
    if( size != 0 ){
        DATA_SIZE = size;
    } else {
        cerr<<"Error: no data"<<endl;
        return 0;
    }

    // Initializae data
    genData( random_data );

    struct timeval start, end;
    gettimeofday(&start, NULL);
    OclAddReduce reduce( random_data );
    reduce.run();
    int device_result = reduce.getResult();
    gettimeofday(&end, NULL);

    int golden_result = 0;
    for (int i = 0; i < DATA_SIZE; ++i)
    {
        golden_result += random_data[i];
    }

    if( golden_result != device_result ){
        cout<<"Result mismatch !!!"<<endl;
        cout<<"Golden: "<<golden_result<<endl;
        cout<<"Yours: "<<device_result<<endl;
    } else {
        cout<<"Test pass, congraz !!!"<<endl;
    }

    printElapsedTime( start, end );

    deleteData( random_data );

    return 0;
}

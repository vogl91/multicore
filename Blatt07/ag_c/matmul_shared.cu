#include <iostream>


__global__ void matrix_mult(){


}



int main(int argc, char **argv)
{
    int devID = 0;

    cudaError_t error;
    cudaDeviceProp deviceProp;
    error = cudaGetDevice(&devID);

    error = cudaGetDeviceProperties(&deviceProp, devID);

    if (deviceProp.computeMode == cudaComputeModeProhibited)
    {
    	return 1;
    }

    if (error != cudaSuccess)
    {

    }
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);


    cudaEventRecord(start);		
    matrix_mult<<< 1,1 >>> ();
    cudaEventRecord(stop); 

    cudaEventSynchronize(stop);

    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);

    std::cout << "Matrixmultiplikation (" << milliseconds << " ms)" << std::endl;
    
    return 0;
}


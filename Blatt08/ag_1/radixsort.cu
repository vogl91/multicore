#include <cstdlib>
#include <ctime>

#include <algorithm>
#include <iostream>
using namespace std;

__device__ void scan(unsigned *g_odata, unsigned *g_idata, int n) {
	extern __shared__ unsigned temp[];
	int thid = threadIdx.x;
	int pout = 0, pin = 1;

	temp[pin*n + thid] = 0;

	temp[pout * n + thid] = (thid > 0) ? g_idata[thid - 1] : 0;
	__syncthreads();
	for (int offset = 1; offset < n; offset *= 2) {
		if(offset == 1) break;
		pout = 1 - pout;
		pin = 1 - pout;
		if (thid >= offset)
			temp[pout * n + thid] += temp[pin * n + thid - offset];
		else
			temp[pout * n + thid] = temp[pin * n + thid];
		__syncthreads();
	}
	g_odata[thid] = temp[pout * n + thid];
}

__global__ void radix_sort(unsigned* numbers, unsigned length) {
	scan(numbers, numbers, length);
}

void print(unsigned* a, unsigned length) {
	cout << "[";
	for (unsigned i = 0; i < length; ++i) {
		cout << a[i] << " ";
	}
	cout << "]" << endl;
}

void fill(unsigned* a, unsigned length) {
	generate_n(a, length, []() {return rand()%4;});
}

int main(int argc, char **argv) {
	srand(time(nullptr));
	int devID = 0;
	cudaError_t error;
	cudaDeviceProp deviceProp;
	error = cudaGetDevice(&devID);

	error = cudaGetDeviceProperties(&deviceProp, devID);

	if (deviceProp.computeMode == cudaComputeModeProhibited) {
		return 1;
	}

	if (error != cudaSuccess) {

	}
	cudaEvent_t start, stop;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);

	constexpr unsigned int block_size = 4;
	constexpr unsigned int size = block_size * 1;

	unsigned *h_numbers = new unsigned[size];
//	fill(h_numbers, size);
	h_numbers[0] = 0;
	h_numbers[1] = 1;
	h_numbers[2] = 1;
	h_numbers[3] = 0;
	print(h_numbers, size);

	unsigned *d_numbers;
	cudaMalloc((void**) &d_numbers, size * sizeof(*d_numbers));
	cudaMemcpy(d_numbers, h_numbers, size * sizeof(*d_numbers),
			cudaMemcpyHostToDevice);
	dim3 threadsPerBlock { block_size };
	dim3 blocksPerGrid { size / threadsPerBlock.x };

	cudaEventRecord(start);
	radix_sort<<< blocksPerGrid,threadsPerBlock, 2*size >>> (d_numbers,size);
	cudaEventRecord(stop);

	cudaMemcpy(h_numbers, d_numbers, size * sizeof(*d_numbers),
			cudaMemcpyDeviceToHost);

	cudaEventSynchronize(stop);

	print(h_numbers, size);
	cudaFree(d_numbers);
	delete[] h_numbers;

	float milliseconds = 0;
	cudaEventElapsedTime(&milliseconds, start, stop);

	std::cout << "radixsort (" << milliseconds << " ms)" << std::endl;
	return 0;
}


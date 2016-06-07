#include <algorithm>
#include <random>
#include <iostream>
#include <iomanip>
#include <functional>

constexpr int BLOCK_SIZE = 32;

__global__ void matrix_mult(float* C, float* A, float* B, int size) {
	int bx = blockIdx.x;
	int by = blockIdx.y;

	int tx = threadIdx.x;
	int ty = threadIdx.y;

	int i = by * blockDim.y + ty;
	int j = bx * blockDim.x + tx;

	float sum = 0.0;
#pragma unroll
	for (int k = 0; k < size; k++) {
		sum += A[i * size + k] * B[k * size + j];
	}

	C[i * size + j] = sum;
}

struct Matrix {
	Matrix(int size) :
			size { size }, data { new float[size * size] } {
	}
	~Matrix() {
		delete[] data;
	}
	const int size;
	float* data;
	float& get(int i, int j) {
		return data[i * size + j];
	}
};

void fill(Matrix& a) {
	const auto min = 0.0f;
	const auto max = 1.0f;
	std::random_device real_random;
	std::mt19937 random { real_random() };
	std::uniform_real_distribution<float> distribution { min, max };
	auto next_val = std::bind(distribution, random);
	std::generate(a.data, a.data + a.size * a.size, next_val);
}

bool is_equal(Matrix& a, Matrix& b) {
	const auto epsilon = 0.0001;
	if (a.size != b.size)
		return false;
	for (auto i = 0; i < a.size * a.size; ++i) {
		if (abs(a.data[i] - b.data[i]) > epsilon)
			return false;
	}
	return true;
}

bool is_correct(Matrix& c, Matrix& a, Matrix& b) {
	const auto epsilon = 0.0001;
	for (int i = 0; i < c.size; ++i) {
		for (int j = 0; j < c.size; ++j) {
			float sum = 0.0f;
			for (int k = 0; k < c.size; ++k) {
				sum += a.get(i, k) * b.get(k, j);
			}
			if (abs(sum - c.get(i, j) > epsilon))
				return false;
		}
	}
	return true;
}

int main(int argc, char **argv) {
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

	{
		constexpr auto matrix_size = 1024;
//		constexpr auto matrix_size = 2048;
//		constexpr auto matrix_size = 4096;
//		constexpr auto matrix_size = 8192;
		Matrix h_A { matrix_size }, h_B { matrix_size }, h_C { matrix_size };
		fill(h_A);
		fill(h_B);

		float* d_A, *d_B, *d_C;

		cudaEventRecord(start);
		const int size_in_bytes = matrix_size * matrix_size * sizeof(float);
		//make space for device matrix representation
		cudaMalloc((void**) &d_A, size_in_bytes);
		cudaMalloc((void**) &d_B, size_in_bytes);
		cudaMalloc((void**) &d_C, size_in_bytes);

		//copy input matrix to device
		cudaMemcpy(d_A, h_A.data, size_in_bytes, cudaMemcpyHostToDevice);
		cudaMemcpy(d_B, h_B.data, size_in_bytes, cudaMemcpyHostToDevice);

		dim3 threadsPerBlock(BLOCK_SIZE, BLOCK_SIZE, 1);
		dim3 blocksPerGrid(matrix_size / BLOCK_SIZE, matrix_size / BLOCK_SIZE,
				1);
		matrix_mult<<<blocksPerGrid, threadsPerBlock>>>(d_C, d_A, d_B,
				matrix_size);

		//copy output matrix to host
		cudaMemcpy(h_C.data, d_C, size_in_bytes, cudaMemcpyDeviceToHost);
		cudaFree(d_A);
		cudaFree(d_B);
		cudaFree(d_C);
		cudaEventRecord(stop);

		cudaEventSynchronize(stop);

		float milliseconds = 0;
		cudaEventElapsedTime(&milliseconds, start, stop);

		std::cout << "Matrixmultiplikation (" << milliseconds << " ms)"
				<< std::endl;

		std::cout << "is_correct:" << std::boolalpha
				<< is_correct(h_C, h_A, h_B) << std::endl;
	}

	return 0;
}


#include <cstdlib>
#include <ctime>
#include <climits>

#include <algorithm>
#include <functional>
#include <iostream>
using namespace std;

/*========* CudaArray *========*/

template<typename T>
class CudaArray {
public:
	CudaArray(int size) :
			size_ { size } {
		host_data = (T*) malloc(sizeof(T) * size);
		cudaMalloc(&device_data, sizeof(T) * size);
	}
	~CudaArray() {
		free(host_data);
		cudaFree(device_data);
	}
	int size() const {
		return size_;
	}
	T* host() {
		return host_data;
	}
	const T* host() const {
		return host_data;
	}
	T*& device() {
		return device_data;
	}
	const T* device() const {
		return device_data;
	}

	void copyToDevice() {
		cudaMemcpy(device_data, host_data, sizeof(T) * size_,
				cudaMemcpyHostToDevice);
	}
	void copyFromDevice() {
		cudaMemcpy(host_data, device_data, sizeof(T) * size_,
				cudaMemcpyDeviceToHost);
	}

private:
	int size_;
	T* host_data;
	T* device_data;
};

template<typename T>
void print(CudaArray<T>& a) {
	cout << "[ ";
	for (int i = 0; i < a.size(); ++i) {
		cout << a.host()[i] << " ";
	}
	cout << "]" << endl;
}

/*========* ArrayUtils *========*/

template<typename T, typename Fun>
void for_each(T *a, int length, Fun f) {
	for (int i = 0; i < length; ++i) {
		f(a[i], i);
	}
}

template<typename T>
void print(T *a, int length) {
	for_each(a, length, [](T& x, int i) {
		cout << x << " ";
	});
	cout << endl;
}

template<typename T, size_t N>
void print(T (&a)[N]) {
	print(a, N);
}

/*========* compile-time utils *========*/

constexpr unsigned ilog2_impl(unsigned x, unsigned bit) {
	return (1 << bit) & x ? bit : ilog2_impl(x, bit - 1);
}

constexpr unsigned ilog2(unsigned x) {
	return ilog2_impl(x, sizeof(unsigned) * CHAR_BIT - 1u);
}

constexpr unsigned next_multiple(unsigned multiple, unsigned x) {
	return x + (x % multiple == 0 ? 0 : (multiple - x % multiple));
}

/*========* device code *========*/

//__host__ __device__ unsigned id(unsigned x) {
//	return x;
//}
//
//__host__ __device__ unsigned is_zero(unsigned bit, unsigned x) {
//	return ((1 << bit) & ~x) >> bit;
//}
//
//template<typename Fun>
//__device__ void transform_scan(unsigned *idata, unsigned *odata, int n,
//		Fun fun) {
//	__shared__ unsigned temp[THREADS_PER_BLOCK * sizeof(unsigned)];
//	int thid = threadIdx.x;
//	int offset = 1;
//	temp[2 * thid] = fun(idata[2 * thid]);
//	temp[2 * thid + 1] = fun(idata[2 * thid + 1]);
//	for (int d = n >> 1; d > 0; d >>= 1) {
//		__syncthreads();
//		if (thid < d) {
//			int ai = offset * (2 * thid + 1) - 1;
//			int bi = offset * (2 * thid + 2) - 1;
//			temp[bi] += temp[ai];
//		}
//		offset *= 2;
//	}
//	if (thid == 0) {
//		temp[n - 1] = 0;
//	}
//	for (int d = 1; d < n; d *= 2) {
//		offset >>= 1;
//		__syncthreads();
//		if (thid < d) {
//			int ai = offset * (2 * thid + 1) - 1;
//			int bi = offset * (2 * thid + 2) - 1;
//			unsigned t = temp[ai];
//			temp[ai] = temp[bi];
//			temp[bi] += t;
//		}
//	}
//	__syncthreads();
//	odata[2 * thid] = temp[2 * thid];
//	odata[2 * thid + 1] = temp[2 * thid + 1];
//}
//
//__global__ void transform_scan_all(unsigned *idata, unsigned *odata, int n,
//		unsigned bit) {
//	int offset = 2 * blockDim.x * blockIdx.x;
//	transform_scan(idata + offset, odata + offset, 2 * blockDim.x,
//			[bit](unsigned x) {
//				return is_zero(bit,x);
//			});
//}
//__global__ void scan_all(unsigned *idata, unsigned *odata, int n) {
//	int offset = 2 * blockDim.x * blockIdx.x;
//	transform_scan(idata + offset, odata + offset, 2 * blockDim.x, id);
//}
//
//__device__ unsigned *scan_buffer;
//__global__ void radix_sort(unsigned *numbers, int n, unsigned bit) {
//	int idx = threadIdx.x + blockDim.x * blockIdx.x;
//	if (idx == 0) {
//		scan_buffer = new unsigned[n];
//	}
//	int offset = 2 * blockDim.x * blockIdx.x;
//	__syncthreads();
//	transform_scan(numbers + offset, scan_buffer + offset, 2 * blockDim.x,
//			[bit](unsigned x) {
//				return is_zero(bit, x);
//			});
//	__syncthreads();
//	const unsigned zeros = scan_buffer[n - 1] + is_zero(numbers[n - 1], bit);
//	const unsigned tmp1 = numbers[2 * idx];
//	const unsigned tmp2 = numbers[2 * idx + 1];
//
//	__syncthreads();
//	const auto new_index = [zeros,bit](unsigned x, unsigned i) {
//		return is_zero(x,bit)
//		? scan_buffer[i]
//		: i - scan_buffer[i] + zeros;
//	};
//	__syncthreads();
//	numbers[new_index(tmp1, 2 * idx)] = tmp1;
//	numbers[new_index(tmp2, 2 * idx + 1)] = tmp2;
//
//	__syncthreads();
//	if (idx == 0) {
//		delete[] scan_buffer;
//	}
//}
__host__ __device__ unsigned is_zero(unsigned bit, unsigned x) {
	return ((1 << bit) & ~x) >> bit;
}
__device__ void block_scan(unsigned *data, int n, unsigned *sum) {
	extern __shared__ unsigned temp[];
	int thid = threadIdx.x;
	int offset = 1;
	temp[2 * thid] = data[2 * thid];
	temp[2 * thid + 1] = data[2 * thid + 1];
	for (int d = n >> 1; d > 0; d >>= 1) {
		__syncthreads();
		if (thid < d) {
			int ai = offset * (2 * thid + 1) - 1;
			int bi = offset * (2 * thid + 2) - 1;
			temp[bi] += temp[ai];
		}
		offset *= 2;
	}
	if (thid == 0) {
		*sum = temp[n - 1];
		temp[n - 1] = 0;
	}
	for (int d = 1; d < n; d *= 2) {
		offset >>= 1;
		__syncthreads();
		if (thid < d) {
			int ai = offset * (2 * thid + 1) - 1;
			int bi = offset * (2 * thid + 2) - 1;
			unsigned t = temp[ai];
			temp[ai] = temp[bi];
			temp[bi] += t;
		}
	}
	__syncthreads();
	data[2 * thid] = temp[2 * thid];
	data[2 * thid + 1] = temp[2 * thid + 1];
}
__global__ void partial_scan(unsigned *data, int n, unsigned *sums) {
	int offset = 2 * blockDim.x * blockIdx.x;
	block_scan(data + offset, 2 * blockDim.x, sums + blockIdx.x);
}

__global__ void d_fill_n(unsigned *data, unsigned n, unsigned value) {
	int idx = threadIdx.x + blockDim.x * blockIdx.x;
	if (idx < n) {
		data[idx] = value;
	}
}

__global__ void add_sums(unsigned *numbers, unsigned *block_sums) {
	int idx = threadIdx.x + blockDim.x * blockIdx.x;
	numbers[2 * idx] += block_sums[blockIdx.x];
	numbers[2 * idx + 1] += block_sums[blockIdx.x];
}

__global__ void d_transform(unsigned *numbers, unsigned *new_numbers,
		unsigned bit) {
	int idx = threadIdx.x + blockDim.x * blockIdx.x;
	new_numbers[idx] = is_zero(bit, numbers[idx]);
}
__global__ void rearange(unsigned *in_numbers, unsigned *out_numbers,
		unsigned *scan, unsigned count, unsigned bit) {
//	int idx = threadIdx.x + blockDim.x * blockIdx.x;
	auto zeros = scan[count - 1] + is_zero(bit, in_numbers[count - 1]);
	int blockOffset = (2 * blockDim.x) * blockIdx.x;

	int myindex1 = blockOffset + 2 * threadIdx.x;
	int myindex2 = blockOffset + 2 * threadIdx.x + 1;

	int val1 = in_numbers[myindex1];
	int val2 = in_numbers[myindex2];

	int index1 =
			is_zero(bit, val1) ?
					scan[myindex1] : myindex1 - scan[myindex1] + zeros;
	int index2 =
			is_zero(bit, val2) ?
					scan[myindex2] : myindex2 - scan[myindex2] + zeros;

	out_numbers[index1] = val1;
	out_numbers[index2] = val2;
}

/*========* host code *========*/

void scan(unsigned *d_numbers, const unsigned count,
		const unsigned block_size) {
	const unsigned count_filled = next_multiple(block_size, count);

	const unsigned block_dim = block_size / 2;
	const unsigned grid_dim = count_filled / block_dim;

	unsigned *d_block_sums;
	cudaMalloc(&d_block_sums, (grid_dim * 2) * sizeof(unsigned));

	d_fill_n<<<grid_dim, block_dim>>>(d_numbers + count, count_filled - count, 0);

	partial_scan<<<grid_dim, block_dim, 2*block_size*sizeof(unsigned)>>>(d_numbers, count_filled, d_block_sums);

	if (count_filled > block_size) {
		scan(d_block_sums, grid_dim, block_size);
		add_sums<<<grid_dim, block_dim>>>(d_numbers, d_block_sums);
	}
	cudaFree(d_block_sums);
}

void test_error(int str) {
	auto error = cudaGetLastError();
	if (error != cudaSuccess) {
		cout << str << ":" << cudaGetErrorString(error) << endl;
	}
}

void foo() {
	srand(time(nullptr));
	constexpr auto BLOCK_SIZE = 4;
	constexpr auto N = 128*1024;


	static_assert((1u<<ilog2(BLOCK_SIZE)) == BLOCK_SIZE,"THREADS_PER_BLOCK muss eine 2er Potenz sein");
	constexpr auto N_FILLED = next_multiple(BLOCK_SIZE, N);
	const unsigned block_dim = BLOCK_SIZE / 2;
	const unsigned grid_dim = N_FILLED / BLOCK_SIZE;

	cout << "foooo:" <<(N == N_FILLED) << endl;

	CudaArray<unsigned> a { N_FILLED };
	std::generate_n(a.host(), a.size(), []() {return rand()%16;});
//	int x = N_FILLED;
//	std::generate_n(a.host(), a.size(), [x]() mutable {return x--;});
//	std::fill_n(a.host(), a.size(),0);
//	print(a);

	a.copyToDevice();

	CudaArray<unsigned> d_buffer { N_FILLED };
	CudaArray<unsigned> d_scan { N_FILLED };
//	unsigned *d_scan;
//	unsigned *d_buffer;
//	cudaMalloc(&d_scan, N_FILLED * sizeof(unsigned));
//	cudaMalloc(&d_buffer, N_FILLED * sizeof(unsigned));
	for (int bit = 0; bit < 32; ++bit) {
		d_transform<<<grid_dim,BLOCK_SIZE>>>(a.device(), d_scan.device(), bit);
		test_error(__LINE__);
		scan(d_scan.device(), N, BLOCK_SIZE);
		test_error(__LINE__);
		rearange<<<grid_dim,block_dim>>>(a.device(), d_buffer.device(), d_scan.device(), N_FILLED, bit);
		test_error(__LINE__);
		d_scan.copyFromDevice();
		print(d_scan);
//		d_buffer.copyFromDevice();
//		print(d_buffer);
//		break;
		unsigned *tmp = a.device();
		a.device() = d_buffer.device();
		d_buffer.device() = tmp;
	}
//	cudaFree(d_scan);
//	cudaFree(d_buffer);
	a.copyFromDevice();
//	print(a);
	cout << is_sorted(a.host(), a.host() + a.size()) << endl;
}

void test_partial_scan() {
	constexpr auto BLOCK_SIZE = 4;
	constexpr auto N = 16;

	static_assert((1u<<ilog2(BLOCK_SIZE)) == BLOCK_SIZE,"THREADS_PER_BLOCK muss eine 2er Potenz sein");
	constexpr auto N_FILLED = next_multiple(BLOCK_SIZE, N);

	CudaArray<unsigned> a { N_FILLED };
	CudaArray<unsigned> sums { ilog2(N_FILLED) };
	std::generate_n(a.host(), a.size(), []() {return rand()%4;});
	print(a);
	a.copyToDevice();
	partial_scan<<<N/BLOCK_SIZE,BLOCK_SIZE/2,BLOCK_SIZE>>>(a.device(), a.size(), sums.device());
	a.copyFromDevice();
	sums.copyFromDevice();

	print(a);
	print(sums);

	auto error = cudaGetLastError();
	if (error != cudaSuccess) {
		cout << cudaGetErrorString(error) << endl;
	}
}

//void scan(unsigned *block_max_elem, unsigned n) {
//	if (n <= THREADS_PER_BLOCK)
//		return;
//	const auto N = next_multiple(THREADS_PER_BLOCK, n / THREADS_PER_BLOCK);
//	CudaArray<unsigned> max_elems { N };
//
//	for (int i = 0; i < N; ++i) {
//		max_elems.host()[i] = block_max_elem[(i + 1) * THREADS_PER_BLOCK - 1]
//				+ block_max;
//	}
//
//	max_elems.copyToDevice();
//	scan_all<<<N/THREADS_PER_BLOCK,THREADS_PER_BLOCK/2>>>(max_elems.device(),max_elems.device(),max_elems.size());
//	max_elems.copyFromDevice();
//
//	scan(max_elems.host(), N);
//
//	for (int i = 0; i < N; ++i) {
//		for (int j = 0; j < THREADS_PER_BLOCK; ++j) {
//			block_max_elem[i * THREADS_PER_BLOCK + j] += max_elems.host()[i];
//		}
//	}
//}

//void foo() {
//	srand(time(nullptr));
//	CudaArray<unsigned> a { N_FILLED };
//	CudaArray<unsigned> block_scan { N_FILLED };
//	CudaArray<unsigned> block_max_elem { N_FILLED / THREADS_PER_BLOCK };
//
//	std::fill_n(a.host(), N, 0);
////	std::generate_n(a.host(), a.size(), []() {return rand()%256;});
//	unsigned x = 1;
////	std::generate_n(a.host(), N, [x]() mutable {return x++;});
//	std::fill_n(a.host() + N, N_FILLED - N, UINT_MAX);
////	a.host()[0] = 3; //0
////	a.host()[1] = 0; //1
////	a.host()[2] = 2; //1
////	a.host()[3] = 1; //0
////	a.host()[4] = 3; //0
////	a.host()[5] = 0; //1
////	a.host()[6] = 2; //1
////	a.host()[7] = 1; //0
//
//	print(a.host(), N);
//
//	a.copyToDevice();
//	for (unsigned bit = 0u; bit < 32; ++bit) {
//		transform_scan_all<<<N_FILLED/THREADS_PER_BLOCK,THREADS_PER_BLOCK/2>>>(a.device(),block_scan.device(),N_FILLED,bit);
//		block_scan.copyFromDevice();
//		scan(block_scan.host(),block_scan.size());
//		break;
//	}
//	a.copyFromDevice();
//
//	bool ok = true;
//	x = 1;
//	for (int i = 0; i < N; ++i) {
//		if (a.host()[i] != x++) {
//			ok = false;
//			cout << i << endl;
//			break;
//		}
//	}
//	cout << "ok: " << ok << endl;
//	auto error = cudaGetLastError();
//	if (error != cudaSuccess) {
//		cout << cudaGetErrorString(error) << endl;
//	}
//	print(block_scan.host(), N);
//}

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
	cudaEventRecord(start);

	foo();

	cudaEventRecord(stop);

	cudaEventSynchronize(stop);

	float milliseconds = 0;
	cudaEventElapsedTime(&milliseconds, start, stop);

	cout << "time (" << milliseconds << " ms)" << endl;
	return 0;
}


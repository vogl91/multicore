#include <mpi.h>

#include <cassert>

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "common.h"
#include "matrix.h"

using namespace std;

template <typename T>
void fill_random(Matrix<T>& n, T min, T max) {
  default_random_engine generator;
  uniform_int_distribution<T> distribution{min, max};
  auto next_rand = bind(distribution, generator);

  generate(begin(n), end(n), next_rand);
}

void test() {
  /*
* [1 2 3    [7  8     [ 58  64
*  4 5 6] *  9 10  =   139 154]
*           11 12]
*/

  value_t valsA[2][3] = {{1, 2, 3}, {4, 5, 6}};
  value_t valsB[3][2] = {{7, 8}, {9, 10}, {11, 12}};
  Matrix<value_t> A{3, 2, valsA};
  Matrix<value_t> B{2, 3, valsB};
  Matrix<value_t> C{2, 2};
  cout << A << endl;
  cout << B << endl;
  mult(C, A, B);
  cout << C << endl;
}

void sendMatrix(Matrix<value_t>& A) {
  // DBG(__PRETTY_FUNCTION__);
  int width = A.width();
  int height = A.height();
  MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(A.data(), A.width() * A.height(), MPI_INT, 0, MPI_COMM_WORLD);
}

void partialReceiveMatrix(Matrix<value_t>& A, int startIndex, int endIndex, int sendRank) {
  DBG(__FUNCTION__ << "(" << startIndex << ", " << endIndex << ")");
  MPI_Recv(A.data() + startIndex, endIndex - startIndex, MPI_INT, sendRank, 0,
           MPI_COMM_WORLD, NULL);
}

// TODO correct size_t for send
template <typename T>
Matrix<T> distributed_mult(Matrix<T>& A, Matrix<T>& B) {
  DBG(__PRETTY_FUNCTION__);
  assert(A.width() == B.height());
  const auto N = A.height();
  // const auto M = A.width();
  const auto P = B.width();
  const auto elems = N * P;

  Matrix<T> BTransposed = B.transposed();
  Matrix<T> C{N, P};

  int numtasks;
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

  const int chunk_size = ceil((double)elems / (numtasks - 1));

      sendMatrix(A);
      sendMatrix(BTransposed);
  {
    int startIndex = 0;
    for (int i = 1; i < numtasks; ++i) {
      int indizes[2];
      indizes[0] = startIndex;
      indizes[1] = startIndex + chunk_size;
      MPI_Send(&indizes, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
      startIndex += chunk_size;
    }
  }
  {
    int startIndex = 0;
    for (int i = 1; i < numtasks; ++i) {
      partialReceiveMatrix(C, startIndex, startIndex + chunk_size, i);
      startIndex += chunk_size;
    }
  }

  return C;
}

int main(int argc, char* argv[]) {
  int rank, size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Matrix<value_t> A{3, 3, {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}}};
  // Matrix<value_t> B{3, 2, {{1, 3}, {2, 2}, {3, 1}}};

  size_t width  = 1000;
  size_t height = 1000;
  if(argc == 3) {
  	height = atoi(argv[1]);
  	width = atoi(argv[2]);
  }

  Matrix<value_t> A{height, width};
  Matrix<value_t> B{height, width};
  fill_random(A,1,1000);
  fill_random(B,1,1000);

  // cout << A << endl;
  // cout << B << endl;

  Matrix<value_t> C = distributed_mult(A, B);
  // mult(C,A,B);

  // cout << C;

  MPI_Finalize();
}

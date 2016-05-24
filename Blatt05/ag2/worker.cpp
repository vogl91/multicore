#include <mpi.h>

#include <algorithm>
#include <iostream>

#include "common.h"
#include "matrix.h"

using namespace std;

Matrix<value_t> receiveMatrix() {
  // DBG(__PRETTY_FUNCTION__);
  int width, height;
  MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
  Matrix<value_t> result{(size_t)height, (size_t)width};
  MPI_Bcast(result.data(), height * width, MPI_INT, 0, MPI_COMM_WORLD);
  return result;
}

void partialSendMatrix(Matrix<value_t>& A, int startIndex, int endIndex) {
  DBG(__FUNCTION__ << "(" << startIndex << ", " << endIndex << ")");
  MPI_Send(A.data() + startIndex, endIndex - startIndex, MPI_INT, 0, 0,
           MPI_COMM_WORLD);
}

int main(int argc, char* argv[]) {
  int rank;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  DBG(rank << " started...");

  // get calculation matrizes
  auto A = receiveMatrix();
  auto BTransposed = receiveMatrix();

  // create result matrix
  Matrix<decltype(A)::value_t> C{A.height(), BTransposed.height()};

  // get start and end index
  int indizes[2];
  MPI_Recv(&indizes, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, NULL);
  indizes[1] = std::min(indizes[1],(int)(C.width()*C.height()));

  assert(indizes[0] <= indizes[1]);

  DBG(rank << " start: " << indizes[0]);
  DBG(rank << " end: " << indizes[1]);

  partial_mult(C, A, BTransposed, indizes[0], indizes[1]);

  partialSendMatrix(C, indizes[0], indizes[1]);

  MPI_Finalize();
}

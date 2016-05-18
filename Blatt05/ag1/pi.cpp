#include <mpi.h>

#include <boost/timer/timer.hpp>
#include <cmath>

#include <algorithm>
#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>
using namespace std;

#define DBG(str) (std::cout << (str) << std::endl)

constexpr int MASTER_ID = 0;
constexpr double PI = 3.14159265359;

double monte_carlo(long samples) {
  long inside = 0;
  unsigned seed = chrono::system_clock::now().time_since_epoch().count();
  mt19937 generator{seed};
  uniform_real_distribution<double> distribution(0.0, 1.0);
  auto next_double = bind(distribution, generator);

  for (long i = 0; i < samples; ++i) {
    double x = next_double();
    double y = next_double();
    if (x * x + y * y <= 1.0) ++inside;
  }
  return (4.0 * (double)inside) / (double)samples;
}

int main(int argc, char *argv[]) {
  // Siehe http://www.boost.org/doc/libs/1_61_0/libs/timer/doc/cpu_timers.html
  boost::timer::auto_cpu_timer t;

  int samples = 10000;
  if (argc == 2) samples = atoi(argv[1]);

  int taskid;
  int numtasks;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
  MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

  cout << "MPI task " << taskid << " started..." << endl;

  double my_pi = monte_carlo(samples);

  double pi;
  MPI_Reduce(&my_pi, &pi, 1, MPI_DOUBLE, MPI_SUM, MASTER_ID, MPI_COMM_WORLD);
  if (taskid == MASTER_ID) {
    pi = pi / numtasks;
    cout << setprecision(8) << pi << endl;
    cout << "delta to PI: " << abs(PI - pi) << endl;
  }

  // if (taskid == MASTER_ID) {
  // vector<double> pi_values;
  // pi_values.resize(numtasks, 0);
  // pi_values[0] = my_pi;
  // for (int i = 1; i < numtasks; ++i) {
  //   MPI_Recv(&pi_values[i], 1, MPI_DOUBLE, i, 0, MPI_COMM_WORLD,
  //            MPI_STATUS_IGNORE);
  // }
  // for (int i = 0; i < pi_values.size(); ++i) {
  //   cout << i << ": " << setprecision(8) << pi_values[i] << endl;
  // }
  // double pi = accumulate(begin(pi_values), end(pi_values), 0.0) / numtasks;
  //   cout << "-> " << setprecision(8) << pi << endl;
  //   cout << "delta to PI: " << abs(PI - pi) << endl;
  // } else {
  //   MPI_Send(&my_pi, 1, MPI_DOUBLE, MASTER_ID, 0, MPI_COMM_WORLD);
  // }

  MPI_Finalize();
}

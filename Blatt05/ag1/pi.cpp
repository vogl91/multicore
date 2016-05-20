#include <mpi.h>

#include <boost/timer/timer.hpp>

#include <cassert>
#include <cmath>

#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <random>
#include <vector>
using namespace std;

#define ASSERT_10_PROCESSES 1

#define DBG(str) (std::cout << (str) << std::endl)

constexpr int MASTER_RANK = 0;
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

  MPI::Init(argc, argv);
  const int numtasks = MPI::COMM_WORLD.Get_size();
  const int world_rank = MPI::COMM_WORLD.Get_rank();

  if (numtasks < 2) {
    cerr << "must specify at least 2 tasks" << endl;
    MPI::COMM_WORLD.Abort(1);
    return 1;
  }
#if ASSERT_10_PROCESSES
  if (numtasks != 10) {
    cerr << "please specify 10 tasks" << endl;
    MPI::COMM_WORLD.Abort(2);
    return 2;
  }
#endif

  MPI::Intracomm group_comm = MPI::COMM_WORLD.Split(world_rank % 2, 0);
  const int group_numtasks = group_comm.Get_size();
  const int group_rank = group_comm.Get_rank();

#if ASSERT_10_PROCESSES
  assert(("each group should contain 5 processes", group_numtasks == 5));
#endif

  cout << "MPI task " << world_rank << "," << group_rank << " started..."
       << endl;

  double my_pi = monte_carlo(samples);

  double pi;
  group_comm.Reduce(&my_pi, &pi, 1, MPI_DOUBLE, MPI_SUM, MASTER_RANK);
  if (group_rank == MASTER_RANK) {
    pi = pi / group_numtasks;
    cout << pi << endl;
    cout << "delta to PI: " << abs(PI - pi) << endl;
    if (world_rank == MASTER_RANK) {
      double other_pi;
      MPI::COMM_WORLD.Recv(&other_pi, 1, MPI_DOUBLE, 1, 0);
      cout << "delta between calculations: " << abs(pi - other_pi) << endl;
    } else {
      MPI_Send(&pi, 1, MPI_DOUBLE, MASTER_RANK, 0, MPI_COMM_WORLD);
    }
  }
  MPI::Finalize();
  return 0;
}

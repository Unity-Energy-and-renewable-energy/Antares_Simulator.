#define BOOST_TEST_MODULE hydro remix

#define WIN32_LEAN_AND_MEAN

#include <vector>

#include <boost/test/unit_test.hpp>

namespace Antares::Solver::Simulation
{
bool new_remix_hydro();
std::pair<std::vector<double>, std::vector<double>> new_remix_hydro(
  const std::vector<double>& G,
  const std::vector<double>& H,
  const std::vector<double>& D,
  const std::vector<double>& P_max,
  const std::vector<double>& P_min,
  double initial_level,
  double capa,
  const std::vector<double>& inflow);
} // namespace Antares::Solver::Simulation

using namespace Antares::Solver::Simulation;
using namespace std;

BOOST_AUTO_TEST_CASE(my_first_unit_test)
{
    BOOST_CHECK(new_remix_hydro());
}

BOOST_AUTO_TEST_CASE(my_second_unit_test)
{
    vector<double> G = {1.0, 2.0, 3.0, 4.0, 5.0};
    vector<double> H = {2.0, 3.0, 4.0, 5.0, 6.0};
    vector<double> D = {1.0, 1.5, 2.0, 2.5, 3.0};
    vector<double> P_max = {10.0, 10.0, 10.0, 10.0, 10.0};
    vector<double> P_min = {0.0, 0.0, 0.0, 0.0, 0.0};
    double initial_level = 5.0;
    double capa = 20.0;
    vector<double> inflow = {3.0, 3.0, 3.0, 3.0, 3.0};

    // Call the function
    auto [new_H, new_D] = new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflow);

    BOOST_CHECK(true);
}

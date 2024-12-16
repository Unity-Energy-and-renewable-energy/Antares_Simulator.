#define BOOST_TEST_MODULE hydro remix

#define WIN32_LEAN_AND_MEAN

#include <algorithm>
#include <unit_test_utils.h>
#include <vector>

#include <boost/test/unit_test.hpp>

namespace Antares::Solver::Simulation
{
std::pair<std::vector<double>, std::vector<double>> new_remix_hydro(
  const std::vector<double>& G,
  const std::vector<double>& H,
  const std::vector<double>& D,
  const std::vector<double>& P_max,
  const std::vector<double>& P_min,
  double initial_level,
  double capa,
  const std::vector<double>& inflow);
}

using namespace Antares::Solver::Simulation;

BOOST_AUTO_TEST_CASE(dummy_unit_test___will_be_removed)
{
    std::vector<double> G = {1.0, 2.0, 3.0, 4.0, 5.0};
    std::vector<double> H = {2.0, 3.0, 4.0, 5.0, 6.0};
    std::vector<double> D = {1.0, 1.5, 2.0, 2.5, 3.0};
    std::vector<double> P_max = {10.0, 10.0, 10.0, 10.0, 10.0};
    std::vector<double> P_min = {0.0, 0.0, 0.0, 0.0, 0.0};
    double initial_level = 5.0;
    double capa = 20.0;
    std::vector<double> inflow = {3.0, 3.0, 3.0, 3.0, 3.0};

    auto [new_H, new_D] = new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflow);

    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_CASE(input_arrays_of_different_sizes__exception_raised)
{
    std::vector<double> G, D, P_max, P_min, inflows;
    std::vector<double> H = {0., 0.};
    double initial_level = 0.;
    double capa = 0.;

    BOOST_CHECK_EXCEPTION(new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows),
                          std::invalid_argument,
                          checkMessage("Remix hydro input : arrays of different sizes"));
}

BOOST_AUTO_TEST_CASE(input_init_level_exceeds_capacity__exception_raised)
{
    std::vector<double> G, D, P_max, P_min, inflows;
    std::vector<double> H = {0., 0.};
    double initial_level = 2.;
    double capa = 1.;

    BOOST_CHECK_EXCEPTION(new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows),
                          std::invalid_argument,
                          checkMessage("Remix hydro input : initial level > reservoir capacity"));
}

BOOST_AUTO_TEST_CASE(all_input_arrays_of_size_0__exception_raised)
{
    std::vector<double> G, H, D, P_max, P_min, inflows;
    double initial_level = 0.;
    double capa = 1.;

    BOOST_CHECK_EXCEPTION(new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows),
                          std::invalid_argument,
                          checkMessage("Remix hydro input : all arrays of sizes 0"));
}

BOOST_AUTO_TEST_CASE(H_not_smaller_than_pmax__exception_raised)
{
    std::vector<double> G(5, 0.), D(5, 0.), P_min(5, 0.), inflows(5, 0.);
    std::vector<double> H = {1., 2., 3., 4., 5.};
    std::vector<double> P_max = {2., 2., 2., 4., 5.};
    double initial_level = 0.;
    double capa = 1.;

    BOOST_CHECK_EXCEPTION(new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows),
                          std::invalid_argument,
                          checkMessage("Remix hydro input : H not smaller than Pmax everywhere"));
}

BOOST_AUTO_TEST_CASE(H_not_greater_than_pmin__exception_raised)
{
    std::vector<double> G(5, 0.), D(5, 0.), P_max(5, 1000.), inflows(5, 0.);
    std::vector<double> H = {1., 2., 3., 4., 5.};
    std::vector<double> P_min = {0., 0., 4., 0., 0.};
    double initial_level = 0.;
    double capa = 1.;

    BOOST_CHECK_EXCEPTION(new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows),
                          std::invalid_argument,
                          checkMessage("Remix hydro input : H not greater than Pmin everywhere"));
}

BOOST_AUTO_TEST_CASE(input_is_acceptable__no_exception_raised)
{
    std::vector<double> G = {0.}, H = {0.}, D = {0.}, P_max = {0.}, P_min = {0.}, inflows = {0.};
    double initial_level = 0.;
    double capa = 1.;

    BOOST_CHECK_NO_THROW(new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows));
}

BOOST_AUTO_TEST_CASE(hydro_increases_and_pmax_40mwh___H_is_smoothed_to_mean_H_20mwh)
{
    std::vector<double> P_max(5, 40.);
    std::vector<double> P_min(5, 0.);
    std::vector<double> G(5, 100.);
    std::vector<double> H = {0., 10., 20., 30., 40.}; // we have Pmin <= H <= Pmax
    std::vector<double> D = {80.0, 60., 40., 20., 0.};
    double initial_level = 500.;
    double capa = 1000.;
    std::vector<double> inflows(5, 0.);

    auto [new_H, new_D] = new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows);

    std::vector<double> expected_H = {20., 20., 20., 20., 20.};
    // D such as G + H + D remains constant at each hour
    std::vector<double> expected_D = {60., 50., 40., 30., 20.};
    BOOST_CHECK(new_H == expected_H);
    BOOST_CHECK(new_D == expected_D);
}

BOOST_AUTO_TEST_CASE(Pmax_does_not_impact_results_when_greater_than_40mwh)
{
    std::vector<double> P_max(5, 50.);
    std::vector<double> P_min(5, 0.);
    std::vector<double> G(5, 100.);
    std::vector<double> H = {0., 10., 20., 30., 40.};
    std::vector<double> D = {80.0, 60., 40., 20., 0.};
    double initial_level = 500.;
    double capa = 1000.;
    std::vector<double> inflows(5, 0.);

    auto [new_H, new_D] = new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows);

    std::vector<double> expected_H = {20., 20., 20., 20., 20.};
    // D such as G + H + D remains constant at each hour
    std::vector<double> expected_D = {60., 50., 40., 30., 20.};
    BOOST_CHECK(new_H == expected_H);
    BOOST_CHECK(new_D == expected_D);
}

BOOST_AUTO_TEST_CASE(hydro_decreases_and_pmax_40mwh___H_is_smoothed_to_mean_H_20mwh)
{
    std::vector<double> P_max(5, 40.);
    std::vector<double> P_min(5, 0.);
    std::vector<double> G(5, 100.);
    std::vector<double> H = {40., 30., 20., 10., 0.};
    std::vector<double> D = {0., 20., 40., 60., 80.};
    double initial_level = 500.;
    double capa = 1000.;
    std::vector<double> inflows(5, 0.);

    auto [new_H, new_D] = new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows);

    std::vector<double> expected_H = {20., 20., 20., 20., 20.};
    // D such as G + H + D remains constant at each hour
    std::vector<double> expected_D = {20., 30., 40., 50., 60.};
    BOOST_CHECK(new_H == expected_H);
    BOOST_CHECK(new_D == expected_D);
}

BOOST_AUTO_TEST_CASE(influence_of_pmax)
{
    std::vector<double> P_max(5, 20.);
    std::vector<double> P_min(5, 0.);

    // G decreases
    std::vector<double> G = {100., 80., 60., 40., 20.};

    // H is flat and must respect H <= Pmax everywhere
    std::vector<double> H = {20., 20., 20., 20., 20.};
    std::vector<double> D = {50., 50., 50., 50., 50.};
    double initial_level = 500.;
    double capa = 1000.;
    std::vector<double> inflows(5, 0.);

    // What we expect to happen :
    // Algorithm tends to flatten G + H, so it would require H to increase.
    // But H is limited by P_max. So Algo does nothing in the end.
    auto [new_H, new_D] = new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows);

    std::vector<double> expected_H = {20., 20., 20., 20., 20.};
    std::vector<double> expected_D = {50., 50., 50., 50., 50.};
    BOOST_CHECK(new_H == expected_H);
    BOOST_CHECK(new_D == expected_D);
}

BOOST_AUTO_TEST_CASE(influence_of_pmin)
{
    std::vector<double> P_max(5, std::numeric_limits<double>::max());
    std::vector<double> P_min(5, 20.);

    // G decreases
    std::vector<double> G = {100., 80., 60., 40., 20.};

    // H is flat and must respect  Pmin <= H <= Pmax everywhere
    std::vector<double> H = {20., 20., 20., 20., 20.};
    std::vector<double> D = {50., 50., 50., 50., 50.};
    double initial_level = 500.;
    double capa = 1000.;
    std::vector<double> inflows(5, 0.);

    // What we expect to happen :
    // Algorithm tends to flatten G + H, so it would require H to decrease.
    // But H is low bounded by P_min. So Algo does nothing in the end.
    auto [new_H, new_D] = new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows);

    std::vector<double> expected_H = {20., 20., 20., 20., 20.};
    std::vector<double> expected_D = {50., 50., 50., 50., 50.};
    BOOST_CHECK(new_H == expected_H);
    BOOST_CHECK(new_D == expected_D);
}

// Ideas for building further tests :
// ================================
// - Remix hydro algorithm seems symmetrical (if we have input vectors and corresponding output
//   vectors, run the algo on reversed vectors gives reversed output result vectors)
// - How to test influence of Pmin ?
// - After running remix hydro algo, sum(H), sum(H + D) must remain the same.
// - influence of D : low values of G + H are searched where D > 0 (not where D >= 0)

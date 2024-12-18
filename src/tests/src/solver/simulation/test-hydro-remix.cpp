#define BOOST_TEST_MODULE hydro remix

#define WIN32_LEAN_AND_MEAN

#include <algorithm>
#include <unit_test_utils.h>
#include <vector>

#include <boost/test/unit_test.hpp>

namespace Antares::Solver::Simulation
{
// gp : instead of this, we should make a header asociated to hydro-remix-new.cpp
//      ==> hydro-remix-new.h
struct RemixHydroOutput
{
    std::vector<double> new_H;
    std::vector<double> new_D;
    std::vector<double> levels;
};

RemixHydroOutput new_remix_hydro(const std::vector<double>& G,
                                 const std::vector<double>& H,
                                 const std::vector<double>& D,
                                 const std::vector<double>& P_max,
                                 const std::vector<double>& P_min,
                                 double initial_level,
                                 double capa,
                                 const std::vector<double>& inflow,
                                 const std::vector<double>& overflow,
                                 const std::vector<double>& pump,
                                 const std::vector<double>& S,
                                 const std::vector<double>& DTG_MRG);

} // namespace Antares::Solver::Simulation

using namespace Antares::Solver::Simulation;

BOOST_AUTO_TEST_CASE(input_arrays_of_different_sizes__exception_raised)
{
    std::vector<double> G, D, P_max, P_min, inflows, ovf, pump, S, DTG_MRG;
    std::vector<double> H = {0., 0.};
    double initial_level = 0.;
    double capa = 0.;

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage("Remix hydro input : arrays of different sizes"));
}

BOOST_AUTO_TEST_CASE(input_init_level_exceeds_capacity__exception_raised)
{
    std::vector<double> G, D, P_max, P_min, inflows, ovf, pump, S, DTG_MRG;
    std::vector<double> H = {0., 0.};
    double initial_level = 2.;
    double capa = 1.;

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage("Remix hydro input : initial level > reservoir capacity"));
}

BOOST_AUTO_TEST_CASE(all_input_arrays_of_size_0__exception_raised)
{
    std::vector<double> G, H, D, P_max, P_min, inflows, ovf, pump, S, DTG_MRG;
    double initial_level = 0.;
    double capa = 1.;

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage("Remix hydro input : all arrays of sizes 0"));
}

BOOST_AUTO_TEST_CASE(H_not_smaller_than_pmax__exception_raised)
{
    std::vector<double> G(5, 0.), D(5, 0.), P_min(5, 0.), inflows(5, 0.);
    std::vector<double> ovf(5, 0.), pump(5, 0.), S(5, 0.), DTG_MRG(5, 0.);
    std::vector<double> H = {1., 2., 3., 4., 5.};
    std::vector<double> P_max = {2., 2., 2., 4., 5.};
    double initial_level = 0.;
    double capa = 1.;

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage("Remix hydro input : H not smaller than Pmax everywhere"));
}

BOOST_AUTO_TEST_CASE(H_not_greater_than_pmin__exception_raised)
{
    std::vector<double> G(5, 0.), D(5, 0.), P_max(5, 1000.), inflows(5, 0.);
    std::vector<double> ovf(5, 0.), pump(5, 0.), S(5, 0.), DTG_MRG(5, 0.);
    std::vector<double> H = {1., 2., 3., 4., 5.};
    std::vector<double> P_min = {0., 0., 4., 0., 0.};
    double initial_level = 0.;
    double capa = 1.;

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage("Remix hydro input : H not greater than Pmin everywhere"));
}

BOOST_AUTO_TEST_CASE(input_is_acceptable__no_exception_raised)
{
    std::vector<double> G = {0.}, H = {0.}, D = {0.}, P_max = {0.}, P_min = {0.}, inflows = {0.};
    std::vector<double> ovf = {0.}, pump = {0.}, S = {0.}, DTG_MRG = {0.};
    double initial_level = 0.;
    double capa = 1.;

    BOOST_CHECK_NO_THROW(
      new_remix_hydro(G, H, D, P_max, P_min, initial_level, capa, inflows, ovf, pump, S, DTG_MRG));
}

BOOST_AUTO_TEST_CASE(hydro_increases_and_pmax_40mwh___H_is_flattened_to_mean_H_20mwh)
{
    std::vector<double> P_max(5, 40.);
    std::vector<double> P_min(5, 0.);
    std::vector<double> G(5, 100.);
    std::vector<double> H = {0., 10., 20., 30., 40.}; // we have Pmin <= H <= Pmax
    std::vector<double> D = {80.0, 60., 40., 20., 0.};
    double initial_level = 500.;
    double capa = 1000.;
    std::vector<double> inflows(5, 0.);
    std::vector<double> ovf(5, 0.), pump(5, 0.), S(5, 0.), DTG_MRG(5, 0.);

    auto [new_H, new_D, _] = new_remix_hydro(G,
                                             H,
                                             D,
                                             P_max,
                                             P_min,
                                             initial_level,
                                             capa,
                                             inflows,
                                             ovf,
                                             pump,
                                             S,
                                             DTG_MRG);

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
    std::vector<double> ovf(5, 0.), pump(5, 0.), S(5, 0.), DTG_MRG(5, 0.);

    auto [new_H, new_D, _] = new_remix_hydro(G,
                                             H,
                                             D,
                                             P_max,
                                             P_min,
                                             initial_level,
                                             capa,
                                             inflows,
                                             ovf,
                                             pump,
                                             S,
                                             DTG_MRG);

    std::vector<double> expected_H = {20., 20., 20., 20., 20.};
    // D such as G + H + D remains constant at each hour
    std::vector<double> expected_D = {60., 50., 40., 30., 20.};
    BOOST_CHECK(new_H == expected_H);
    BOOST_CHECK(new_D == expected_D);
}

BOOST_AUTO_TEST_CASE(hydro_decreases_and_pmax_40mwh___H_is_flattened_to_mean_H_20mwh)
{
    std::vector<double> P_max(5, 40.);
    std::vector<double> P_min(5, 0.);
    std::vector<double> G(5, 100.);
    std::vector<double> H = {40., 30., 20., 10., 0.};
    std::vector<double> D = {0., 20., 40., 60., 80.};
    double initial_level = 500.;
    double capa = 1000.;
    std::vector<double> inflows(5, 0.);
    std::vector<double> ovf(5, 0.), pump(5, 0.), S(5, 0.), DTG_MRG(5, 0.);

    auto [new_H, new_D, _] = new_remix_hydro(G,
                                             H,
                                             D,
                                             P_max,
                                             P_min,
                                             initial_level,
                                             capa,
                                             inflows,
                                             ovf,
                                             pump,
                                             S,
                                             DTG_MRG);

    std::vector<double> expected_H = {20., 20., 20., 20., 20.};
    // D such as G + H + D remains constant at each hour
    std::vector<double> expected_D = {20., 30., 40., 50., 60.};
    BOOST_CHECK(new_H == expected_H);
    BOOST_CHECK(new_D == expected_D);
}

BOOST_AUTO_TEST_CASE(influence_of_pmax, *boost::unit_test::tolerance(0.01))
{
    std::vector<double> P_min(5, 0.);

    // G decreases
    std::vector<double> G = {100., 80., 60., 40., 20.};

    // H is flat and must respect H <= Pmax everywhere
    std::vector<double> H = {20., 20., 20., 20., 20.};
    std::vector<double> D = {50., 50., 50., 50., 50.};
    double initial_level = 500.;
    double capa = 1000.;
    std::vector<double> inflows(5, 0.);
    std::vector<double> ovf(5, 0.), pump(5, 0.), S(5, 0.), DTG_MRG(5, 0.);

    // 1. Algorithm tends to flatten G + H, so it would require H to increase.
    // Proof :
    std::vector<double> P_max(5., std::numeric_limits<double>::max());

    auto [new_H1, new_D1, L] = new_remix_hydro(G,
                                               H,
                                               D,
                                               P_max,
                                               P_min,
                                               initial_level,
                                               capa,
                                               inflows,
                                               ovf,
                                               pump,
                                               S,
                                               DTG_MRG);

    std::vector<double> expected_H1 = {0., 0., 13.33, 33.33, 53.33};
    BOOST_TEST(new_H1 == expected_H1, boost::test_tools::per_element());

    // 2. But H is limited by P_max. So Algo does nothing in the end.
    // Proof :
    P_max = {20., 20., 20., 20., 20.};
    auto [new_H2, new_D2, _] = new_remix_hydro(G,
                                               H,
                                               D,
                                               P_max,
                                               P_min,
                                               initial_level,
                                               capa,
                                               inflows,
                                               ovf,
                                               pump,
                                               S,
                                               DTG_MRG);

    std::vector<double> expected_H2 = {20., 20., 20., 20., 20.};
    std::vector<double> expected_D2 = {50., 50., 50., 50., 50.};
    BOOST_CHECK(new_H2 == expected_H2);
    BOOST_CHECK(new_D2 == expected_D2);
}

BOOST_AUTO_TEST_CASE(influence_of_pmin, *boost::unit_test::tolerance(0.01))
{
    std::vector<double> P_max(5, std::numeric_limits<double>::max());

    // G decreases
    std::vector<double> G = {100., 80., 60., 40., 20.};

    // H is flat and must respect  Pmin <= H <= Pmax everywhere
    std::vector<double> H = {20., 20., 20., 20., 20.};
    std::vector<double> D = {50., 50., 50., 50., 50.};
    double initial_level = 500.;
    double capa = 1000.;
    std::vector<double> inflows(5, 0.);
    std::vector<double> ovf(5, 0.), pump(5, 0.), S(5, 0.), DTG_MRG(5, 0.);

    // 1. Algorithm tends to flatten G + H, so it would require H to increase.
    std::vector<double> P_min(5, 0.);
    auto [new_H1, new_D1, L] = new_remix_hydro(G,
                                               H,
                                               D,
                                               P_max,
                                               P_min,
                                               initial_level,
                                               capa,
                                               inflows,
                                               ovf,
                                               pump,
                                               S,
                                               DTG_MRG);
    std::vector<double> expected_H1 = {0., 0., 13.33, 33.33, 53.33};
    BOOST_TEST(new_H1 == expected_H1, boost::test_tools::per_element());

    // 2. But H is low bounded by P_min. So Algo does nothing in the end.
    P_min = {20., 20., 20., 20., 20.};
    auto [new_H2, new_D2, _] = new_remix_hydro(G,
                                               H,
                                               D,
                                               P_max,
                                               P_min,
                                               initial_level,
                                               capa,
                                               inflows,
                                               ovf,
                                               pump,
                                               S,
                                               DTG_MRG);

    std::vector<double> expected_H2 = {20., 20., 20., 20., 20.};
    std::vector<double> expected_D2 = {50., 50., 50., 50., 50.};
    BOOST_CHECK(new_H2 == expected_H2);
    BOOST_CHECK(new_D2 == expected_D2);
}

BOOST_AUTO_TEST_CASE(H_is_already_flat___remix_is_useless__level_easily_computed)
{
    // Not important
    std::vector<double> P_max(5, 25.), P_min(5, 0.), G(5, 0.), S(5, 0.), DTG_MRG(5, 0.);
    std::vector<double> D(5, 10.);
    double capa = 1000.;

    // Used for level computations
    double initial_level = 500.;
    std::vector<double> ovf(5, 25.), H(5, 20.);        // Cause levels to lower
    std::vector<double> inflows(5, 15.), pump(5, 10.); // Cause levels to raise

    auto [new_H, new_D, levels] = new_remix_hydro(G,
                                                  H,
                                                  D,
                                                  P_max,
                                                  P_min,
                                                  initial_level,
                                                  capa,
                                                  inflows,
                                                  ovf,
                                                  pump,
                                                  S,
                                                  DTG_MRG);

    std::vector<double> expected_levels = {480., 460., 440., 420., 400.};
    BOOST_TEST(levels == expected_levels, boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(what_if_levels_are_up_bounded_by_capacity)
{
    // Not important
    std::vector<double> P_max(5, 25.), P_min(5, 0.), G(5, 0.), S(5, 0.), DTG_MRG(5, 0.);
    std::vector<double> D(5, 10.);

    // Used for level computations
    double initial_level = 500.;
    double capacity = 550.;
    std::vector<double> ovf(5, 15.), H(5, 10.);        // Cause levels to lower
    std::vector<double> inflows(5, 25.), pump(5, 20.); // Cause levels to raise

    auto [new_H, new_D, levels] = new_remix_hydro(G,
                                                  H,
                                                  D,
                                                  P_max,
                                                  P_min,
                                                  initial_level,
                                                  capacity,
                                                  inflows,
                                                  ovf,
                                                  pump,
                                                  S,
                                                  DTG_MRG);

    // Bad ! Levels not limited by capacity.
    std::vector<double> expected_levels = {520., 540., 560., 580., 600.};
    BOOST_TEST(levels == expected_levels, boost::test_tools::per_element());

    BOOST_CHECK(std::ranges::all_of(levels, [&capacity](double e) { return e <= capacity; }));
}

// Ideas for building further tests :
// ================================
// - Remix hydro algorithm seems symmetrical (if we have input vectors and corresponding output
//   vectors, run the algo on reversed vectors gives reversed output result vectors)
// - After running remix hydro algo, sum(H), sum(H + D) must remain the same.
// - influence of D : low values of G + H are searched where D > 0 (not where D == 0)

// Possible simplifications / clarifications of the algorithm itself :
// - remove french from variable names
// - the algo is flat, it's C (not C++), it should be divided in a small number of steps
// - max_pic is a up hydro production margin (H_up_mrg)
// - max_creux is a down hydro production margin (H_down_mrg)
// - an iter updates new_H : it's its main job. So new_D could be updated from new_H at the
//   end of an iteration, separately.
// - they are 3 while loops. 2 loops should be enough (the iteration loop and
//   another one simply updating new_H and new_D)

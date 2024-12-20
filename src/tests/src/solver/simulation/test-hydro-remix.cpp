#define BOOST_TEST_MODULE hydro remix

#define WIN32_LEAN_AND_MEAN

#include <algorithm>
#include <unit_test_utils.h>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "include/antares/solver/simulation/hydro-remix-new.h"

using namespace Antares::Solver::Simulation;

template<unsigned int size>
struct InputFixture
{
    InputFixture()
    {
        G.assign(size, 0.);
        H.assign(size, 0.);
        D.assign(size, 0.);
        P_max.assign(size, std::numeric_limits<double>::max());
        P_min.assign(size, 0.);
        inflows.assign(size, 0.);
        ovf.assign(size, 0.);
        pump.assign(size, 0.);
        S.assign(size, 0.);
        DTG_MRG.assign(size, 0.);
    }

    std::vector<double> G, H, D, P_max, P_min, inflows, ovf, pump, S, DTG_MRG;
    double init_level = 0.;
    double capacity = std::numeric_limits<double>::max();
};

BOOST_FIXTURE_TEST_CASE(input_vectors_of_different_sizes__exception_raised, InputFixture<0>)
{
    H = {0., 0.};

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, init_level, capacity, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage("Remix hydro input : arrays of different sizes"));
}

BOOST_FIXTURE_TEST_CASE(input_init_level_exceeds_capacity__exception_raised, InputFixture<0>)
{
    H = {0., 0.};
    init_level = 2.;
    capacity = 1.;

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, init_level, capacity, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage("Remix hydro input : initial level > reservoir capacity"));
}

BOOST_FIXTURE_TEST_CASE(all_input_arrays_of_size_0__exception_raised, InputFixture<0>)
{
    init_level = 0.;
    capacity = 1.;

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, init_level, capacity, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage("Remix hydro input : all arrays of sizes 0"));
}

BOOST_FIXTURE_TEST_CASE(H_not_smaller_than_pmax__exception_raised, InputFixture<5>)
{
    H = {1., 2., 3., 4., 5.};
    P_max = {2., 2., 2., 4., 5.};
    init_level = 0.;
    capacity = 1.;

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, init_level, capacity, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage("Remix hydro input : H not smaller than Pmax everywhere"));
}

BOOST_FIXTURE_TEST_CASE(H_not_greater_than_pmin__exception_raised, InputFixture<5>)
{
    H = {1., 2., 3., 4., 5.};
    P_min = {0., 0., 4., 0., 0.};
    init_level = 0.;
    capacity = 1.;

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, init_level, capacity, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage("Remix hydro input : H not greater than Pmin everywhere"));
}

BOOST_FIXTURE_TEST_CASE(input_is_acceptable__no_exception_raised, InputFixture<1>)
{
    init_level = 0.;
    capacity = 1.;

    BOOST_CHECK_NO_THROW(
      new_remix_hydro(G, H, D, P_max, P_min, init_level, capacity, inflows, ovf, pump, S, DTG_MRG));
}

BOOST_FIXTURE_TEST_CASE(hydro_increases_and_pmax_40mwh___H_is_flattened_to_mean_H_20mwh,
                        InputFixture<5>)
{
    std::ranges::fill(P_max, 40.);
    std::ranges::fill(G, 100.);
    H = {0., 10., 20., 30., 40.}; // we have Pmin <= H <= Pmax
    D = {80.0, 60., 40., 20., 0.};
    init_level = 500.;
    capacity = 1000.;

    auto [new_H, new_D, _] = new_remix_hydro(G,
                                             H,
                                             D,
                                             P_max,
                                             P_min,
                                             init_level,
                                             capacity,
                                             inflows,
                                             ovf,
                                             pump,
                                             S,
                                             DTG_MRG);

    std::vector<double> expected_H = {20., 20., 20., 20., 20.};
    // D such as G + H + D remains flat
    std::vector<double> expected_D = {60., 50., 40., 30., 20.};
    BOOST_CHECK(new_H == expected_H);
    BOOST_CHECK(new_D == expected_D);
}

BOOST_FIXTURE_TEST_CASE(Pmax_does_not_impact_results_when_greater_than_40mwh, InputFixture<5>)
{
    std::ranges::fill(P_max, 50.);
    std::ranges::fill(G, 100.);
    H = {0., 10., 20., 30., 40.};
    D = {80.0, 60., 40., 20., 0.};
    init_level = 500.;
    capacity = 1000.;

    auto [new_H, new_D, _] = new_remix_hydro(G,
                                             H,
                                             D,
                                             P_max,
                                             P_min,
                                             init_level,
                                             capacity,
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

BOOST_FIXTURE_TEST_CASE(hydro_decreases_and_pmax_40mwh___H_is_flattened_to_mean_H_20mwh,
                        InputFixture<5>)
{
    std::ranges::fill(P_max, 40.);
    std::ranges::fill(G, 100.);
    H = {40., 30., 20., 10., 0.};
    D = {0., 20., 40., 60., 80.};
    init_level = 500.;
    capacity = 1000.;

    auto [new_H, new_D, _] = new_remix_hydro(G,
                                             H,
                                             D,
                                             P_max,
                                             P_min,
                                             init_level,
                                             capacity,
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

BOOST_FIXTURE_TEST_CASE(influence_of_pmax, InputFixture<5>, *boost::unit_test::tolerance(0.01))
{
    // G decreases
    G = {100., 80., 60., 40., 20.};

    // H is flat and must respect H <= Pmax everywhere
    H = {20., 20., 20., 20., 20.};
    D = {50., 50., 50., 50., 50.};
    init_level = 500.;
    capacity = 1000.;

    // 1. Algorithm tends to flatten G + H, so it would require H to increase.
    // Proof :
    auto [new_H1, new_D1, L] = new_remix_hydro(G,
                                               H,
                                               D,
                                               P_max,
                                               P_min,
                                               init_level,
                                               capacity,
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
                                               init_level,
                                               capacity,
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

BOOST_FIXTURE_TEST_CASE(influence_of_pmin, InputFixture<5>, *boost::unit_test::tolerance(0.01))
{
    // G decreases
    G = {100., 80., 60., 40., 20.};

    // H is flat and must respect  Pmin <= H <= Pmax everywhere
    H = {20., 20., 20., 20., 20.};
    D = {50., 50., 50., 50., 50.};
    init_level = 500.;
    capacity = 1000.;

    // 1. Algorithm tends to flatten G + H, so it would require H to increase.
    auto [new_H1, new_D1, L] = new_remix_hydro(G,
                                               H,
                                               D,
                                               P_max,
                                               P_min,
                                               init_level,
                                               capacity,
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
                                               init_level,
                                               capacity,
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

BOOST_FIXTURE_TEST_CASE(H_is_already_flat___remix_is_useless__level_easily_computed,
                        InputFixture<5>)
{
    init_level = 500.;
    std::ranges::fill(ovf, 25.);     // Cause levels to decrease
    std::ranges::fill(H, 20.);       // Cause levels to decrease
    std::ranges::fill(inflows, 15.); // Cause levels to increase
    std::ranges::fill(pump, 10.);    // Cause levels to increase

    auto [new_H, new_D, levels] = new_remix_hydro(G,
                                                  H,
                                                  D,
                                                  P_max,
                                                  P_min,
                                                  init_level,
                                                  capacity,
                                                  inflows,
                                                  ovf,
                                                  pump,
                                                  S,
                                                  DTG_MRG);

    std::vector<double> expected_levels = {480., 460., 440., 420., 400.};
    BOOST_TEST(levels == expected_levels, boost::test_tools::per_element());
}

BOOST_FIXTURE_TEST_CASE(input_leads_to_levels_over_capacity___exception_raised, InputFixture<5>)
{
    init_level = 500.;
    capacity = 550.;
    std::ranges::fill(ovf, 15);     // Cause levels to decrease
    std::ranges::fill(H, 10);       // Cause levels to decrease
    std::ranges::fill(inflows, 25); // Cause levels to increase
    std::ranges::fill(pump, 20);    // Cause levels to increase

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, init_level, capacity, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage(
        "Remix hydro input : levels computed from input don't respect reservoir bounds"));
}

BOOST_FIXTURE_TEST_CASE(input_leads_to_levels_less_than_zero___exception_raised, InputFixture<5>)
{
    init_level = 50.;
    std::ranges::fill(ovf, 30);    // Cause levels to decrease
    std::ranges::fill(H, 10);      // Cause levels to decrease
    std::ranges::fill(inflows, 5); // Cause levels to increase
    std::ranges::fill(pump, 10);   // Cause levels to increase

    BOOST_CHECK_EXCEPTION(
      new_remix_hydro(G, H, D, P_max, P_min, init_level, capacity, inflows, ovf, pump, S, DTG_MRG),
      std::invalid_argument,
      checkMessage(
        "Remix hydro input : levels computed from input don't respect reservoir bounds"));
}

BOOST_FIXTURE_TEST_CASE(influence_of_capacity_on_algorithm___case_where_no_influence,
                        InputFixture<10>,
                        *boost::unit_test::tolerance(0.001))
{
    std::ranges::fill(D, 20); // Not important for this test

    // H oscillates between 10 and 20 (new H will be flattened to 15 everywhere)
    H = {10., 20., 10., 20., 10., 20., 10., 20., 10., 20.};
    // First inflows > H, then inflows < H. Consequence : levels first increase, then decrease.
    inflows = {25., 25., 25., 25., 25., 5., 5., 5., 5., 5.};
    init_level = 100.;
    // H and inflows result in : input_levels = {115, 120, 135, 140, 155, 140, 135, 120, 115, 100}
    // Note sup(input_levels) = 155

    // Case 1 : capacity relaxed (infinite by default) ==> leads to optimal solution (H is flat)
    auto [new_H, new_D, L] = new_remix_hydro(G,
                                             H,
                                             D,
                                             P_max,
                                             P_min,
                                             init_level,
                                             capacity,
                                             inflows,
                                             ovf,
                                             pump,
                                             S,
                                             DTG_MRG);

    std::vector<double> expected_H(10, 15.); // H is flat and is 15. (means of initial H)
    // Levels associated to new H are such as sup(L) = 150. < sup(input_levels) = 155
    std::vector<double> expected_L = {110., 120., 130., 140., 150., 140., 130., 120., 110., 100.};
    BOOST_TEST(new_H == expected_H, boost::test_tools::per_element());
    BOOST_TEST(L == expected_L, boost::test_tools::per_element());

    // Case 2 : now, if we lower capacity to sup(input_levels) = 155, we should
    // have H and L identical to previously : this value of capacity should
    // not have an influence on H and levels as results of the algorithm.
    capacity = 155.;
    auto [new_H2, new_D2, L2] = new_remix_hydro(G,
                                                H,
                                                D,
                                                P_max,
                                                P_min,
                                                init_level,
                                                capacity,
                                                inflows,
                                                ovf,
                                                pump,
                                                S,
                                                DTG_MRG);

    BOOST_TEST(new_H2 == expected_H, boost::test_tools::per_element());
    BOOST_TEST(L2 == expected_L, boost::test_tools::per_element());
}

BOOST_FIXTURE_TEST_CASE(lowering_capacity_too_low_leads_to_suboptimal_solution_for_GplusH,
                        InputFixture<10>,
                        *boost::unit_test::tolerance(0.001))
{
    std::ranges::fill(D, 20); // Not important for this test

    // H oscillates between 10 and 20 (new H will be flattened to 15 everywhere)
    H = {20., 10., 20., 10., 20., 10., 20., 10., 20., 10.};
    // First inflows > H, then inflows < H. Consequence : levels first increase, then decrease.
    inflows = {25., 25., 25., 25., 25., 5., 5., 5., 5., 5.};
    init_level = 100.;
    // H and inflows lead to have :
    // input_levels = {105, 120, 125, 140, 145, 140, 125, 120, 105,100}
    // Note sup(input_levels) = 145

    // Case 1 : capacity relaxed (infinite by default) ==> leads to optimal solution (H is flat)
    auto [new_H, new_D, L] = new_remix_hydro(G,
                                             H,
                                             D,
                                             P_max,
                                             P_min,
                                             init_level,
                                             capacity,
                                             inflows,
                                             ovf,
                                             pump,
                                             S,
                                             DTG_MRG);

    std::vector<double> expected_H(10, 15.); // H is flat and is 15. (means of initial H)
    // Levels associated to new H are such as sup(L) = 150. > sup(input_levels) = 145
    std::vector<double> expected_L = {110., 120., 130., 140., 150., 140., 130., 120., 110., 100.};
    BOOST_TEST(new_H == expected_H, boost::test_tools::per_element());
    BOOST_TEST(L == expected_L, boost::test_tools::per_element());

    // Case 2 : we lower capacity to sup(input_levels) = 145.
    // This makes input acceptable for algo : levels computed from input have an
    // up bound <= capacity
    // But this time levels can not increase up to sup(L) = 150., as it would if capacity
    // was infinite. Therefore, solution found is suboptimal : we expect to get an
    // output H flat by interval, not flat on the whole domain.
    capacity = 145.;
    auto [new_H2, new_D2, L2] = new_remix_hydro(G,
                                                H,
                                                D,
                                                P_max,
                                                P_min,
                                                init_level,
                                                capacity,
                                                inflows,
                                                ovf,
                                                pump,
                                                S,
                                                DTG_MRG);

    // new_H2 is flat by interval
    std::vector<double> expected_H2 = {16., 16., 16., 16., 16., 14., 14., 14., 14., 14.};
    BOOST_TEST(new_H2 == expected_H2, boost::test_tools::per_element());
}

BOOST_FIXTURE_TEST_CASE(lowering_initial_level_too_low_leads_to_suboptimal_solution_for_GplusH,
                        InputFixture<10>,
                        *boost::unit_test::tolerance(0.001))
{
    std::ranges::fill(D, 20); // Not important for this test

    // H oscillates between 20 and 30 (new H will be flattened to 25 everywhere)
    H = {20., 30., 20., 30., 20., 30., 20., 30., 20., 30.};
    // First inflows < H, then inflows > H. Consequence : levels first decrease, then increase.
    inflows = {5., 5., 5., 5., 5., 45., 45., 45., 45., 45.};
    capacity = std::numeric_limits<double>::max();
    init_level = 100.;
    // H and inflows result in : input_levels = {85, 60, 45, 20, 5, 20, 45, 60, 85, 100}
    // Note : inf(input_levels) = 5

    // Case 1 : init level (== 100) is high enough so that input levels (computed from input data)
    // are acceptable for algorithm (input levels >= 0.), and running algorithm leads to optimal
    // solution (new_H is flat)
    auto [new_H, new_D, L] = new_remix_hydro(G,
                                             H,
                                             D,
                                             P_max,
                                             P_min,
                                             init_level,
                                             capacity,
                                             inflows,
                                             ovf,
                                             pump,
                                             S,
                                             DTG_MRG);

    std::vector<double> expected_H(10, 25.); // H is flat and is 25. (means of initial H)
    // Levels associated to new H are such as inf(L) = 0. > inf(input_levels) = 5
    std::vector<double> expected_L = {80., 60., 40., 20., 0., 20., 40., 60., 80., 100.};
    BOOST_TEST(new_H == expected_H, boost::test_tools::per_element());
    BOOST_TEST(L == expected_L, boost::test_tools::per_element());

    // Case 2 : we lower initial level. Input data are still acceptable
    // for algorithm (despite the new init level), algorithm will have to take the levels lower
    // bound (0.) into account. As the levels change, the solution new_H will be suboptimal, that is
    // flat by interval (not flat on the whole domain).
    init_level = 95.;
    auto [new_H2, new_D2, L2] = new_remix_hydro(G,
                                                H,
                                                D,
                                                P_max,
                                                P_min,
                                                init_level,
                                                capacity,
                                                inflows,
                                                ovf,
                                                pump,
                                                S,
                                                DTG_MRG);

    // new_H2 is flat by interval
    std::vector<double> expected_H2 = {24., 24., 24., 24., 24., 26., 26., 26., 26., 26.};
    BOOST_TEST(new_H2 == expected_H2, boost::test_tools::per_element());
}

BOOST_FIXTURE_TEST_CASE(influence_of_initial_level_on_algorithm___case_where_no_influence,
                        InputFixture<10>,
                        *boost::unit_test::tolerance(0.001))
{
    std::ranges::fill(D, 20); // Not important for this test

    // H oscillates between 10 and 20 (new H will be flattened to 15 everywhere)
    H = {20., 10., 20., 10., 20., 10., 20., 10., 20., 10.};
    // First inflows < H, then inflows > H. Consequence : levels first decrease, then increase.
    inflows = {5., 5., 5., 5., 5., 25., 25., 25., 25., 25.};
    init_level = 100.;
    // H and inflows are such as inf(input_levels) = 45

    // Case 1 : init level (== 100) is high enough so that input levels (computed from input data)
    // are acceptable by algorithm, and levels computed by algorithm (output) are optimal, that
    // is computed from a optimal (that is flat) new_H.
    auto [new_H, new_D, L] = new_remix_hydro(G,
                                             H,
                                             D,
                                             P_max,
                                             P_min,
                                             init_level,
                                             capacity,
                                             inflows,
                                             ovf,
                                             pump,
                                             S,
                                             DTG_MRG);

    std::vector<double> expected_H(10, 15.); // H is flat and is 15. (means of initial H)
    // Levels associated to new H are such as inf(L) = 50 > inf(input_levels) = 45
    std::vector<double> expected_L = {90., 80., 70., 60., 50., 60., 70., 80., 90., 100.};
    BOOST_TEST(new_H == expected_H, boost::test_tools::per_element());
    BOOST_TEST(L == expected_L, boost::test_tools::per_element());

    // Case 2 : now we lower initial level down to 55.
    // In this way, input data is still acceptable for algorithm
    // and algorithm won't have to take the levels lower bound (0.) into account.
    // The solution new_H will be optimal, that is flat by interval.
    init_level = 55.;
    auto [new_H2, new_D2, L2] = new_remix_hydro(G,
                                                H,
                                                D,
                                                P_max,
                                                P_min,
                                                init_level,
                                                capacity,
                                                inflows,
                                                ovf,
                                                pump,
                                                S,
                                                DTG_MRG);

    // new_H2 is flat (and optimal)
    std::vector<double> expected_H2(10, 15.);
    BOOST_TEST(new_H2 == expected_H2, boost::test_tools::per_element());
}

// Ideas for building further tests :
// ================================
// - Remix hydro algorithm seems symmetrical (if we have input vectors and corresponding output
//   vectors, run the algo on reversed vectors gives reversed output result vectors)
// - After running remix hydro algo, sum(H), sum(H + D) must remain the same.
// - influence of D : low values of G + H are searched where D > 0 (not where D == 0)
// -

// Possible simplifications / clarifications of the algorithm itself :
// - remove french from variable names
// - the algo is flat, it's C (not C++), it should be divided in a small number of steps
// - max_pic is an up hydro production margin (H_up_mrg)
// - max_creux is a down hydro production margin (H_down_mrg)
// - an iter updates new_H : it's its main job. So new_D could be updated from new_H at the
//   end of an iteration, separately.
// - they are 3 while loops. 2 loops should be enough (the iteration loop and
//   another one simply updating new_H and new_D)

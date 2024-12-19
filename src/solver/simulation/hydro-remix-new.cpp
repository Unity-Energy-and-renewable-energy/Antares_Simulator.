#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <vector>

namespace Antares::Solver::Simulation
{

int find_min_index(const std::vector<double>& G_plus_H,
                   const std::vector<double>& new_D,
                   const std::vector<double>& new_H,
                   const std::vector<int>& tried_creux,
                   const std::vector<double>& P_max,
                   const std::vector<bool>& filter_hours_remix,
                   double top)
{
    double min_val = top;
    int min_idx = -1;
    for (int i = 0; i < G_plus_H.size(); ++i)
    {
        if (new_D[i] > 0 && new_H[i] < P_max[i] && tried_creux[i] == 0 && filter_hours_remix[i])
        {
            if (G_plus_H[i] < min_val)
            {
                min_val = G_plus_H[i];
                min_idx = i;
            }
        }
    }
    return min_idx;
}

int find_max_index(const std::vector<double>& G_plus_H,
                   const std::vector<double>& new_H,
                   const std::vector<int>& tried_pic,
                   const std::vector<double>& P_min,
                   const std::vector<bool>& filter_hours_remix,
                   double ref_value,
                   double eps)
{
    double max_val = 0;
    int max_idx = -1;
    for (int i = 0; i < G_plus_H.size(); ++i)
    {
        if (new_H[i] > P_min[i] && G_plus_H[i] >= ref_value + eps && tried_pic[i] == 0
            && filter_hours_remix[i])
        {
            if (G_plus_H[i] > max_val)
            {
                max_val = G_plus_H[i];
                max_idx = i;
            }
        }
    }
    return max_idx;
}

static bool isLessThan(const std::vector<double>& a, const std::vector<double>& b)
{
    std::vector<double> a_minus_b;
    std::ranges::transform(a, b, std::back_inserter(a_minus_b), std::minus<double>());
    return std::ranges::all_of(a_minus_b, [](const double& e) { return e <= 0.; });
}

static bool isLessThan(const std::vector<double>& v, const double c)
{
    return std::ranges::all_of(v, [&c](const double& e) { return e <= c; });
}

static bool isGreaterThan(const std::vector<double>& v, const double c)
{
    return std::ranges::all_of(v, [&c](const double& e) { return e >= c; });
}

static void checkInputCorrectness(const std::vector<double>& G,
                                  const std::vector<double>& H,
                                  const std::vector<double>& D,
                                  const std::vector<double>& levels,
                                  const std::vector<double>& P_max,
                                  const std::vector<double>& P_min,
                                  double initial_level,
                                  double capacity,
                                  const std::vector<double>& inflows,
                                  const std::vector<double>& overflow,
                                  const std::vector<double>& pump,
                                  const std::vector<double>& S,
                                  const std::vector<double>& DTG_MRG)
{
    std::string msg_prefix = "Remix hydro input : ";

    // Initial level smaller than capacity
    if (initial_level > capacity)
    {
        throw std::invalid_argument(msg_prefix + "initial level > reservoir capacity");
    }
    // Arrays sizes must be identical
    std::vector<size_t> sizes = {G.size(),
                                 H.size(),
                                 D.size(),
                                 levels.size(),
                                 P_max.size(),
                                 P_min.size(),
                                 inflows.size(),
                                 overflow.size(),
                                 pump.size(),
                                 S.size(),
                                 DTG_MRG.size()};

    if (std::ranges::adjacent_find(sizes, std::not_equal_to()) != sizes.end())
    {
        throw std::invalid_argument(msg_prefix + "arrays of different sizes");
    }

    // Arrays are of size 0
    if (!G.size())
    {
        throw std::invalid_argument(msg_prefix + "all arrays of sizes 0");
    }

    // Hydro production < Pmax
    if (!isLessThan(H, P_max))
    {
        throw std::invalid_argument(msg_prefix + "H not smaller than Pmax everywhere");
    }
    // Hydro production > Pmin
    if (!isLessThan(P_min, H))
    {
        throw std::invalid_argument(msg_prefix + "H not greater than Pmin everywhere");
    }

    if (!isLessThan(levels, capacity) || !isGreaterThan(levels, 0.))
    {
        throw std::invalid_argument(msg_prefix
                                    + "levels computed from input don't respect reservoir bounds");
    }
}

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
                                 const std::vector<double>& inflows,
                                 const std::vector<double>& overflow,
                                 const std::vector<double>& pump,
                                 const std::vector<double>& S,
                                 const std::vector<double>& DTG_MRG)
{
    std::vector<double> levels(G.size());
    if (levels.size())
    {
        levels[0] = initial_level + inflows[0] - overflow[0] + pump[0] - H[0];
        for (size_t i = 1; i < levels.size(); ++i)
        {
            levels[i] = levels[i - 1] + inflows[i] - overflow[i] + pump[i] - H[i];
        }
    }

    checkInputCorrectness(G,
                          H,
                          D,
                          levels,
                          P_max,
                          P_min,
                          initial_level,
                          capa,
                          inflows,
                          overflow,
                          pump,
                          S,
                          DTG_MRG);

    std::vector<double> new_H = H;
    std::vector<double> new_D = D;

    int loop = 1000;
    double eps = 1e-3;
    double top = *std::max_element(G.begin(), G.end()) + *std::max_element(H.begin(), H.end())
                 + *std::max_element(D.begin(), D.end()) + 1;

    std::vector<bool> filter_hours_remix(G.size(), false);
    for (unsigned int h = 0; h < filter_hours_remix.size(); h++)
    {
        if (S[h] + DTG_MRG[h] == 0. && H[h] + D[h] > 0.)
        {
            filter_hours_remix[h] = true;
        }
    }

    std::vector<double> G_plus_H(G.size());
    std::transform(G.begin(), G.end(), new_H.begin(), G_plus_H.begin(), std::plus<>());

    while (loop-- > 0)
    {
        std::vector<int> tried_creux(G.size(), 0);
        double delta = 0;

        while (true)
        {
            int idx_creux = find_min_index(G_plus_H,
                                           new_D,
                                           new_H,
                                           tried_creux,
                                           P_max,
                                           filter_hours_remix,
                                           top);
            if (idx_creux == -1)
            {
                break;
            }

            std::vector<int> tried_pic(G.size(), 0);
            while (true)
            {
                int idx_pic = find_max_index(G_plus_H,
                                             new_H,
                                             tried_pic,
                                             P_min,
                                             filter_hours_remix,
                                             G_plus_H[idx_creux],
                                             eps);
                if (idx_pic == -1)
                {
                    break;
                }

                std::vector<double> intermediate_level(levels.begin()
                                                         + std::min(idx_creux, idx_pic),
                                                       levels.begin()
                                                         + std::max(idx_creux, idx_pic));
                double max_pic, max_creux;
                if (idx_creux < idx_pic)
                {
                    max_pic = capa;
                    max_creux = *std::min_element(intermediate_level.begin(),
                                                  intermediate_level.end());
                }
                else
                {
                    max_pic = capa
                              - *std::max_element(intermediate_level.begin(),
                                                  intermediate_level.end());
                    max_creux = capa;
                }

                max_pic = std::min(new_H[idx_pic] - P_min[idx_pic], max_pic);
                max_creux = std::min(
                  {P_max[idx_creux] - new_H[idx_creux], new_D[idx_creux], max_creux});

                double dif_pic_creux = std::max(G_plus_H[idx_pic] - G_plus_H[idx_creux], 0.);

                delta = std::max(std::min({max_pic, max_creux, dif_pic_creux / 2.}), 0.);

                if (delta > 0)
                {
                    new_H[idx_pic] -= delta;
                    new_H[idx_creux] += delta;
                    new_D[idx_pic] = H[idx_pic] + D[idx_pic] - new_H[idx_pic];
                    new_D[idx_creux] = H[idx_creux] + D[idx_creux] - new_H[idx_creux];
                    break;
                }
                else
                {
                    tried_pic[idx_pic] = 1;
                }
            }

            if (delta > 0)
            {
                break;
            }
            tried_creux[idx_creux] = 1;
        }

        if (delta == 0)
        {
            break;
        }

        std::transform(G.begin(), G.end(), new_H.begin(), G_plus_H.begin(), std::plus<>());
        levels[0] = initial_level + inflows[0] - overflow[0] + pump[0] - new_H[0];
        for (size_t i = 1; i < levels.size(); ++i)
        {
            levels[i] = levels[i - 1] + inflows[i] - overflow[i] + pump[i] - new_H[i];
        }
    }
    return {new_H, new_D, levels};
}

} // End namespace Antares::Solver::Simulation

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
                   double top)
{
    double min_val = top;
    int min_idx = -1;
    for (int i = 0; i < G_plus_H.size(); ++i)
    {
        if (new_D[i] > 0 && new_H[i] < P_max[i] && tried_creux[i] == 0)
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
                   double ref_value,
                   double eps)
{
    double max_val = 0;
    int max_idx = -1;
    for (int i = 0; i < G_plus_H.size(); ++i)
    {
        if (new_H[i] > P_min[i] && G_plus_H[i] >= ref_value + eps && tried_pic[i] == 0)
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

static void checkInputCorrectness(const std::vector<double>& G,
                                  const std::vector<double>& H,
                                  const std::vector<double>& D,
                                  const std::vector<double>& P_max,
                                  const std::vector<double>& P_min,
                                  double initial_level,
                                  double capa,
                                  const std::vector<double>& inflows)
{
    std::string msg_prefix = "Remix hydro input : ";

    // Initial level smaller than capacity
    if (initial_level > capa)
    {
        throw std::invalid_argument(msg_prefix + "initial level > reservoir capacity");
    }
    // Arrays sizes must be identical
    std::vector<size_t> sizes = {G.size(),
                                 H.size(),
                                 D.size(),
                                 P_max.size(),
                                 P_min.size(),
                                 inflows.size()};
    if (std::ranges::adjacent_find(sizes, std::not_equal_to()) != sizes.end())
    {
        throw std::invalid_argument(msg_prefix + "arrays of different sizes");
    }

    // Arrays are of size 0
    if (!G.size())
    {
        throw std::invalid_argument(msg_prefix + "all arrays of sizes 0");
    }
}

std::pair<std::vector<double>, std::vector<double>> new_remix_hydro(
  const std::vector<double>& G,
  const std::vector<double>& H,
  const std::vector<double>& D,
  const std::vector<double>& P_max,
  const std::vector<double>& P_min,
  double initial_level,
  double capa,
  const std::vector<double>& inflows)
{
    checkInputCorrectness(G, H, D, P_max, P_min, initial_level, capa, inflows);

    std::vector<double> new_H = H;
    std::vector<double> new_D = D;

    int loop = 1000;
    double eps = 1e-2;
    double top = *std::max_element(G.begin(), G.end()) + *std::max_element(H.begin(), H.end())
                 + *std::max_element(D.begin(), D.end()) + 1;

    std::vector<double> G_plus_H(G.size());
    std::transform(G.begin(), G.end(), new_H.begin(), G_plus_H.begin(), std::plus<>());

    std::vector<double> level(G.size());
    level[0] = initial_level + inflows[0] - new_H[0];
    for (size_t i = 1; i < level.size(); ++i)
    {
        level[i] = level[i - 1] + inflows[i] - new_H[i];
    }

    while (loop-- > 0)
    {
        std::vector<int> tried_creux(G.size(), 0);
        double delta = 0;

        while (true)
        {
            int idx_creux = find_min_index(G_plus_H, new_D, new_H, tried_creux, P_max, top);
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
                                             G_plus_H[idx_creux],
                                             eps);
                if (idx_pic == -1)
                {
                    break;
                }

                std::vector<double> intermediate_level(level.begin() + std::min(idx_creux, idx_pic),
                                                       level.begin()
                                                         + std::max(idx_creux, idx_pic));

                double max_pic = std::min(new_H[idx_pic] - P_min[idx_pic],
                                          capa
                                            - *std::max_element(intermediate_level.begin(),
                                                                intermediate_level.end()));
                double max_creux = std::min(
                  {P_max[idx_creux] - new_H[idx_creux],
                   new_D[idx_creux],
                   *std::min_element(intermediate_level.begin(), intermediate_level.end())});
                double dif_pic_creux = std::max(G_plus_H[idx_pic] - G_plus_H[idx_creux], 0.0);

                delta = std::max(std::min({max_pic, max_creux, dif_pic_creux / 2.0}), 0.0);

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
        level[0] = initial_level + inflows[0] - new_H[0];
        for (size_t i = 1; i < level.size(); ++i)
        {
            level[i] = level[i - 1] + inflows[i] - new_H[i];
        }
    }

    return {new_H, new_D};
}

} // End namespace Antares::Solver::Simulation

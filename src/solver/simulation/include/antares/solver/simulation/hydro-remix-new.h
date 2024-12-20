
#pragma once

#include <vector>

namespace Antares::Solver::Simulation
{

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
                                 double init_level,
                                 double capacity,
                                 const std::vector<double>& inflow,
                                 const std::vector<double>& overflow,
                                 const std::vector<double>& pump,
                                 const std::vector<double>& S,
                                 const std::vector<double>& DTG_MRG);

} // namespace Antares::Solver::Simulation

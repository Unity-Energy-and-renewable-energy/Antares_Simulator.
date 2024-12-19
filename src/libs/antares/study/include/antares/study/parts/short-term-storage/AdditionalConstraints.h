/*
 * Copyright 2007-2024, RTE (https://www.rte-france.com)
 * See AUTHORS.txt
 * SPDX-License-Identifier: MPL-2.0
 * This file is part of Antares-Simulator,
 * Adequacy and Performance assessment for interconnected energy networks.
 *
 * Antares_Simulator is free software: you can redistribute it and/or modify
 * it under the terms of the Mozilla Public Licence 2.0 as published by
 * the Mozilla Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Antares_Simulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Mozilla Public Licence 2.0 for more details.
 *
 * You should have received a copy of the Mozilla Public Licence 2.0
 * along with Antares_Simulator. If not, see <https://opensource.org/license/mpl-2-0/>.
 */

#pragma once
#include <set>
#include <string>
#include <vector>

namespace Antares::Data::ShortTermStorage
{
struct SingleAdditionalConstraint
{
    std::set<int> hours;
    unsigned int globalIndex = 0;
    unsigned int localIndex = 0;

    bool isValidHoursRange() const;
};

struct AdditionalConstraints
{
    std::string name;
    std::string cluster_id;
    std::string variable;
    std::string operatorType;
    // TODO a lot unused entries
    //std::array<double, HOURS_PER_YEAR> rhs = {};
    std::vector<double> rhs = {};

    std::vector<SingleAdditionalConstraint> constraints = {};

    struct ValidateResult
    {
        bool ok;
        std::string error_msg;
    };

    ValidateResult validate() const;

private:
    bool isValidVariable() const;
    bool isValidOperatorType() const;

    bool isValidHours() const;
};
} // namespace Antares::Data::ShortTermStorage

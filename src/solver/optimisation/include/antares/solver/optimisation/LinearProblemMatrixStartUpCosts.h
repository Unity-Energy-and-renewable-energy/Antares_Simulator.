/*
** Copyright 2007-2024, RTE (https://www.rte-france.com)
** See AUTHORS.txt
** SPDX-License-Identifier: MPL-2.0
** This file is part of Antares-Simulator,
** Adequacy and Performance assessment for interconnected energy networks.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the Mozilla Public Licence 2.0 as published by
** the Mozilla Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** Mozilla Public Licence 2.0 for more details.
**
** You should have received a copy of the Mozilla Public Licence 2.0
** along with Antares_Simulator. If not, see <https://opensource.org/license/mpl-2-0/>.
*/
#pragma once
#include <antares/study/study.h>
#include "antares/solver/optimisation/opt_structure_probleme_a_resoudre.h"
#include "antares/solver/simulation/sim_structure_probleme_economique.h"

#include "ProblemMatrixEssential.h"
#include "constraints/ConsistenceNumberOfDispatchableUnitsGroup.h"
#include "constraints/ConstraintGroup.h"
#include "constraints/MinDownTimeGroup.h"
#include "constraints/NbDispUnitsMinBoundSinceMinUpTimeGroup.h"
#include "constraints/NbUnitsOutageLessThanNbUnitsStopGroup.h"
#include "constraints/PMinMaxDispatchableGenerationGroup.h"

using namespace Antares::Data;

class LinearProblemMatrixStartUpCosts: public ProblemMatrixEssential
{
public:
    explicit LinearProblemMatrixStartUpCosts(PROBLEME_HEBDO* problemeHebdo,
                                             bool Simulation,
                                             ConstraintBuilder& builder);

private:
    bool simulation_ = false;
    PMinMaxDispatchableGenerationGroup pMinMaxDispatchableGenerationGroup_;
    ConsistenceNumberOfDispatchableUnitsGroup consistenceNumberOfDispatchableUnitsGroup_;
    NbUnitsOutageLessThanNbUnitsStopGroup nbUnitsOutageLessThanNbUnitsStopGroup_;
    NbDispUnitsMinBoundSinceMinUpTimeGroup nbDispUnitsMinBoundSinceMinUpTimeGroup_;
    MinDownTimeGroup minDownTimeGroup_;
};

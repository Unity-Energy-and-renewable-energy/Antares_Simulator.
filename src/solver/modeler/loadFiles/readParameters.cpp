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

#include <yaml-cpp/yaml.h>

#include <antares/io/file.h>
#include <antares/logs/logs.h>
#include "antares/solver/modeler/loadFiles/loadFiles.h"
#include "antares/solver/modeler/parameters/parseModelerParameters.h"

namespace fs = std::filesystem;

namespace Antares::Solver::LoadFiles
{

ModelerParameters loadParameters(const fs::path& studyPath)
{
    try
    {
        const std::string paramStr = IO::readFile(studyPath / "parameters.yml");
        return parseModelerParameters(paramStr);
    }
    catch (const YAML::Exception& e)
    {
        logs.error() << "Error while parsing the yaml parameters file";
        if (!e.mark.is_null())
        {
            logs.error() << "Line " << e.mark.line << " column " << e.mark.column;
        }
        logs.error() << e.what();

        throw std::runtime_error(e.what());
    }
    catch (const std::runtime_error& e)
    {
        logs.error() << "Error while parsing the yaml parameters file:";
        logs.error() << e.what();

        throw std::runtime_error(e.what());
    }
}

} // namespace Antares::Solver::LoadFiles

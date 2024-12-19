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
#define WIN32_LEAN_AND_MEAN

#include <fstream>
#define BOOST_TEST_MODULE load modeler files

#include <boost/test/unit_test.hpp>

#include <antares/solver/modeler/loadFiles/loadFiles.h>

#include "files-system.h"

BOOST_AUTO_TEST_SUITE(read_modeler_parameters)

namespace fs = std::filesystem;

struct FixtureLoadFile
{
    fs::path studyPath;
    fs::path inputPath;
    fs::path libraryDirPath;

    FixtureLoadFile()
    {
        studyPath = CREATE_TMP_DIR_BASED_ON_TEST_NAME();
        inputPath = createFolder(studyPath, "input");
        libraryDirPath = createFolder(inputPath, "model-libraries");
    }

    ~FixtureLoadFile()
    {
        fs::remove_all(studyPath);
    }
};

BOOST_FIXTURE_TEST_CASE(read_one_lib_file, FixtureLoadFile)
{
    std::ofstream libStream(libraryDirPath / "simple.yml");
    libStream << R"(
        library:
            id: lib_id
            description: lib_description
            port-types: []
            models: []
    )";
    libStream.close();

    auto libraries = Antares::Solver::LoadFiles::loadLibraries(studyPath);
    /* BOOST_CHECK_EQUAL(libraries[0].Id(), "lib_id"); */
}

BOOST_AUTO_TEST_SUITE_END()

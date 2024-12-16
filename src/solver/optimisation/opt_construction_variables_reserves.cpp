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

#include <spx_constantes_externes.h>

#include "antares/solver/optimisation/opt_fonctions.h"
#include "antares/solver/optimisation/opt_rename_problem.h"
#include "antares/solver/optimisation/opt_structure_probleme_a_resoudre.h"

void OPT_ConstruireLaListeDesVariablesOptimiseesDuProblemeLineaireReserves(
  PROBLEME_HEBDO* problemeHebdo,
  bool Simulation)
{
    VariableNamer variableNamer(problemeHebdo->ProblemeAResoudre->NomDesVariables);
    int NombreDePasDeTempsPourUneOptimisation = problemeHebdo
                                                  ->NombreDePasDeTempsPourUneOptimisation;

    struct ReserveVariablesInitializer
    {
        PROBLEME_HEBDO* problemeHebdo;
        bool Simulation;
        const std::unique_ptr<PROBLEME_ANTARES_A_RESOUDRE>& ProblemeAResoudre;
        int& NombreDeVariables;
        VariableNamer& variableNamer;
        VariableManagement::VariableManager variableManager;

        ReserveVariablesInitializer(PROBLEME_HEBDO* hebdo, bool sim, VariableNamer& namer):
            problemeHebdo(hebdo),
            Simulation(sim),
            ProblemeAResoudre(hebdo->ProblemeAResoudre),
            NombreDeVariables(ProblemeAResoudre->NombreDeVariables),
            variableNamer(namer),
            variableManager(VariableManagerFromProblemHebdo(hebdo))
        {
        }

        // Init variables for a reserve
        void initReserve(int pdt, const int reserveIndex, const std::string& reserveName)
        {
            if (Simulation)
            {
                NombreDeVariables += 2;
            }
            else
            {
                // For Unsatisfied Reserves
                variableManager.InternalUnsatisfiedReserve(reserveIndex, pdt) = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.InternalUnsatisfiedReserve(NombreDeVariables, reserveName);
                NombreDeVariables++;

                // For Excess Reserves
                variableManager.InternalExcessReserve(reserveIndex, pdt) = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.InternalExcessReserve(NombreDeVariables, reserveName);
                NombreDeVariables++;
            }
        }

        // Init variables for a Thermal cluster participation to a reserve up
        void initThermalReserveUpParticipation(
          int pdt,
          const RESERVE_PARTICIPATION_THERMAL& clusterReserveParticipation,
          const std::string& reserveName)
        {
            const auto& clusterName = clusterReserveParticipation.clusterName;
            if (Simulation)
            {
                NombreDeVariables += 4;
            }
            else
            {
                // For running units in cluster
                variableManager.RunningThermalClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfRunningUnitsToReserve(NombreDeVariables,
                                                                   clusterName,
                                                                   reserveName);
                NombreDeVariables++;

                // For off units in cluster
                variableManager.OffThermalClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfOffUnitsToReserve(NombreDeVariables,
                                                               clusterName,
                                                               reserveName);
                NombreDeVariables++;

                variableManager.NumberOfOffUnitsParticipatingToReserve(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                ProblemeAResoudre->VariablesEntieres[NombreDeVariables]
                  = problemeHebdo->OptimisationAvecVariablesEntieres;
                variableNamer.NumberOfOffUnitsParticipatingToReserve(NombreDeVariables,
                                                                     clusterName,
                                                                     reserveName);
                NombreDeVariables++;

                // For all units in cluster
                variableManager.ThermalClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ThermalClusterReserveParticipation(NombreDeVariables,
                                                                 clusterName,
                                                                 reserveName);
                NombreDeVariables++;
            }
        }

        // Init variables for a Thermal cluster participation to a reserve down
        void initThermalReserveDownParticipation(
          int pdt,
          const RESERVE_PARTICIPATION_THERMAL& clusterReserveParticipation,
          const std::string& reserveName)
        {
            const auto& clusterName = clusterReserveParticipation.clusterName;
            if (Simulation)
            {
                NombreDeVariables += 2;
            }
            else
            {
                // For running units in cluster
                variableManager.RunningThermalClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfRunningUnitsToReserve(NombreDeVariables,
                                                                   clusterName,
                                                                   reserveName);
                NombreDeVariables++;

                // For all units in cluster (off units can not participate to down reserves)
                variableManager.ThermalClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ThermalClusterReserveParticipation(NombreDeVariables,
                                                                 clusterName,
                                                                 reserveName);
                NombreDeVariables++;
            }
        }

        // Init variables for a ShortTerm cluster participation to a reserve up
        void initSTStorageReserveUpParticipation(
          int pdt,
          const RESERVE_PARTICIPATION_STSTORAGE& clusterReserveParticipation,
          const std::string& reserveName)
        {
            const auto& clusterName = clusterReserveParticipation.clusterName;
            if (Simulation)
            {
                NombreDeVariables += 3;
            }
            else
            {
                // For Turbining participation to the reserves
                variableManager.STStorageTurbiningClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfSTStorageTurbiningToReserve(NombreDeVariables,
                                                                         clusterName,
                                                                         reserveName);
                NombreDeVariables++;

                // For Pumping participation to the reserves
                variableManager.STStoragePumpingClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfSTStoragePumpingToReserve(NombreDeVariables,
                                                                       clusterName,
                                                                       reserveName);
                NombreDeVariables++;

                // For Short Term Storage participation to the up reserves
                variableManager.STStorageClusterReserveUpParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfSTStorageToUpReserve(NombreDeVariables,
                                                                  clusterName,
                                                                  reserveName);
                NombreDeVariables++;
            }
        }

        // Init variables for a ShortTerm cluster participation to a reserve down
        void initSTStorageReserveDownParticipation(
          int pdt,
          const RESERVE_PARTICIPATION_STSTORAGE& clusterReserveParticipation,
          const std::string& reserveName)
        {
            const auto& clusterName = clusterReserveParticipation.clusterName;
            if (Simulation)
            {
                NombreDeVariables += 3;
            }
            else
            {
                // For Turbining participation to the reserves
                variableManager.STStorageTurbiningClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfSTStorageTurbiningToReserve(NombreDeVariables,
                                                                         clusterName,
                                                                         reserveName);
                NombreDeVariables++;

                // For Pumping participation to the reserves
                variableManager.STStoragePumpingClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfSTStoragePumpingToReserve(NombreDeVariables,
                                                                       clusterName,
                                                                       reserveName);
                NombreDeVariables++;

                // For Short Term Storage participation to the Down reserves
                variableManager.STStorageClusterReserveDownParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfSTStorageToDownReserve(NombreDeVariables,
                                                                    clusterName,
                                                                    reserveName);
                NombreDeVariables++;
            }
        }

        // Init variables for a LongTerm cluster participation to a reserve up
        void initLTStorageReserveUpParticipation(
          int pdt,
          const RESERVE_PARTICIPATION_LTSTORAGE& clusterReserveParticipation,
          const std::string& reserveName)
        {
            const auto& clusterName = clusterReserveParticipation.clusterName;
            if (Simulation)
            {
                NombreDeVariables += 3;
            }
            else
            {
                // For Turbining participation to the reserves
                variableManager.LTStorageTurbiningClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfLTStorageTurbiningToReserve(NombreDeVariables,
                                                                         clusterName,
                                                                         reserveName);
                NombreDeVariables++;

                // For Pumping participation to the reserves
                variableManager.LTStoragePumpingClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfLTStoragePumpingToReserve(NombreDeVariables,
                                                                       clusterName,
                                                                       reserveName);
                NombreDeVariables++;

                // For Long Term Storage participation to the up reserves
                variableManager.LTStorageClusterReserveUpParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfLTStorageToUpReserve(NombreDeVariables,
                                                                  clusterName,
                                                                  reserveName);
                NombreDeVariables++;
            }
        }

        // Init variables for a LongTerm cluster participation to a reserve down
        void initLTStorageReserveDownParticipation(
          int pdt,
          const RESERVE_PARTICIPATION_LTSTORAGE& clusterReserveParticipation,
          const std::string& reserveName)
        {
            const auto& clusterName = clusterReserveParticipation.clusterName;
            if (Simulation)
            {
                NombreDeVariables += 3;
            }
            else
            {
                // For Turbining participation to the reserves
                variableManager.LTStorageTurbiningClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfLTStorageTurbiningToReserve(NombreDeVariables,
                                                                         clusterName,
                                                                         reserveName);
                NombreDeVariables++;

                // For Pumping participation to the reserves
                variableManager.LTStoragePumpingClusterReserveParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfLTStoragePumpingToReserve(NombreDeVariables,
                                                                       clusterName,
                                                                       reserveName);
                NombreDeVariables++;

                // For Long Term Storage participation to the Down reserves
                variableManager.LTStorageClusterReserveDownParticipation(
                  clusterReserveParticipation.globalIndexClusterParticipation,
                  pdt)
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                variableNamer.ParticipationOfLTStorageToDownReserve(NombreDeVariables,
                                                                    clusterName,
                                                                    reserveName);
                NombreDeVariables++;
            }
        }
    };

    ReserveVariablesInitializer reserveVariablesInitializer(problemeHebdo,
                                                            Simulation,
                                                            variableNamer);
    for (int pdt = 0; pdt < NombreDePasDeTempsPourUneOptimisation; pdt++)
    {
        variableNamer.UpdateTimeStep(problemeHebdo->weekInTheYear * 168 + pdt);
        auto& CorrespondanceVarNativesVarOptim = problemeHebdo
                                                   ->CorrespondanceVarNativesVarOptim[pdt];

        for (uint32_t pays = 0; pays < problemeHebdo->NombreDePays; pays++)
        {
            variableNamer.UpdateArea(problemeHebdo->NomsDesPays[pays]);
            auto areaReserves = problemeHebdo->allReserves[pays];
            int index = 0;
            int reserveIndex = 0;

            // For Up Reserves
            for (auto& areaReserveUp: areaReserves.areaCapacityReservationsUp)
            {
                reserveVariablesInitializer.initReserve(pdt,
                                                        areaReserveUp.globalReserveIndex,
                                                        areaReserveUp.reserveName);

                // Thermal Clusters
                for (auto& [clusterId, clusterReserveParticipation]:
                     areaReserveUp.AllThermalReservesParticipation)
                {
                    reserveVariablesInitializer.initThermalReserveUpParticipation(
                      pdt,
                      clusterReserveParticipation,
                      areaReserveUp.reserveName);
                }

                // Short Term Storage Clusters
                for (auto& [clusterId, clusterReserveParticipation]:
                     areaReserveUp.AllSTStorageReservesParticipation)
                {
                    reserveVariablesInitializer.initSTStorageReserveUpParticipation(
                      pdt,
                      clusterReserveParticipation,
                      areaReserveUp.reserveName);
                }

                // Long Term Storage Clusters
                for (auto& clusterReserveParticipation:
                     areaReserveUp.AllLTStorageReservesParticipation)
                {
                    reserveVariablesInitializer.initLTStorageReserveUpParticipation(
                      pdt,
                      clusterReserveParticipation,
                      areaReserveUp.reserveName);
                }
            }

            // For Down Reserves
            for (auto& areaReserveDown: areaReserves.areaCapacityReservationsDown)
            {
                reserveVariablesInitializer.initReserve(pdt,
                                                        areaReserveDown.globalReserveIndex,
                                                        areaReserveDown.reserveName);

                // Thermal Clusters
                for (auto& [clusterId, clusterReserveParticipation]:
                     areaReserveDown.AllThermalReservesParticipation)
                {
                    reserveVariablesInitializer.initThermalReserveDownParticipation(
                      pdt,
                      clusterReserveParticipation,
                      areaReserveDown.reserveName);
                }

                // Short Term Storage Clusters
                for (auto& [clusterId, clusterReserveParticipation]:
                     areaReserveDown.AllSTStorageReservesParticipation)
                {
                    reserveVariablesInitializer.initSTStorageReserveDownParticipation(
                      pdt,
                      clusterReserveParticipation,
                      areaReserveDown.reserveName);
                }

                // Long Term Storage Clusters
                for (auto& clusterReserveParticipation:
                     areaReserveDown.AllLTStorageReservesParticipation)
                {
                    reserveVariablesInitializer.initLTStorageReserveDownParticipation(
                      pdt,
                      clusterReserveParticipation,
                      areaReserveDown.reserveName);
                }
            }
        }
    }
}

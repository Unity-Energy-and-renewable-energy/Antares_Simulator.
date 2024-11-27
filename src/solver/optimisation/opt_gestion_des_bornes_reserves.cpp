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

#include "antares/solver/optimisation/opt_fonctions.h"
#include "antares/solver/optimisation/opt_structure_probleme_a_resoudre.h"
#include "antares/solver/simulation/sim_structure_donnees.h"
#include "antares/solver/simulation/sim_structure_probleme_economique.h"
#include "antares/solver/simulation/simulation.h"

#include "variables/VariableManagement.h"
#include "variables/VariableManagerUtils.h"

using namespace Yuni;

void OPT_InitialiserLesBornesDesVariablesDuProblemeLineaireReservesThermiques(
  PROBLEME_HEBDO* problemeHebdo,
  const int PremierPdtDeLIntervalle,
  const int DernierPdtDeLIntervalle)
{
    struct ReserveVariablesBoundsSetter
    {
        PROBLEME_HEBDO* problemeHebdo;
        const std::unique_ptr<PROBLEME_ANTARES_A_RESOUDRE>& ProblemeAResoudre;
        std::vector<double>& Xmin;
        std::vector<double>& Xmax;
        std::vector<double*>& AdresseOuPlacerLaValeurDesVariablesOptimisees;
        int pdtJour, pdtHebdo, pays;

        ReserveVariablesBoundsSetter(PROBLEME_HEBDO* hebdo):
            problemeHebdo(hebdo),
            ProblemeAResoudre(hebdo->ProblemeAResoudre),
            Xmin(ProblemeAResoudre->Xmin),
            Xmax(ProblemeAResoudre->Xmax),
            AdresseOuPlacerLaValeurDesVariablesOptimisees(
              ProblemeAResoudre->AdresseOuPlacerLaValeurDesVariablesOptimisees),
            pdtJour(0),
            pdtHebdo(0),
            pays(0)
        {
        }

        void setPdtJour(int pdt)
        {
            pdtJour = pdt;
        }

        void setPdtHebdo(int pdt)
        {
            pdtHebdo = pdt;
        }

        void setPays(int p)
        {
            pays = p;
        }

        // Set variables bounds for a reserve
        void setReserveBounds(int reserveIdInArea, int reserveId)
        {
            const auto& CorrespondanceVarNativesVarOptim = problemeHebdo
                                                             ->CorrespondanceVarNativesVarOptim
                                                               [pdtJour];
            int var = CorrespondanceVarNativesVarOptim.internalUnsatisfiedReserveIndex[reserveId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            double* adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                            .Reserves[pdtHebdo]
                                            .ValeursHorairesInternalUnsatisfied[reserveIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;

            var = CorrespondanceVarNativesVarOptim.internalExcessReserveIndex[reserveId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                    .Reserves[pdtHebdo]
                                    .ValeursHorairesInternalExcessReserve[reserveIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;
        }

        // Set variables bounds for a Thermal cluster participation to a reserve up
        void setThermalReserveUpParticipationBounds(int clusterParticipationIdInArea,
                                                    int clusterParticipationId,
                                                    int clusterIdInArea,
                                                    int clusterId)
        {
            const auto& CorrespondanceVarNativesVarOptim = problemeHebdo
                                                             ->CorrespondanceVarNativesVarOptim
                                                               [pdtJour];
            int var = CorrespondanceVarNativesVarOptim
                        .runningThermalClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            double* adresseDuResultat = &(
              problemeHebdo->ResultatsHoraires[pays]
                .ProductionThermique[pdtHebdo]
                .ParticipationReservesDuPalierOn[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;

            var = CorrespondanceVarNativesVarOptim
                    .offThermalClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(
              problemeHebdo->ResultatsHoraires[pays]
                .ProductionThermique[pdtHebdo]
                .ParticipationReservesDuPalierOff[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;

            var = CorrespondanceVarNativesVarOptim
                    .nbOffGroupUnitsParticipatingToReservesInThermalClusterIndex
                      [clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = problemeHebdo->PaliersThermiquesDuPays[pays]
                          .TailleUnitaireDUnGroupeDuPalierThermique[clusterId];
            adresseDuResultat = &(
              problemeHebdo->ResultatsHoraires[pays]
                .ProductionThermique[pdtHebdo]
                .NombreDeGroupesEteintDuPalierQuiParticipentAuxReserves[clusterIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;

            var = CorrespondanceVarNativesVarOptim
                    .thermalClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                    .ProductionThermique[pdtHebdo]
                                    .ParticipationReservesDuPalier[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;
        }

        // Set variables bounds for a Thermal cluster participation to a reserve down
        void setThermalReserveDownParticipationBounds(int clusterParticipationIdInArea,
                                                      int clusterParticipationId)
        {
            const auto& CorrespondanceVarNativesVarOptim = problemeHebdo
                                                             ->CorrespondanceVarNativesVarOptim
                                                               [pdtJour];
            int var = CorrespondanceVarNativesVarOptim
                        .runningThermalClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            double* adresseDuResultat = &(
              problemeHebdo->ResultatsHoraires[pays]
                .ProductionThermique[pdtHebdo]
                .ParticipationReservesDuPalierOn[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;

            var = CorrespondanceVarNativesVarOptim
                    .thermalClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                    .ProductionThermique[pdtHebdo]
                                    .ParticipationReservesDuPalier[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;
        }

        // Set variables bounds for a ShortTerm cluster participation to a reserve up
        void setSTStorageReserveUpParticipationBounds(int clusterParticipationIdInArea,
                                                      int clusterParticipationId)
        {
            const auto& CorrespondanceVarNativesVarOptim = problemeHebdo
                                                             ->CorrespondanceVarNativesVarOptim
                                                               [pdtJour];
            int var = CorrespondanceVarNativesVarOptim
                        .STStorageTurbiningClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            double* adresseDuResultat = &(
              problemeHebdo->ResultatsHoraires[pays]
                .ShortTermStorage[pdtHebdo]
                .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = nullptr;

            var = CorrespondanceVarNativesVarOptim
                    .STStoragePumpingClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                    .ShortTermStorage[pdtHebdo]
                                    .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = nullptr;

            var = CorrespondanceVarNativesVarOptim
                    .STStorageClusterReserveUpParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                    .ShortTermStorage[pdtHebdo]
                                    .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;
        }

        // Set variables bounds for a ShortTerm cluster participation to a reserve down
        void setSTStorageReserveDownParticipationBounds(int clusterParticipationIdInArea,
                                                        int clusterParticipationId)
        {
            const auto& CorrespondanceVarNativesVarOptim = problemeHebdo
                                                             ->CorrespondanceVarNativesVarOptim
                                                               [pdtJour];
            int var = CorrespondanceVarNativesVarOptim
                        .STStorageTurbiningClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            double* adresseDuResultat = &(
              problemeHebdo->ResultatsHoraires[pays]
                .ShortTermStorage[pdtHebdo]
                .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = nullptr;

            var = CorrespondanceVarNativesVarOptim
                    .STStoragePumpingClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                    .ShortTermStorage[pdtHebdo]
                                    .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = nullptr;

            var = CorrespondanceVarNativesVarOptim
                    .STStorageClusterReserveDownParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                    .ShortTermStorage[pdtHebdo]
                                    .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;
        }

        // Set variables bounds for a LongTerm cluster participation to a reserve up
        void setLTStorageReserveUpParticipationBounds(int clusterParticipationIdInArea,
                                                      int clusterParticipationId)
        {
            const auto& CorrespondanceVarNativesVarOptim = problemeHebdo
                                                             ->CorrespondanceVarNativesVarOptim
                                                               [pdtJour];
            int var = CorrespondanceVarNativesVarOptim
                        .LTStorageTurbiningClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            double* adresseDuResultat = &(
              problemeHebdo->ResultatsHoraires[pays]
                .HydroUsage[pdtHebdo]
                .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = nullptr;

            var = CorrespondanceVarNativesVarOptim
                    .LTStoragePumpingClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                    .HydroUsage[pdtHebdo]
                                    .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = nullptr;

            var = CorrespondanceVarNativesVarOptim
                    .LTStorageClusterReserveUpParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                    .HydroUsage[pdtHebdo]
                                    .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;
        }

        // Set variables bounds for a LongTerm cluster participation to a reserve down
        void setLTStorageReserveDownParticipationBounds(int clusterParticipationIdInArea,
                                                        int clusterParticipationId)
        {
            const auto& CorrespondanceVarNativesVarOptim = problemeHebdo
                                                             ->CorrespondanceVarNativesVarOptim
                                                               [pdtJour];
            int var = CorrespondanceVarNativesVarOptim
                        .LTStorageTurbiningClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            double* adresseDuResultat = &(
              problemeHebdo->ResultatsHoraires[pays]
                .HydroUsage[pdtHebdo]
                .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = nullptr;

            var = CorrespondanceVarNativesVarOptim
                    .LTStoragePumpingClusterReserveParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                    .HydroUsage[pdtHebdo]
                                    .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = nullptr;

            var = CorrespondanceVarNativesVarOptim
                    .LTStorageClusterReserveDownParticipationIndex[clusterParticipationId];
            Xmin[var] = 0;
            Xmax[var] = LINFINI_ANTARES;
            adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                    .HydroUsage[pdtHebdo]
                                    .reserveParticipationOfCluster[clusterParticipationIdInArea]);
            AdresseOuPlacerLaValeurDesVariablesOptimisees[var] = adresseDuResultat;
        }
    };

    ReserveVariablesBoundsSetter reserveVariablesBoundsSetter(problemeHebdo);

    for (int pdtHebdo = PremierPdtDeLIntervalle, pdtJour = 0; pdtHebdo < DernierPdtDeLIntervalle;
         pdtHebdo++, pdtJour++)
    {
        reserveVariablesBoundsSetter.setPdtJour(pdtJour);
        reserveVariablesBoundsSetter.setPdtHebdo(pdtHebdo);

        for (uint32_t pays = 0; pays < problemeHebdo->NombreDePays; pays++)
        {
            reserveVariablesBoundsSetter.setPays(pays);
            const auto& areaReserves = problemeHebdo->allReserves[pays];

            for (const auto& areaReserveUp: areaReserves.areaCapacityReservationsUp)
            {
                reserveVariablesBoundsSetter.setReserveBounds(areaReserveUp.areaReserveIndex,
                                                              areaReserveUp.globalReserveIndex);

                // Thermal Cluster
                for (const auto& [clusterId, clusterReserveParticipation]:
                     areaReserveUp.AllThermalReservesParticipation)
                {
                    reserveVariablesBoundsSetter.setThermalReserveUpParticipationBounds(
                      clusterReserveParticipation.areaIndexClusterParticipation,
                      clusterReserveParticipation.globalIndexClusterParticipation,
                      clusterReserveParticipation.clusterIdInArea,
                      clusterId);
                }

                // Short Term Storage Cluster
                for (const auto& [clusterId, clusterReserveParticipation]:
                     areaReserveUp.AllSTStorageReservesParticipation)
                {
                    reserveVariablesBoundsSetter.setSTStorageReserveUpParticipationBounds(
                      clusterReserveParticipation.areaIndexClusterParticipation,
                      clusterReserveParticipation.globalIndexClusterParticipation);
                }

                // Long Term Storage Cluster
                for (const auto& clusterReserveParticipation:
                     areaReserveUp.AllLTStorageReservesParticipation)
                {
                    reserveVariablesBoundsSetter.setLTStorageReserveUpParticipationBounds(
                      clusterReserveParticipation.areaIndexClusterParticipation,
                      clusterReserveParticipation.globalIndexClusterParticipation);
                }
            }
            for (const auto& areaReserveDown: areaReserves.areaCapacityReservationsDown)
            {
                reserveVariablesBoundsSetter.setReserveBounds(areaReserveDown.areaReserveIndex,
                                                              areaReserveDown.globalReserveIndex);

                // Thermal Clusters
                for (const auto& [clusterId, clusterReserveParticipation]:
                     areaReserveDown.AllThermalReservesParticipation)
                {
                    reserveVariablesBoundsSetter.setThermalReserveDownParticipationBounds(
                      clusterReserveParticipation.areaIndexClusterParticipation,
                      clusterReserveParticipation.globalIndexClusterParticipation);
                }

                // Short Term Storage Cluster
                for (const auto& [clusterId, clusterReserveParticipation]:
                     areaReserveDown.AllSTStorageReservesParticipation)
                {
                    reserveVariablesBoundsSetter.setSTStorageReserveDownParticipationBounds(
                      clusterReserveParticipation.areaIndexClusterParticipation,
                      clusterReserveParticipation.globalIndexClusterParticipation);
                }

                // Long Term Storage Cluster
                for (const auto& clusterReserveParticipation:
                     areaReserveDown.AllLTStorageReservesParticipation)
                {
                    reserveVariablesBoundsSetter.setLTStorageReserveDownParticipationBounds(
                      clusterReserveParticipation.areaIndexClusterParticipation,
                      clusterReserveParticipation.globalIndexClusterParticipation);
                }
            }
        }
    }
}

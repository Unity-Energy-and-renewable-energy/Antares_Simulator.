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

#include <antares/study/study.h>
#include "antares/solver/optimisation/opt_fonctions.h"
#include "antares/solver/optimisation/opt_structure_probleme_a_resoudre.h"
#include "antares/solver/simulation/sim_structure_donnees.h"

using namespace Antares;
using namespace Antares::Data;
using namespace Yuni;

void OPT_InitialiserLeSecondMembreDuProblemeLineaireReserves(PROBLEME_HEBDO* problemeHebdo,
                                                             int PremierPdtDeLIntervalle,
                                                             int DernierPdtDeLIntervalle)
{
    const auto& ProblemeAResoudre = problemeHebdo->ProblemeAResoudre;
    std::vector<double>& SecondMembre = ProblemeAResoudre->SecondMembre;

    std::vector<double*>& AdresseOuPlacerLaValeurDesCoutsMarginaux
      = ProblemeAResoudre->AdresseOuPlacerLaValeurDesCoutsMarginaux;

    struct ReserveVariablesRightSidesSetter
    {
        PROBLEME_HEBDO* problemeHebdo;
        const std::unique_ptr<PROBLEME_ANTARES_A_RESOUDRE>& ProblemeAResoudre;
        std::vector<double>& SecondMembre;
        std::vector<double*>& AdresseOuPlacerLaValeurDesCoutsMarginaux;
        int pdtJour, pdtHebdo, pdtGlobal, pays;

        ReserveVariablesRightSidesSetter(PROBLEME_HEBDO* hebdo):
            problemeHebdo(hebdo),
            ProblemeAResoudre(hebdo->ProblemeAResoudre),
            SecondMembre(ProblemeAResoudre->SecondMembre),
            AdresseOuPlacerLaValeurDesCoutsMarginaux(
              ProblemeAResoudre->AdresseOuPlacerLaValeurDesCoutsMarginaux),
            pdtJour(0),
            pdtHebdo(0),
            pdtGlobal(0),
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

        void setPdtGlobal(int pdt)
        {
            pdtGlobal = pdt;
        }

        void setPays(int p)
        {
            pays = p;
        }

        // Set the rigth sides of equations for a reserve
        void setReserveRightSides(const CAPACITY_RESERVATION& reserve)
        {
            const auto& CorrespondanceCntNativesCntOptim = problemeHebdo
                                                             ->CorrespondanceCntNativesCntOptim
                                                               [pdtJour];
            int cnt = CorrespondanceCntNativesCntOptim
                        .NumeroDeContrainteDesContraintesDeBesoinEnReserves[reserve
                                                                              .globalReserveIndex];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = reserve.need.at(pdtGlobal);
                double* adresseDuResultat = &(problemeHebdo->ResultatsHoraires[pays]
                                                .Reserves[pdtHebdo]
                                                .CoutsMarginauxHoraires[reserve.areaReserveIndex]);
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = adresseDuResultat;
            }
        }

        // Common setter for the thermal clusters
        void setThermalReserveParticipationRightSides(
          const RESERVE_PARTICIPATION_THERMAL& reserveParticipation)
        {
            const auto& CorrespondanceCntNativesCntOptim = problemeHebdo
                                                             ->CorrespondanceCntNativesCntOptim
                                                               [pdtJour];
            int cnt = CorrespondanceCntNativesCntOptim
                        .NumeroDeContrainteDesContraintesDePuissanceMinDuPalier[reserveParticipation
                                                                                  .clusterIdInArea];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = problemeHebdo->PaliersThermiquesDuPays[pays]
                                      .PuissanceDisponibleEtCout[reserveParticipation
                                                                   .clusterIdInArea]
                                      .PuissanceMinDuPalierThermiqueRef[pdtJour];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesDePuissanceMaxDuPalier[reserveParticipation
                                                                              .clusterIdInArea];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = problemeHebdo->PaliersThermiquesDuPays[pays]
                                      .PuissanceDisponibleEtCout[reserveParticipation
                                                                   .clusterIdInArea]
                                      .PuissanceDisponibleDuPalierThermiqueRef[pdtJour];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }

        // Set the rigth sides of equations for a Thermal cluster participation to a reserve up
        void setThermalReserveUpParticipationRightSides(
          const RESERVE_PARTICIPATION_THERMAL& reserveParticipation)
        {
            setThermalReserveParticipationRightSides(reserveParticipation);
            const auto& CorrespondanceCntNativesCntOptim = problemeHebdo
                                                             ->CorrespondanceCntNativesCntOptim
                                                               [pdtJour];
            int cnt = CorrespondanceCntNativesCntOptim
                        .nbOffGroupUnitsParticipatingToReservesInThermalClusterConstraintIndex
                          [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = problemeHebdo->PaliersThermiquesDuPays[pays]
                                      .PuissanceDisponibleEtCout[reserveParticipation
                                                                   .clusterIdInArea]
                                      .NombreMaxDeGroupesEnMarcheDuPalierThermique[pdtJour];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }

        // Set the rigth sides of equations for a Thermal cluster participation to a reserve down
        void setThermalReserveDownParticipationRightSides(
          const RESERVE_PARTICIPATION_THERMAL& reserveParticipation)
        {
            setThermalReserveParticipationRightSides(reserveParticipation);
        }

        // Common setter for the ShortTerm Storage clusters
        void setSTStorageReserveParticipationRightSides(
          const RESERVE_PARTICIPATION_STSTORAGE& reserveParticipation,
          const AREA_RESERVES_VECTOR& reserves,
          const CAPACITY_RESERVATION& reserve)
        {
            const auto& CorrespondanceCntNativesCntOptim = problemeHebdo
                                                             ->CorrespondanceCntNativesCntOptim
                                                               [pdtJour];
            int cnt
              = CorrespondanceCntNativesCntOptim
                  .NumeroDeContrainteDesContraintesSTStorageClusterTurbiningCapacityThreasholdsMax
                    [reserveParticipation.clusterId];

            auto& cluster = problemeHebdo->ShortTermStorage[pays][reserveParticipation.clusterId];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = cluster.series.get()->maxWithdrawalModulation[pdtGlobal]
                                    * cluster.withdrawalNominalCapacity;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesSTStorageClusterTurbiningCapacityThreasholdsMin
                      [reserveParticipation.clusterId];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = cluster.series.get()->lowerRuleCurve[pdtGlobal]
                                    * cluster.withdrawalNominalCapacity;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesSTStorageClusterPumpingCapacityThreasholds
                      [reserveParticipation.clusterId];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = cluster.series.get()->maxInjectionModulation[pdtGlobal]
                                    * cluster.injectionNominalCapacity;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
            double level_max = cluster.reservoirCapacity
                               * cluster.series->upperRuleCurve[pdtGlobal];
            double level_min = cluster.reservoirCapacity
                               * cluster.series->lowerRuleCurve[pdtGlobal];

            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesSTStockLevelReserveParticipationDown
                      [reserveParticipation.clusterId];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = level_max;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesSTStockLevelReserveParticipationUp
                      [reserveParticipation.clusterId];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = -level_min;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            cnt
              = CorrespondanceCntNativesCntOptim
                  .NumeroDeContrainteDesContraintesSTGlobalStockEnergyLevelReserveParticipationDown
                    [reserveParticipation.clusterId];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = reserves.maxGlobalActivationDurationDown
                                    * reserves.maxGlobalEnergyActivationRatioDown * level_max;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesSTGlobalStockEnergyLevelReserveParticipationUp
                      [reserveParticipation.clusterId];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = -reserves.maxGlobalActivationDurationUp
                                    * reserves.maxGlobalEnergyActivationRatioUp * level_min;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }

        // Set the rigth sides of equations for a ShortTerm cluster participation to a reserve up
        void setSTStorageReserveUpParticipationRightSides(
          const RESERVE_PARTICIPATION_STSTORAGE& reserveParticipation,
          const AREA_RESERVES_VECTOR& reserves,
          const CAPACITY_RESERVATION& reserve)
        {
            setSTStorageReserveParticipationRightSides(reserveParticipation, reserves, reserve);
            const auto& CorrespondanceCntNativesCntOptim = problemeHebdo
                                                             ->CorrespondanceCntNativesCntOptim
                                                               [pdtJour];
            int cnt = CorrespondanceCntNativesCntOptim
                        .NumeroDeContrainteDesContraintesSTStorageClusterMaxWithdrawParticipation
                          [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = reserveParticipation.maxTurbining;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesSTStorageClusterMaxInjectionParticipation
                      [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = reserveParticipation.maxPumping;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesSTStockEnergyLevelReserveParticipation
                      [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                auto& cluster = problemeHebdo
                                  ->ShortTermStorage[pays][reserveParticipation.clusterIdInArea];
                double level_min = cluster.reservoirCapacity
                                   * cluster.series->lowerRuleCurve[pdtGlobal];

                SecondMembre[cnt] = -reserve.maxEnergyActivationRatio
                                    * reserve.maxActivationDuration * level_min;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }

        // Set the rigth sides of equations for a ShortTerm cluster participation to a reserve down
        void setSTStorageReserveDownParticipationRightSides(
          const RESERVE_PARTICIPATION_STSTORAGE& reserveParticipation,
          const AREA_RESERVES_VECTOR& reserves,
          const CAPACITY_RESERVATION& reserve)
        {
            setSTStorageReserveParticipationRightSides(reserveParticipation, reserves, reserve);
            const auto& CorrespondanceCntNativesCntOptim = problemeHebdo
                                                             ->CorrespondanceCntNativesCntOptim
                                                               [pdtJour];
            int cnt = CorrespondanceCntNativesCntOptim
                        .NumeroDeContrainteDesContraintesSTStorageClusterMaxWithdrawParticipation
                          [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = reserveParticipation.maxTurbining;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesSTStorageClusterMaxInjectionParticipation
                      [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = reserveParticipation.maxPumping;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesSTStockEnergyLevelReserveParticipation
                      [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                auto& cluster = problemeHebdo
                                  ->ShortTermStorage[pays][reserveParticipation.clusterIdInArea];
                double level_max = cluster.reservoirCapacity
                                   * cluster.series->upperRuleCurve[pdtGlobal];
                SecondMembre[cnt] = reserve.maxEnergyActivationRatio * reserve.maxActivationDuration
                                    * level_max;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }

        // Common setter for the LongTerm Storage clusters
        void setLTStorageReserveParticipationRightSides(
          const RESERVE_PARTICIPATION_LTSTORAGE& reserveParticipation,
          const AREA_RESERVES_VECTOR& reserves,
          const CAPACITY_RESERVATION& reserve)
        {
            const auto& CorrespondanceCntNativesCntOptim = problemeHebdo
                                                             ->CorrespondanceCntNativesCntOptim
                                                               [pdtJour];
            auto& hydroCluster = problemeHebdo->CaracteristiquesHydrauliques[pays];
            int globalClusterIdx = hydroCluster.GlobalHydroIndex;

            int cnt
              = CorrespondanceCntNativesCntOptim
                  .NumeroDeContrainteDesContraintesLTStorageClusterTurbiningCapacityThreasholdsMax
                    [globalClusterIdx];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = hydroCluster.ContrainteDePmaxHydrauliqueHoraire[pdtJour];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesLTStorageClusterTurbiningCapacityThreasholdsMin
                      [globalClusterIdx];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = hydroCluster.MingenHoraire[pdtJour];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesLTStorageClusterPumpingCapacityThreasholds
                      [globalClusterIdx];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = hydroCluster.ContrainteDePmaxPompageHoraire[pdtJour];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            double level_max = hydroCluster.NiveauHoraireSup[pdtHebdo];
            double level_min = hydroCluster.NiveauHoraireInf[pdtHebdo];
            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesLTStockLevelReserveParticipationDown
                      [globalClusterIdx];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = hydroCluster.NiveauHoraireSup[pdtHebdo];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesLTStockLevelReserveParticipationUp
                      [globalClusterIdx];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = hydroCluster.NiveauHoraireInf[pdtHebdo];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            cnt
              = CorrespondanceCntNativesCntOptim
                  .NumeroDeContrainteDesContraintesLTGlobalStockEnergyLevelReserveParticipationDown
                    [globalClusterIdx];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = reserves.maxGlobalActivationDurationDown
                                    * reserves.maxGlobalEnergyActivationRatioDown * level_max;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }

            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesLTGlobalStockEnergyLevelReserveParticipationUp
                      [globalClusterIdx];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = -reserves.maxGlobalActivationDurationUp
                                    * reserves.maxGlobalEnergyActivationRatioUp * level_min;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }

        // Set the rigth sides of equations for a LongTerm cluster participation to a reserve up
        void setLTStorageReserveUpParticipationRightSides(
          const RESERVE_PARTICIPATION_LTSTORAGE& reserveParticipation,
          const AREA_RESERVES_VECTOR& reserves,
          const CAPACITY_RESERVATION& reserve)
        {
            setLTStorageReserveParticipationRightSides(reserveParticipation, reserves, reserve);
            const auto& CorrespondanceCntNativesCntOptim = problemeHebdo
                                                             ->CorrespondanceCntNativesCntOptim
                                                               [pdtJour];
            int cnt = CorrespondanceCntNativesCntOptim
                        .NumeroDeContrainteDesContraintesLTStorageClusterMaxWithdrawParticipation
                          [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = reserveParticipation.maxTurbining;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesLTStorageClusterMaxInjectionParticipation
                      [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = reserveParticipation.maxPumping;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesLTStockEnergyLevelReserveParticipation
                      [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                auto& hydroCluster = problemeHebdo->CaracteristiquesHydrauliques[pays];
                double level_min = hydroCluster.NiveauHoraireInf[pdtHebdo];

                SecondMembre[cnt] = -reserve.maxEnergyActivationRatio
                                    * reserve.maxActivationDuration * level_min;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }

        // Set the rigth sides of equations for a LongTerm cluster participation to a reserve down
        void setLTStorageReserveDownParticipationRightSides(
          const RESERVE_PARTICIPATION_LTSTORAGE& reserveParticipation,
          const AREA_RESERVES_VECTOR& reserves,
          const CAPACITY_RESERVATION& reserve)
        {
            setLTStorageReserveParticipationRightSides(reserveParticipation, reserves, reserve);
            const auto& CorrespondanceCntNativesCntOptim = problemeHebdo
                                                             ->CorrespondanceCntNativesCntOptim
                                                               [pdtJour];
            int cnt = CorrespondanceCntNativesCntOptim
                        .NumeroDeContrainteDesContraintesLTStorageClusterMaxWithdrawParticipation
                          [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = reserveParticipation.maxTurbining;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesLTStorageClusterMaxInjectionParticipation
                      [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = reserveParticipation.maxPumping;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContrainteDesContraintesLTStockEnergyLevelReserveParticipation
                      [reserveParticipation.globalIndexClusterParticipation];
            if (cnt >= 0)
            {
                auto& hydroCluster = problemeHebdo->CaracteristiquesHydrauliques[pays];
                double level_max = hydroCluster.NiveauHoraireSup[pdtHebdo];
                SecondMembre[cnt] = reserve.maxEnergyActivationRatio * reserve.maxActivationDuration
                                    * level_max;
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }
    };

    ReserveVariablesRightSidesSetter reserveVariablesRightSidesSetter(problemeHebdo);

    for (int pdtHebdo = PremierPdtDeLIntervalle, pdtJour = 0; pdtHebdo < DernierPdtDeLIntervalle;
         pdtHebdo++, pdtJour++)
    {
        int pdtGlobal = problemeHebdo->weekInTheYear * problemeHebdo->NombreDePasDeTempsDUneJournee
                          * problemeHebdo->NombreDeJours
                        + pdtJour;

        reserveVariablesRightSidesSetter.setPdtJour(pdtJour);
        reserveVariablesRightSidesSetter.setPdtHebdo(pdtHebdo);
        reserveVariablesRightSidesSetter.setPdtGlobal(pdtGlobal);

        for (int pays = 0; pays < problemeHebdo->NombreDePays; pays++)
        {
            reserveVariablesRightSidesSetter.setPays(pays);
            const auto& areaReserves = problemeHebdo->allReserves[pays];

            // Up Reserves Right Sides
            for (const auto& areaReserveUp: areaReserves.areaCapacityReservationsUp)
            {
                reserveVariablesRightSidesSetter.setReserveRightSides(areaReserveUp);

                // Thermal Cluster
                for (const auto& [clusterId, clusterReserveParticipation]:
                     areaReserveUp.AllThermalReservesParticipation)
                {
                    reserveVariablesRightSidesSetter.setThermalReserveUpParticipationRightSides(
                      clusterReserveParticipation);
                }

                // Short Term Storage Cluster
                for (const auto& [clusterId, clusterReserveParticipation]:
                     areaReserveUp.AllSTStorageReservesParticipation)
                {
                    reserveVariablesRightSidesSetter.setSTStorageReserveUpParticipationRightSides(
                      clusterReserveParticipation,
                      areaReserves,
                      areaReserveUp);
                }

                // Long Term Storage Cluster
                for (const auto& clusterReserveParticipation:
                     areaReserveUp.AllLTStorageReservesParticipation)
                {
                    reserveVariablesRightSidesSetter.setLTStorageReserveUpParticipationRightSides(
                      clusterReserveParticipation,
                      areaReserves,
                      areaReserveUp);
                }
            }
            for (const auto& areaReserveDown: areaReserves.areaCapacityReservationsDown)
            {
                reserveVariablesRightSidesSetter.setReserveRightSides(areaReserveDown);

                // Thermal Cluster
                for (const auto& [clusterId, clusterReserveParticipation]:
                     areaReserveDown.AllThermalReservesParticipation)
                {
                    reserveVariablesRightSidesSetter.setThermalReserveDownParticipationRightSides(
                      clusterReserveParticipation);
                }

                // Short Term Storage Cluster
                for (const auto& [clusterId, clusterReserveParticipation]:
                     areaReserveDown.AllSTStorageReservesParticipation)
                {
                    reserveVariablesRightSidesSetter.setSTStorageReserveDownParticipationRightSides(
                      clusterReserveParticipation,
                      areaReserves,
                      areaReserveDown);
                }

                // Long Term Storage Cluster
                for (const auto& clusterReserveParticipation:
                     areaReserveDown.AllLTStorageReservesParticipation)
                {
                    reserveVariablesRightSidesSetter.setLTStorageReserveDownParticipationRightSides(
                      clusterReserveParticipation,
                      areaReserves,
                      areaReserveDown);
                }
            }
        }
    }

    return;
}

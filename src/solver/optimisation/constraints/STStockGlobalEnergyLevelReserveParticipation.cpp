#include "antares/solver/optimisation/constraints/STStockGlobalEnergyLevelReserveParticipation.h"

void STStockGlobalEnergyLevelReserveParticipation::add(int pays, int cluster, int pdt)
{
    int globalClusterIdx = data.shortTermStorageOfArea[pays][cluster].clusterGlobalIndex;


    if (!data.Simulation)
    {
        // 15 (i)
        // Stock participation is energy constrained (optional constraints)
        //  Sum(P_{res,t_st} * R_{min,res} +/- J_down/up * R_{lambda,t_st}) <= n_min * R_up
        // R_t : stock level at time t 
        // P_{res,t_st} : power participation for reserve down res at time t_st
        // R_{min,res} : max power participation ratio 
        // R_up : max stock level

        // DOWN reserves
        {
            builder.updateHourWithinWeek(pdt);

            for (int t=0; t < data.areaReserves[pays].maxGlobalActivationDurationDown; t++)
            {

                for (auto& capacityReservation : data.areaReserves[pays].areaCapacityReservationsDown)
                {
                    if (capacityReservation.AllSTStorageReservesParticipation.contains(cluster))
                    {
                        RESERVE_PARTICIPATION_STSTORAGE& reserveParticipation = capacityReservation.AllSTStorageReservesParticipation[cluster];
                        builder.STStorageClusterReserveDownParticipation(
                            reserveParticipation.globalIndexClusterParticipation,
                            capacityReservation.maxActivationRatio,
                            t, builder.data.NombreDePasDeTempsPourUneOptimisation);
                    }
                }
                if (builder.NumberOfVariables() > 0)
                {
                    builder.ShortTermStorageLevel(globalClusterIdx, data.areaReserves[pays].maxGlobalEnergyActivationRatioDown,
                            t, builder.data.NombreDePasDeTempsPourUneOptimisation);
                }
            }

            if (builder.NumberOfVariables() > 0)
            {
                builder.lessThan();

                data.CorrespondanceCntNativesCntOptim[pdt]
                    .NumeroDeContrainteDesContraintesSTGlobalStockEnergyLevelReserveParticipationDown
                    [globalClusterIdx]
                    = builder.data.nombreDeContraintes;

                ConstraintNamer namer(builder.data.NomDesContraintes);
                const int hourInTheYear = builder.data.weekInTheYear * 168 + pdt;
                namer.UpdateTimeStep(hourInTheYear);
                namer.UpdateArea(builder.data.NomsDesPays[pays]);
                namer.STGlobalEnergyStockLevelReserveParticipationDown(builder.data.nombreDeContraintes,
                    "ShortTermStorage");
                builder.build();
            }
        }

        // UP reserves
        {
            builder.updateHourWithinWeek(pdt);

            for (int t=0; t < data.areaReserves[pays].maxGlobalActivationDurationUp; t++)
            {
                for (auto& capacityReservation : data.areaReserves[pays].areaCapacityReservationsUp)
                {
                    if (capacityReservation.AllSTStorageReservesParticipation.contains(cluster))
                    {
                        RESERVE_PARTICIPATION_STSTORAGE& reserveParticipation = capacityReservation.AllSTStorageReservesParticipation[cluster];
                        builder.STStorageClusterReserveUpParticipation(
                            reserveParticipation.globalIndexClusterParticipation,
                            capacityReservation.maxActivationRatio,
                            t, builder.data.NombreDePasDeTempsPourUneOptimisation);
                    }
                }
                if (builder.NumberOfVariables() > 0)
                {
                    builder.ShortTermStorageLevel(globalClusterIdx, -data.areaReserves[pays].maxGlobalEnergyActivationRatioUp,
                            t, builder.data.NombreDePasDeTempsPourUneOptimisation);
                }
            }
            
            if (builder.NumberOfVariables() > 0)
            {
                builder.lessThan();

                data.CorrespondanceCntNativesCntOptim[pdt]
                    .NumeroDeContrainteDesContraintesSTGlobalStockEnergyLevelReserveParticipationUp
                    [globalClusterIdx]
                    = builder.data.nombreDeContraintes;

                ConstraintNamer namer(builder.data.NomDesContraintes);
                const int hourInTheYear = builder.data.weekInTheYear * 168 + pdt;
                namer.UpdateTimeStep(hourInTheYear);
                namer.UpdateArea(builder.data.NomsDesPays[pays]);
                namer.STGlobalEnergyStockLevelReserveParticipationUp(builder.data.nombreDeContraintes,
                    "ShortTermStorage");
                builder.build();
            }
        }
    }
    else
    {
        // Lambda that count the number of reserveParticipations
        auto countReservesParticipations = [cluster](const std::vector<CAPACITY_RESERVATION>& reservations, int t_max)
        {
            int counter = 0;
            for (const auto& capacityReservation: reservations)
            {
                counter += capacityReservation.AllSTStorageReservesParticipation.count(cluster) * t_max;
            }
            return counter;
        };

        int nbTermsUp = countReservesParticipations(data.areaReserves[pays].areaCapacityReservationsUp, data.areaReserves[pays].maxGlobalActivationDurationUp);
        int nbTermsDown = countReservesParticipations(data.areaReserves[pays].areaCapacityReservationsDown, data.areaReserves[pays].maxGlobalActivationDurationDown);

        builder.data.NbTermesContraintesPourLesReserves += (nbTermsUp + data.areaReserves[pays].maxGlobalActivationDurationUp) * (nbTermsUp > 0) + (nbTermsDown + data.areaReserves[pays].maxGlobalActivationDurationDown) * (nbTermsDown > 0);
        builder.data.nombreDeContraintes += (nbTermsUp > 0) + (nbTermsDown > 0);
    }
}
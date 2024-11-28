#include "antares/solver/optimisation/constraints/STStockLevelReserveParticipation.h"

void STStockLevelReserveParticipation::add(int pays, int cluster, int pdt)
{
    int globalClusterIdx = data.shortTermStorageOfArea[pays][cluster].clusterGlobalIndex;

    if (!data.Simulation)
    {
        // 15 (g) (1)
        // Participation of down reserves requires a sufficient level of stock
        // R_t + Sum(P_{res} * R_{min,res}) <= R_up
        // R_t : stock level at time t 
        // P_{res} : power participation for reserve down res
        // R_{min,res} : max power participation ratio 
        // R_up : max stock level
        {
            builder.updateHourWithinWeek(pdt);

            for (auto& capacityReservation : data.areaReserves[pays].areaCapacityReservationsDown)
            {
                if (capacityReservation.AllSTStorageReservesParticipation.contains(cluster))
                {
                    RESERVE_PARTICIPATION_STSTORAGE reserveParticipations = capacityReservation.AllSTStorageReservesParticipation[cluster];
                    builder.STStorageClusterReserveDownParticipation(
                                reserveParticipations.globalIndexClusterParticipation,
                                capacityReservation.maxActivationRatio);
                }
            }
            if (builder.NumberOfVariables() > 0)
            {
                builder.ShortTermStorageLevel(globalClusterIdx, 1.);
                builder.lessThan();
                data.CorrespondanceCntNativesCntOptim[pdt]
                    .NumeroDeContrainteDesContraintesSTStockLevelReserveParticipationDown
                    [globalClusterIdx]
                    = builder.data.nombreDeContraintes;
                ConstraintNamer namer(builder.data.NomDesContraintes);
                const int hourInTheYear = builder.data.weekInTheYear * 168 + pdt;
                namer.UpdateTimeStep(hourInTheYear);
                namer.UpdateArea(builder.data.NomsDesPays[pays]);
                namer.STStockLevelReserveParticipationDown(builder.data.nombreDeContraintes,
                    "ShortTermStorage");
                builder.build();
            }
        }

        // 15 (g) (2)
        // Participation of up reserves requires a sufficient level of stock
        // R_t - Sum(P_{res} * R_{min,res}) >= R_down
        // R_t : stock level at time t 
        // P_{res} : power participation for reserve up res
        // R_{min,res} : max power participation ratio 
        // R_down : min stock level
        {
            builder.updateHourWithinWeek(pdt);

            for (auto& capacityReservation : data.areaReserves[pays].areaCapacityReservationsUp)
            {
                if (capacityReservation.AllSTStorageReservesParticipation.contains(cluster))
                {
                  RESERVE_PARTICIPATION_STSTORAGE reserveParticipations = capacityReservation.AllSTStorageReservesParticipation[cluster];
                  builder.STStorageClusterReserveUpParticipation(
                                reserveParticipations.globalIndexClusterParticipation,
                                -capacityReservation.maxActivationRatio);
                }
            }
            if (builder.NumberOfVariables() > 0)
            {
                builder.ShortTermStorageLevel(globalClusterIdx, -1.);
                builder.lessThan();
                data.CorrespondanceCntNativesCntOptim[pdt]
                    .NumeroDeContrainteDesContraintesSTStockLevelReserveParticipationUp
                    [globalClusterIdx]
                    = builder.data.nombreDeContraintes;
                ConstraintNamer namer(builder.data.NomDesContraintes);
                const int hourInTheYear = builder.data.weekInTheYear * 168 + pdt;
                namer.UpdateTimeStep(hourInTheYear);
                namer.UpdateArea(builder.data.NomsDesPays[pays]);
                namer.STStockLevelReserveParticipationUp(builder.data.nombreDeContraintes,
                    "ShortTermStorage");
                builder.build();
            }
        }
    }
    else
    {
        // Lambda that count the number of reserveParticipations
        auto countReservesParticipations = [cluster](const std::vector<CAPACITY_RESERVATION>& reservations)
        {
            int counter = 0;
            for (const auto& capacityReservation: reservations)
            {
                counter += capacityReservation.AllSTStorageReservesParticipation.count(cluster);
            }
            return counter;
        };

        int nbTermsUp = countReservesParticipations(data.areaReserves[pays].areaCapacityReservationsUp);
        int nbTermsDown = countReservesParticipations(data.areaReserves[pays].areaCapacityReservationsDown);

        builder.data.NbTermesContraintesPourLesReserves += (nbTermsUp + 1) * (nbTermsUp > 0) + (nbTermsDown + 1) * (nbTermsDown > 0);
        builder.data.nombreDeContraintes += (nbTermsUp > 0) + (nbTermsDown > 0);
    }
}
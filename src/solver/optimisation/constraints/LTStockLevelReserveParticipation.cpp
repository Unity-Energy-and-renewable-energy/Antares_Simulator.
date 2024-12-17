#include "antares/solver/optimisation/constraints/LTStockLevelReserveParticipation.h"

void LTStockLevelReserveParticipation::add(int pays, int cluster, int pdt)
{
    int globalClusterIdx = data.longTermStorageOfArea[pays].GlobalHydroIndex;

    if (!data.Simulation)
    {
        // 15 (r) (1)
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
                if (capacityReservation.AllLTStorageReservesParticipation.size())
                {
                    RESERVE_PARTICIPATION_LTSTORAGE reserveParticipations = capacityReservation.AllLTStorageReservesParticipation[cluster];
                    builder.LTStorageClusterReserveDownParticipation(
                                reserveParticipations.globalIndexClusterParticipation,
                                capacityReservation.maxActivationRatio);
                }
            }
            if (builder.NumberOfVariables() > 0)
            {
                builder.HydroLevel(globalClusterIdx, 1.);
                builder.lessThan();
                data.CorrespondanceCntNativesCntOptim[pdt]
                    .NumeroDeContrainteDesContraintesLTStockLevelReserveParticipationDown
                    [globalClusterIdx]
                    = builder.data.nombreDeContraintes;
                ConstraintNamer namer(builder.data.NomDesContraintes);
                const int hourInTheYear = builder.data.weekInTheYear * 168 + pdt;
                namer.UpdateTimeStep(hourInTheYear);
                namer.UpdateArea(builder.data.NomsDesPays[pays]);
                namer.LTStockLevelReserveParticipationDown(builder.data.nombreDeContraintes,
                    "LongTermStorage");
                builder.build();
            }
        }

        // 15 (r) (2)
        // Participation of up reserves requires a sufficient level of stock
        // -R_t + Sum(P_{res} * R_{min,res}) <= -R_down
        // R_t : stock level at time t 
        // P_{res} : power participation for reserve up res
        // R_{min,res} : max power participation ratio 
        // R_down : min stock level
        {
            builder.updateHourWithinWeek(pdt);

            for (auto& capacityReservation : data.areaReserves[pays].areaCapacityReservationsUp)
            {
                if (capacityReservation.AllLTStorageReservesParticipation.size())
                {
                  RESERVE_PARTICIPATION_LTSTORAGE reserveParticipations = capacityReservation.AllLTStorageReservesParticipation[cluster];
                  builder.LTStorageClusterReserveUpParticipation(
                                reserveParticipations.globalIndexClusterParticipation,
                                capacityReservation.maxActivationRatio);
                }
            }
            if (builder.NumberOfVariables() > 0)
            {
                builder.HydroLevel(globalClusterIdx, -1.);
                builder.lessThan();
                data.CorrespondanceCntNativesCntOptim[pdt]
                    .NumeroDeContrainteDesContraintesLTStockLevelReserveParticipationUp
                    [globalClusterIdx]
                    = builder.data.nombreDeContraintes;
                ConstraintNamer namer(builder.data.NomDesContraintes);
                const int hourInTheYear = builder.data.weekInTheYear * 168 + pdt;
                namer.UpdateTimeStep(hourInTheYear);
                namer.UpdateArea(builder.data.NomsDesPays[pays]);
                namer.LTStockLevelReserveParticipationUp(builder.data.nombreDeContraintes,
                    "LongTermStorage");
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
                counter += capacityReservation.AllLTStorageReservesParticipation.size();
            }
            return counter;
        };

        int nbTermsUp = countReservesParticipations(data.areaReserves[pays].areaCapacityReservationsUp);
        int nbTermsDown = countReservesParticipations(data.areaReserves[pays].areaCapacityReservationsDown);

        builder.data.nombreDeContraintes += (nbTermsUp > 0) + (nbTermsDown > 0);
    }
}
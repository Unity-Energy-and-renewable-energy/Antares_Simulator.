#include "antares/solver/optimisation/constraints/STStockLevelReserveParticipation.h"

void STStockLevelReserveParticipation::add(int pays, int cluster, int pdt)
{
    int globalClusterIdx = data.shortTermStorageOfArea[pays][cluster].clusterGlobalIndex;

    if (!data.Simulation)
    {
        // 15 (r) (1)
        // Participation of down reserves requires a sufficient level of stock
        // R_t + Sum(P_{res,t_st} * R_{min,res}) <= R_up
        // R_t : stock level at time t 
        // P_{res,t_st} : power participation for reserve down res at time t_st
        // R_{min,res} : max power participation ratio 
        // R_up : max stock level
        {
            builder.updateHourWithinWeek(pdt).ShortTermStorageLevel(globalClusterIdx, 1.);

            for (const auto& capacityReservation : data.areaReserves[pays].areaCapacityReservationsDown)
            {
                int t_max = pdt + capacityReservation.maxActivationDuration;
                if (t_max > builder.data.NombreDePasDeTempsPourUneOptimisation)
                    t_max = builder.data.NombreDePasDeTempsPourUneOptimisation;

                for (int t=pdt; t < t_max; t++)
                {
                    builder.updateHourWithinWeek(t);
                    for (const auto& [clusterId, reserveParticipations] : capacityReservation.AllSTStorageReservesParticipation)
                    {
                        if (cluster == clusterId)
                            builder.STStorageClusterReserveDownParticipation(
                                reserveParticipations.globalIndexClusterParticipation,
                                capacityReservation.maxActivationRatio);
                    }
                }
            }
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

        // 15 (r) (2)
        // Participation of up reserves requires a sufficient level of stock
        // R_t - Sum(P_{res,t_st} * R_{min,res}) >= R_down
        // R_t : stock level at time t 
        // P_{res,t_st} : power participation for reserve up res at time t_st
        // R_{min,res} : max power participation ratio 
        // R_down : min stock level
        {
            builder.updateHourWithinWeek(pdt).ShortTermStorageLevel(globalClusterIdx, 1.);

            for (const auto& capacityReservation : data.areaReserves[pays].areaCapacityReservationsUp)
            {
                int t_max = pdt + capacityReservation.maxActivationDuration;
                if (t_max > builder.data.NombreDePasDeTempsPourUneOptimisation)
                    t_max = builder.data.NombreDePasDeTempsPourUneOptimisation;

                for (int t=pdt; t < t_max; t++)
                {
                    builder.updateHourWithinWeek(t);
                    for (const auto& [clusterId, reserveParticipations] : capacityReservation.AllSTStorageReservesParticipation)
                    {
                        if (cluster == clusterId)
                            builder.STStorageClusterReserveUpParticipation(
                                reserveParticipations.globalIndexClusterParticipation,
                                -capacityReservation.maxActivationRatio);
                    }
                }
            }
            builder.greaterThan();
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
    else
    {
        // Lambda that count the number of reserveParticipations
        auto countReservesParticipations = [cluster](const std::vector<CAPACITY_RESERVATION>& reservations, int time, int time_max)
        {
            int counter = 0;
            for (const auto& capacityReservation: reservations)
            {
                int n_t = capacityReservation.maxActivationDuration;
                if (time + n_t > time_max)
                    n_t = time_max - time;
                counter += capacityReservation.AllSTStorageReservesParticipation.count(cluster) * n_t;
            }
            return counter;
        };

        int nbTermsUp = countReservesParticipations(
          data.areaReserves[pays].areaCapacityReservationsUp, pdt, builder.data.NombreDePasDeTempsPourUneOptimisation);
        int nbTermsDown = countReservesParticipations(
          data.areaReserves[pays].areaCapacityReservationsDown, pdt, builder.data.NombreDePasDeTempsPourUneOptimisation);

        builder.data.NbTermesContraintesPourLesReserves += (nbTermsUp + 1) + (nbTermsDown + 1);
        builder.data.nombreDeContraintes += 2;
    }
}
#include "antares/solver/optimisation/constraints/STStockEnergyLevelReserveParticipation.h"

void STStockEnergyLevelReserveParticipation::add(int pays, int cluster, int reserve, int pdt, bool isUpReserve)
{
    int globalClusterIdx = data.shortTermStorageOfArea[pays][cluster].clusterGlobalIndex;
    CAPACITY_RESERVATION& capacityReservation
              = isUpReserve ? data.areaReserves[pays].areaCapacityReservationsUp[reserve]
                            : data.areaReserves[pays].areaCapacityReservationsDown[reserve];

    if (capacityReservation.maxActivationDuration > 0)
    {
        if (!data.Simulation)
        {
            // 15 (h) (1)
            // Participation of down reserves requires a sufficient level of stock
            //  Sum(P_{res,t_st} * R_{min,res} +/- J_res * R_{lambda,t_st}) <= n_min * R_up
            // R_t : stock level at time t 
            // P_{res,t_st} : power participation for reserve down res at time t_st
            // R_{min,res} : max power participation ratio 
            // R_up : max stock level
            {
                float sign = isUpReserve ? -1. : 1.;

                RESERVE_PARTICIPATION_STSTORAGE& reserveParticipation
                  = capacityReservation.AllSTStorageReservesParticipation[cluster];

                builder.updateHourWithinWeek(pdt);

                for (int t=0; t < capacityReservation.maxActivationDuration; t++)
                {   
                    if (isUpReserve)
                    {
                        builder.STStorageClusterReserveUpParticipation(
                            reserveParticipation.globalIndexClusterParticipation,
                            capacityReservation.maxActivationRatio, 
                            t, builder.data.NombreDePasDeTempsPourUneOptimisation);
                    }
                    else
                    {
                        builder.STStorageClusterReserveDownParticipation(
                            reserveParticipation.globalIndexClusterParticipation,
                            capacityReservation.maxActivationRatio, 
                            t, builder.data.NombreDePasDeTempsPourUneOptimisation);
                    }
                    builder.ShortTermStorageLevel(globalClusterIdx, sign * capacityReservation.maxEnergyActivationRatio,
                        t, builder.data.NombreDePasDeTempsPourUneOptimisation);
                }

                builder.lessThan();

                data.CorrespondanceCntNativesCntOptim[pdt]
                    .NumeroDeContrainteDesContraintesSTStockEnergyLevelReserveParticipation
                    [reserveParticipation.globalIndexClusterParticipation]
                    = builder.data.nombreDeContraintes;

                ConstraintNamer namer(builder.data.NomDesContraintes);
                const int hourInTheYear = builder.data.weekInTheYear * 168 + pdt;
                namer.UpdateTimeStep(hourInTheYear);
                namer.UpdateArea(builder.data.NomsDesPays[pays]);
                namer.STEnergyStockLevelReserveParticipation(builder.data.nombreDeContraintes,
                    reserveParticipation.clusterName,
                    capacityReservation.reserveName);
                builder.build();
            }
        }
        else
        {
            builder.data.nombreDeContraintes += 1;
        }
    }
}
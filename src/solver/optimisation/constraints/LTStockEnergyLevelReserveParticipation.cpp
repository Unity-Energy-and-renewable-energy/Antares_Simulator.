#include "antares/solver/optimisation/constraints/LTStockEnergyLevelReserveParticipation.h"

void LTStockEnergyLevelReserveParticipation::add(int pays,
                                                 int cluster,
                                                 int reserve,
                                                 int pdt,
                                                 bool isUpReserve)
{
    int globalClusterIdx = data.longTermStorageOfArea[pays].GlobalHydroIndex;
    CAPACITY_RESERVATION& capacityReservation = isUpReserve
                                                  ? data.areaReserves[pays]
                                                      .areaCapacityReservationsUp[reserve]
                                                  : data.areaReserves[pays]
                                                      .areaCapacityReservationsDown[reserve];

    if (capacityReservation.maxActivationDuration > 0)
    {
        if (!data.Simulation)
        {
            // 15 (s) (1)
            // Participation of down reserves requires a sufficient level of stock
            //  Sum(P_{res,t_st} * R_{min,res} +/- J_res * R_{lambda,t_st}) <= n_min * R_up
            // R_t : stock level at time t
            // P_{res,t_st} : power participation for reserve down res at time t_st
            // R_{min,res} : max power participation ratio
            // R_up : max stock level
            {
                float sign = isUpReserve ? -1. : 1.;

                RESERVE_PARTICIPATION_LTSTORAGE& reserveParticipation
                  = capacityReservation.AllLTStorageReservesParticipation[cluster];

                builder.updateHourWithinWeek(pdt);

                for (int t = 0; t < capacityReservation.maxActivationDuration; t++)
                {
                    if (isUpReserve)
                    {
                        builder.LTStorageClusterReserveUpParticipation(
                          reserveParticipation.globalIndexClusterParticipation,
                          capacityReservation.maxActivationRatio,
                          t,
                          builder.data.NombreDePasDeTempsPourUneOptimisation);
                    }
                    else
                    {
                        builder.LTStorageClusterReserveDownParticipation(
                          reserveParticipation.globalIndexClusterParticipation,
                          capacityReservation.maxActivationRatio,
                          t,
                          builder.data.NombreDePasDeTempsPourUneOptimisation);
                    }
                    builder.HydroLevel(
                      globalClusterIdx,
                      sign * capacityReservation.maxEnergyActivationRatio,
                      t,
                      builder.data.NombreDePasDeTempsPourUneOptimisation);
                }

                builder.lessThan();

                data.CorrespondanceCntNativesCntOptim[pdt]
                  .NumeroDeContrainteDesContraintesLTStockEnergyLevelReserveParticipation
                    [reserveParticipation.globalIndexClusterParticipation]
                  = builder.data.nombreDeContraintes;

                ConstraintNamer namer(builder.data.NomDesContraintes);
                const int hourInTheYear = builder.data.weekInTheYear * 168 + pdt;
                namer.UpdateTimeStep(hourInTheYear);
                namer.UpdateArea(builder.data.NomsDesPays[pays]);
                namer.LTEnergyStockLevelReserveParticipation(builder.data.nombreDeContraintes,
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
